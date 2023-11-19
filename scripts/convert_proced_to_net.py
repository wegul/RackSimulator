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
    parser.add_argument('-c', default=8)  # packet size, in bytes
    parser.add_argument('-fo', required=True)
    args = parser.parse_args()
    filename = args.fi
    newfilename = args.fo
    trace = pd.read_csv(filename, header=None)

    # Change ReqLen to FlowSize
    reqLen = trace.iloc[:, 5].values
    old_flowSize = trace.iloc[:, 4].values
    for i in range(0, len(reqLen)):
        if reqLen[i] > 0:
            old_flowSize[i] = reqLen[i]
            reqLen[i] = -1

    # Leave a mem mark
    oldflowType_arr = trace.iloc[:, 1].values.copy()
    for i in range(0, len(oldflowType_arr)):
        if oldflowType_arr[i] != 100:
            reqLen[i] = 666

    # Change flowType to NET_TYPE
    flowType_arr = [100]*trace.shape[0]  # NET_TYPE
    trace.iloc[:, 1] = flowType_arr

    trace.to_csv(newfilename, header=False, index=False)


if __name__ == '__main__':
    main()
