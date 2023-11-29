# Process raw trace file:
# Map 512 to 64 nodes
# Give each flow a memType based on -ism
# Randomly pick flows as RREQ or WREQ
# Generate RREQ based on RRESP.

import os
import random
import sys
import re
import argparse
import math
import pandas as pd


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-fi', required=True)
    parser.add_argument('-b', default=100)  # bandwidth in Gbps
    parser.add_argument('-c', default=8)  # packet size, in bytes
    parser.add_argument('-ism', required=True)  # isMem
    parser.add_argument('-msg', required=True)
    parser.add_argument('-fo', required=True)
    args = parser.parse_args()
    slot_time = (float(args.c) * 8.0) / float(args.b) * 1e-9
    filename = args.fi
    newfilename = args.fo
    msgType = int(args.msg)

    isMem = (int)(args.ism)
    trace = pd.read_csv(filename, header=None)
    # Change second to timeslot_num
    timeslots = []
    start_time_arr = trace.iloc[:, -1].values
    for time in start_time_arr:
        slot = (int)(time/slot_time)
        timeslots.append((str)(slot))
    trace.iloc[:, -1] = pd.Series(timeslots)

    # Add flowType
    flowSize = trace.iloc[:, 3].values.copy()
    flowType_arr = []
    if isMem == 1:
        for i in range(0, trace.shape[0]):
            if msgType == 0:  # rreq
                flowType = 1
                flowSize[i] = 16
            else:
                flowType = 3
                flowSize[i] = 64
            flowType_arr.append(flowType)
    else:
        for i in range(0, trace.shape[0]):
            if msgType == 0:
                flowType = 101
                flowSize[i] = 16
            else:
                flowType = 100
                flowSize[i] = 64
            flowType_arr.append(flowType)

    trace.iloc[:, 3] = pd.Series(flowSize)
    trace.insert(1, column=None, value=flowType_arr)

    # Add ReqLen
    reqLen_arr = []
    for i in range(0, len(flowType_arr)):
        if flowType_arr[i] == 1 or flowType_arr[i] == 101:
            reqLen_arr.append(64)
        else:
            reqLen_arr.append(-1)
    trace.insert(5, column=None, value=reqLen_arr)
    trace.to_csv(newfilename, header=False, index=False)


if __name__ == '__main__':
    main()
