# this script mark all flows of given file as isMem-0.
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
    parser.add_argument('-fo', required=True)

    args = parser.parse_args()
    filename = args.fi
    outfilename = args.fo
    trace = pd.read_csv(filename, header=None)
    flowType = trace.iloc[:, 1]
    for i in range(0, trace.shape[0]):
        # reverse
        if flowType[i] == 3:
            trace.iloc[:, 1] = 1  # RREQ
            tmp = trace.iloc[:, 4].copy()  # old flowSize
            trace.iloc[:, 4] = 16  # flowSize
            trace.iloc[:, 5] = tmp  # reqLen

    trace.to_csv(outfilename, header=False, index=False)


if __name__ == '__main__':
    main()
