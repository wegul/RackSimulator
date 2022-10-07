import os
import sys
import re
import argparse
import math

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-b', default=100) # bandwidth in Gbps
    parser.add_argument('-c', default=1500) #packet size, in bytes
    parser.add_argument('-o', default=0) #header overhead, in bytes
    args = parser.parse_args()

    filename = args.f
    slot_time = (float(args.c) * 8.0) / float(args.b)
    usable_packet_size_bits = (int(args.c) - int(args.o)) * 8

    inp = open(filename, 'r')
    out = open(filename+".processed", 'w')

    for line in inp:
        tokens = line.split(',')
        out.write(tokens[0].strip())
        out.write(',')
        out.write(tokens[1].strip())
        out.write(',')
        out.write(tokens[2].strip())
        out.write(',')
        flowsize = int(float(tokens[3].strip()))
        out.write(str(flowsize))
        out.write(',')
        pkts = int(math.ceil((float(tokens[3].strip())*8)/usable_packet_size_bits))
        out.write(str(pkts))
        out.write(',')
        timeslots = int(math.ceil((float(tokens[4].strip()) * 1e9)/slot_time))
        out.write(str(timeslots))
        out.write('\n')

    inp.close()
    out.close()

if __name__ == '__main__' : main()
