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
    parser.add_argument('-b', required=True)  # bandwidth in Gbps
    parser.add_argument('-c', default=8)  # packet size, in bytes
    parser.add_argument('-ism', required=True)  # isMem
    parser.add_argument('-fo', required=True)
    args = parser.parse_args()
    slot_time = (float(args.c) * 8.0) / float(args.b) * 1e-9
    filename = args.fi
    newfilename = args.fo

    isMem = (int)(args.ism)
    trace = pd.read_csv(filename, header=None)

    # Change second to timeslot_num
    timeslots = []
    start_time_arr = trace.iloc[:, -1].values
    for time in start_time_arr:
        slot = (int)(time/slot_time)
        timeslots.append((str)(slot))
    trace.iloc[:, -1] = pd.Series(timeslots)

    # # Map 512 to 64
    # src_arr = trace.iloc[:, 1].values
    # dst_arr = trace.iloc[:, 2].values
    # new_src = []
    # new_dst = []
    # for i in range(0, trace.shape[0]):
    #     a = src_arr[i] % 64
    #     b = dst_arr[i] % 64
    #     if a == b:
    #         b = (b+1) % 64
    #     new_src.append(a)
    #     new_dst.append(b)
    # trace.iloc[:, 1] = pd.Series(new_src)
    # trace.iloc[:, 2] = pd.Series(new_dst)

    # add flowType
    # trace.insert(1, column=None, value=isMem)

    # Add flowType
    flowSize = trace.iloc[:, 3].values.copy()
    old_flowSize = trace.iloc[:, 3].values.copy()
    flowType_arr = []
    if isMem == 1:
        for i in range(0, trace.shape[0]):
            flowType = random.randint(2, 3)
            if flowType == 2:  # Change to RREQ
                flowType = 1
                flowSize[i] = 16
            flowType_arr.append(flowType)
    else:
        flowType_arr = [100]*trace.shape[0]
    trace.iloc[:, 3] = pd.Series(flowSize)
    trace.insert(1, column=None, value=flowType_arr)

    # Add ReqLen
    reqLen_arr = []
    for i in range(0, len(flowType_arr)):
        if flowType_arr[i] == 1:
            reqLen_arr.append(old_flowSize[i])
        else:
            reqLen_arr.append(-1)
    trace.insert(5, column=None, value=reqLen_arr)
    trace.to_csv(newfilename, header=False, index=False)


def main_obsolete():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-b', default=100)  # bandwidth in Gbps
    parser.add_argument('-c', default=64)  # packet size, in bytes
    parser.add_argument('-o', default=0)  # header overhead, in bytes
    args = parser.parse_args()

    filename = args.f
    slot_time = (float(args.c) * 8.0) / float(args.b)
    eth_usable_packet_size_bits = 1400 * 8
    mem_usable_packet_size_bits = 64*8

    inp = open(filename, 'r')
    out = open(filename+".processed", 'w')

    for line in inp:
        tokens = line.split(',')
        out.write(tokens[0].strip())
        out.write(',')
        isMem = (int)(tokens[1].strip())
        out.write(str(isMem))
        out.write(',')
        out.write(tokens[2].strip())
        out.write(',')
        out.write(tokens[3].strip())
        out.write(',')
        flowsize = int(float(tokens[4].strip()))
        out.write(str(flowsize))
        out.write(',')
        if isMem == 1:
            pkts = int(math.floor((float(flowsize)*8) /
                       (mem_usable_packet_size_bits)))
        else:
            pkts = int(math.floor((float(flowsize)*8) /
                       (eth_usable_packet_size_bits)))
        out.write(str(pkts))
        out.write(',')
        timeslots = int(math.ceil((float(tokens[5].strip()) * 1e9)/slot_time))
        out.write(str(timeslots))
        out.write('\n')

    inp.close()
    out.close()


if __name__ == '__main__':
    main()
