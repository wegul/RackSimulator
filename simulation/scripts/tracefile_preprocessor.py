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
    parser.add_argument('-fo', required=True)
    args = parser.parse_args()
    slot_time = (float(args.c) * 8.0) / float(args.b) * 1e-9
    filename = args.fi
    isMem = (int)(args.ism)
    trace = pd.read_csv(filename, header=None)

    # Change second to timeslot_num
    timeslots = []
    start_time_arr = trace.iloc[:, -1].values
    for time in start_time_arr:
        slot = (int)(time/slot_time)
        timeslots.append((str)(slot))
    trace.iloc[:, -1] = pd.Series(timeslots)
    # pathList = filename.split('/')
    # newfilename = pathList[-1]
    # newfilename = "./proced_workloads/"+newfilename[:-4]+".proced.csv"
    newfilename = args.fo

    # Map 512 to 64
    src_arr = trace.iloc[:, 1].values
    dst_arr = trace.iloc[:, 2].values
    new_src = []
    new_dst = []
    for i in range(0, trace.shape[0]):
        a = src_arr[i] % 64
        b = dst_arr[i] % 64
        if a == b:
            b = (b+1) % 64
        new_src.append(a)
        new_dst.append(b)
    trace.iloc[:, 1] = pd.Series(new_src)
    trace.iloc[:, 2] = pd.Series(new_dst)

    # add isMem
    trace.insert(1, column=None, value=isMem)

    # Add MemType
    flowSize = trace.iloc[:, 4].values
    memType_arr = []
    if isMem == 1:
        for i in range(0, trace.shape[0]):
            memType = random.randint(0, 1)
            if memType == 1:
                memType = 999
            else:
                flowSize[i] = 24
            memType_arr.append(memType)
    else:
        memType_arr = [-1]*trace.shape[0]
    trace.iloc[:, 4] = pd.Series(flowSize)
    trace.insert(2, column=None, value=memType_arr)

    # Add ReqLen
    reqLen_arr = []
    for i in range(0, len(memType_arr)):
        if memType_arr[i] == 0:
            reqLen_arr.append(flowSize[i])
        else:
            reqLen_arr.append(-1)
    trace.insert(6, column=None, value=reqLen_arr)
    trace.iloc[0:8000, :].to_csv(newfilename, header=False, index=False)


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
