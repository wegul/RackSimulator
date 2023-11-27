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
    parser.add_argument('-sz', required=True)
    args = parser.parse_args()
    filename = args.fi
    newfilename = args.fo
    sz = (int)(args.sz)
    sz = int(sz/8)

    print("Reading ", filename, "...")

    # trace = pd.read_csv(filename, header=None)
    trace = pd.read_csv(filename, header=None)
    MAX_FLOW_ID = trace.shape[0]

    cur_timeslot = 0
    flowList = init_FlowList(trace=trace)
    totalFlowStarted = 0
    activeFlows = []

    print("Starting calculation...")

    send_busy = np.zeros(shape=(144, 1))
    recv_busy = np.zeros(shape=(144, 1))

    # FlowID-0, FlowType-1, Src-2, Dst-3, FlowSize-4, ReqSize=5(ignore), Create-6
    while totalFlowStarted < MAX_FLOW_ID:
        # Activate flows for each node
        for i in range(0, MAX_FLOW_ID):
            flow = flowList[i]
            if flow.Create == cur_timeslot:
                activeFlows.append(flow)

        # In each timeslot, sort activeFlows by creation time
        activeFlows = sorted(
            activeFlows, key=lambda f: f.Create)
        send_busy = np.zeros(shape=(144, 1))
        recv_busy = np.zeros(shape=(144, 1))

        for idx in range(0, len(activeFlows)):
            flow = activeFlows[idx]
            Src = flow.Src
            Dst = flow.Dst
            if send_busy[Src] == 0 and recv_busy[Dst] == 0 and flow.idealStart < 0:
                flow.idealStart = cur_timeslot
                totalFlowStarted += 1
                send_busy[Src] = 1
                recv_busy[Dst] = 1

        # # Remove those already started
        nextActiveFlows = []
        for flow in activeFlows:
            if flow.idealStart < 0:
                nextActiveFlows.append(flow)
        activeFlows = nextActiveFlows
        cur_timeslot += sz

    print("Finished!, total flow started = ", totalFlowStarted)

    # Write to output file
    newFlowTrace = pd.DataFrame([vars(t) for t in flowList])
    idealFCT_list = []
    for i in range(0, MAX_FLOW_ID):
        flow = flowList[i]
        idealFCT_list.append(
            flow.idealStart+34-flow.Create)  # rresp.ReqLen is its RREQ's create time.

    newFlowTrace['IdealFCT'] = idealFCT_list
    newFlowTrace.to_csv(newfilename, index=None)

    # trace.to_csv(newfilename, header=None, index=None)


if __name__ == '__main__':
    main()
