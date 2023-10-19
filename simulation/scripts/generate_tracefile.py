import sys
import csv
import math
import argparse
import random


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-n', default=4)  # number of hosts
    parser.add_argument('-x', default=1)
    parser.add_argument('-t', default=10000)  # size of flow
    args = parser.parse_args()

    filename = args.f
    hosts = int(args.n)
    host_to_host_flows = int(args.x)
    # total_flows = int(args.t)

    flow_id = 0

    with open(filename, 'w') as csvfile:
        for src in range(hosts):
            for dst in range(hosts):
                if (dst != src):
                    for flow in range(host_to_host_flows):
                        isMem = random.randint(0, 1)
                        w_str = f'{flow_id},{isMem},{src},{dst},10000,0\n'
                        csvfile.write(w_str)
                        flow_id += 1
        # end_str = f'{flow_id},0,0,10000000,0.0005'
        # csvfile.write(end_str)


if __name__ == '__main__':
    main()
