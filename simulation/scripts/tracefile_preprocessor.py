import os
import sys
import re
import argparse
import math


def main():
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
