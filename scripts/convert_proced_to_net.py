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

    flowTypes = trace.iloc[:, 1].values
    traceSize=trace.shape[0]
    for i in range(0,traceSize):
        if flowTypes[i] ==1: #RREQ, change to 101
            flowTypes[i]=101
        else:
            flowTypes[i] = 100

    trace.iloc[:, 1] = flowTypes

    trace.to_csv(newfilename, header=False, index=False)


if __name__ == '__main__':
    main()
