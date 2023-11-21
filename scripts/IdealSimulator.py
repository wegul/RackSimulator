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
        self.flowId = int(elmts[0])
        self.flowType = int(elmts[1])
        self.src = int(elmts[2])
        self.dst = int(elmts[3])
        self.flowSize = int(elmts[4])
        # self.reqLen = int(elmts[5])
        self.createTime = int(elmts[5])
        self.notifTime = int(elmts[6])
        self.grantTime = int(elmts[7])
        self.startTime = int(elmts[8])
        self.finishTime = int(elmts[9])
        self.fct = int(elmts[10])
        self.sld = int(elmts[11])
        self.xput = int(elmts[12])
        self.idealStart = self.createTime
        self.started = False

    def toArr(self):
        return [self.flowId, self.flowType, self.src, self.dst, self.flowSize, self.createTime, self.notifTime,
                self.grantTime, self.startTime, self.finishTime, self.fct, self.sld, self.xput,
                self.idealStart]


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
    trace = pd.read_csv(filename)
    MAX_FLOW_ID = trace.shape[0]

    cur_timeslot = 0
    flowList = init_FlowList(trace=trace)
    totalFlowStarted = 0
    activeFlows = []
    startedFlows = []

    print("Starting calculation...")

    # FlowID-0, FlowType-1, Src-2, Dst-3, FlowSize-4, ReqSize=5(ignore), CreateTime-6
    while totalFlowStarted < MAX_FLOW_ID:
        # Activate flows for each node
        for i in range(0, MAX_FLOW_ID):
            flow = flowList[i]
            if flow.createTime == cur_timeslot:
                activeFlows.append(flow)

        # In each timeslot, sort activeFlows by creation time
        activeFlows = sorted(activeFlows, key=lambda f: f.createTime)
        send_busy = np.zeros(shape=(144, 1))
        recv_busy = np.zeros(shape=(144, 1))
        for idx in range(0, len(activeFlows)):
            flow = activeFlows[idx]
            src = flow.src
            dst = flow.dst
            if send_busy[src] == 0 and recv_busy[dst] == 0 and not flow.started:
                flow.idealStart = cur_timeslot
                flow.started = True
                totalFlowStarted += 1
                send_busy[src] = 1
                recv_busy[dst] = 1
                # print("Assign flow {0}: {1} -> {2}, left: {3}".format(flow.flowId,
                #                                            flow.createTime, flow.idealStart, MAX_FLOW_ID-totalFlowStarted))
        # Remove those already started
        nextActiveFlows = []
        for flow in activeFlows:
            if not flow.started:
                nextActiveFlows.append(flow)
        activeFlows = nextActiveFlows

        cur_timeslot += 1

    print("Finished!, total flow started = ", totalFlowStarted)

    # Write to output file
    idealStart_list = []
    idealSld_list = []
    for i in range(0, MAX_FLOW_ID):
        flow = flowList[i]
        idealStart_list.append(flow.idealStart)
        sld = (flow.fct)/(flow.idealStart-flow.createTime+33)
        idealSld_list.append(sld)

    trace['IdealStart'] = idealStart_list
    trace['IdealSlowdown'] = idealSld_list

    trace.to_csv(newfilename)


if __name__ == '__main__':
    main()
