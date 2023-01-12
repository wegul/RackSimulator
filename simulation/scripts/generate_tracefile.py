import sys
import csv
import math
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-n', default=144)
    parser.add_argument('-x', default=1)
    parser.add_argument('-t', default=1000000)
    args = parser.parse_args()

    filename = args.f
    hosts = int(args.n)
    host_to_host_flows = int(args.x)
    #total_flows = int(args.t)
    #host_to_host_flows = math.ceil(total_flows / (hosts * (hosts - 1)))

    flow_id = 0

    with open(filename, 'w') as csvfile:
        for src in range(hosts):
            for dst in range(hosts):
                if (dst != src):
                    for flow in range(host_to_host_flows):
                        w_str = f'{flow_id},{src},{dst},10000000,0\n'
                        csvfile.write(w_str)
                        flow_id += 1
        end_str = f'{flow_id},0,0,10000000,0.0005'
        csvfile.write(end_str)

if __name__ == '__main__' : main()