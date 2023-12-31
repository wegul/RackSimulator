import os
import random
import sys
import re
import argparse
import math
import pandas as pd
import numpy as np
from queue import PriorityQueue
MAX_FLOW_ID = -1


class Node:
    def __init__(self, id):
        self.NodeID = id
        self.sAvl = 1
        self.nxtSAvl = -1

        self.rAvl = 1
        self.nxtRAvl = -1


def init_BusyList(nodeNum):
    busyList = np.empty(nodeNum, dtype=Node)
    for i in range(0, nodeNum):
        node = Node(i)
        busyList[i] = node
    return busyList


class Flow:
    def __init__(self, elmts):
        elmts = np.array(elmts)
        self.FlowID = int(elmts[0])
        self.FlowType = int(elmts[1])
        self.Src = int(elmts[2])
        self.Dst = int(elmts[3])
        self.FlowSize = int(elmts[4])
        self.ReqLen = int(elmts[5])
        self.Create = int(elmts[6])
        self.requestedAt = -1
        # self.notifTime = int(elmts.iloc[6])
        # self.grantTime = int(elmts.iloc[7])
        # self.startTime = int(elmts.iloc[8])
        # self.finishTime = int(elmts.iloc[9])
        # self.fct = int(elmts.iloc[10])
        # self.sld = int(elmts.iloc[11])
        # self.xput = int(elmts.iloc[12])

        self.idealStart = -1

        # self.rreqIdealStart = self.Create
        # self.rrespIdealCreate = self.rreqIdealStart+33
        # self.rrespIdealStart = -1

    def toArr(self):
        return [self.FlowID, self.FlowType, self.Src, self.Dst, self.FlowSize, self.ReqLen, self.Create, self.idealStart]


def init_FlowList(trace):
    global MAX_FLOW_ID
    flowList = np.empty(MAX_FLOW_ID, dtype=Flow)
    for i in range(0, MAX_FLOW_ID):
        elmts = trace.iloc[i, :]
        # elmts = [trace.loc[i].at['FlowID'], trace.loc[i].at['FlowType'], trace.loc[i].at['Src'],
        #          trace.loc[i].at['Dst'], trace.loc[i].at['FlowSize'], -1, trace.loc[i].at['Create']]
        flow = Flow(elmts)
        flowList[i] = flow
    return flowList


def main():
    global MAX_FLOW_ID
    parser = argparse.ArgumentParser()
    parser.add_argument('-fi', required=True)
    parser.add_argument('-fo', required=True)
    args = parser.parse_args()
    filename = args.fi
    newfilename = args.fo

    print("Reading ", filename, "...")

    # trace = pd.read_csv(filename, header=None)
    trace = pd.read_csv(filename, header=None)
    MAX_FLOW_ID = trace.shape[0]

    cur_timeslot = 0
    busyList = init_BusyList(144)
    flowList = init_FlowList(trace=trace)
    totalFlowStarted = 0
    activeFlows = []

    print("Starting calculation...")

    # FlowID-0, FlowType-1, Src-2, Dst-3, FlowSize-4, ReqSize=5(ignore), Create-6
    while totalFlowStarted < MAX_FLOW_ID:
        # Activate flows for each node
        for i in range(0, MAX_FLOW_ID):
            flow = flowList[i]
            if flow.Create == cur_timeslot:
                activeFlows.append(flow)

        # In each timeslot, sort activeFlows by creation time
        activeFlows = sorted(
            activeFlows, key=lambda f: (f.Create))

        for idx in range(0, len(activeFlows)):
            flow = activeFlows[idx]
            Src = flow.Src
            Dst = flow.Dst
            if busyList[Src].sAvl == 1 and busyList[Dst].rAvl == 1 and flow.idealStart < 0:
                if flow.FlowType == 1:
                    flow.idealStart = cur_timeslot
                    totalFlowStarted += 1
                    busyList[Src].sAvl = 0  # Not available any more
                    busyList[Dst].rAvl = 0
                    busyList[Src].nxtSAvl = cur_timeslot+2
                    busyList[Dst].nxtRAvl = cur_timeslot+2  # RREQ has 2 blocks

                    # Create RRESP
                    rresp = Flow([MAX_FLOW_ID, 2, flow.Dst,
                                 flow.Src, flow.ReqLen, -1, flow.idealStart+33])
                    rresp.requestedAt = flow.Create
                    MAX_FLOW_ID += 1
                    flowList = np.append(flowList, rresp)
                elif flow.FlowType == 2 or flow.FlowType == 3:
                    flow.idealStart = cur_timeslot
                    totalFlowStarted += 1
                    busyList[Src].sAvl = 0
                    busyList[Dst].rAvl = 0

                    busyList[Src].nxtSAvl = cur_timeslot+(int)(flow.FlowSize/8)
                    busyList[Dst].nxtRAvl = cur_timeslot+(int)(flow.FlowSize/8)

        # # Remove those already started
        nextActiveFlows = []
        for flow in activeFlows:
            if flow.idealStart < 0:
                nextActiveFlows.append(flow)
        activeFlows = nextActiveFlows
        cur_timeslot += 1

        for i in range(0, 144):
            if busyList[i].nxtSAvl == cur_timeslot:
                busyList[i].sAvl = 1
                busyList[i].nxtSAvl = -1
            if busyList[i].nxtRAvl == cur_timeslot:
                busyList[i].rAvl = 1
                busyList[i].nxtRAvl = -1

    print("Finished!, total flow started = ", totalFlowStarted)

    # Write to output file
    newFlowTrace = pd.DataFrame([vars(t) for t in flowList])
    idealFCT_list = []
    for i in range(0, MAX_FLOW_ID):
        flow = flowList[i]
        if flow.FlowType == 1:
            idealFCT_list.append(
                flow.idealStart+32 + (int)(flow.FlowSize/8)-flow.Create)
        elif flow.FlowType == 2:
            idealFCT_list.append(
                flow.idealStart+32 + (int)(flow.FlowSize/8)-flow.requestedAt)  # rresp.ReqLen is its RREQ's create time.
        else:
            idealFCT_list.append(
                flow.idealStart + 34 + (int)(flow.FlowSize/8)-flow.Create)
    newFlowTrace['FCT'] = idealFCT_list
    newFlowTrace.to_csv(newfilename, index=None)

    # trace.to_csv(newfilename, header=None, index=None)


if __name__ == '__main__':
    main()
