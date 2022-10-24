import sys
import csv
import argparse
import numpy as np
import matplotlib.pyplot as plt

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-s', default=16)
    parser.add_argument('-t', default=32)
    parser.add_argument('-n', default=16)
    parser.add_argument('-l', default=120)
    args = parser.parse_args()

    filename = args.f
    spines = int(args.s)
    tors = int(args.t)
    hosts = int(args.n)
    timeslot_length = float(args.l) / 1e9

    spine_ts = [[[] for j in range(tors)] for i in range(spines)]
    upstream_tor_ts = [[[] for j in range(spines)] for i in range(tors)]
    downstream_tor_ts = [[[] for j in range(hosts)] for i in range(tors)]

    with open(filename, 'r') as csvfile:
        cr = csv.reader(csvfile, delimiter = ',')
        for row in cr:
            row.pop()
            row = list(map(int, row))

            spines_data = row[0:spines*tors]
            upstream_tors_data = row[spines*tors:spines*tors*2]
            downstream_tors_data = row[spines*tors*2:]
            
            for i in range(spines):
                spine_queue_data = spines_data[i*tors:(i+1)*tors]
                for j in range(tors):
                    spine_ts[i][j].append(spine_queue_data[j] / 1000)
            
            for i in range(tors):
                upstream_queue_data = upstream_tors_data[i*spines:(i+1)*spines]
                for j in range(spines):
                    upstream_tor_ts[i][j].append(upstream_queue_data[j] / 1000)

                downstream_queue_data = downstream_tors_data[i*hosts:(i+1)*hosts]
                for j in range(hosts):
                    downstream_tor_ts[i][j].append(downstream_queue_data[j] / 1000)
    
    num_timeseries = len(spine_ts[0][0])
    print(str(num_timeseries) + " ts")

    times = np.arange(0, num_timeseries*timeslot_length, timeslot_length)

    spine_fig, spine_axs = plt.subplots(spines / 4, 4)
    spine_fig.suptitle("Spine queue lengths")
    spine_fig.tight_layout()

    for spine in range(spines):
        x_idx = spine / 4
        y_idx = spine % 4
        title = "Spine " + str(spine)
        spine_axs[x_idx, y_idx].set_title(title)
        for tor in range(tors):
            spine_axs[x_idx, y_idx].plot(times, spine_ts[spine][tor])

    for ax in spine_axs.flat:
        ax.set(xlabel='Time (s)', ylabel='Queue Length (KB)')
    
    plt.figure(1)

    up_tor_fig, up_tor_axs = plt.subplots(tors / 4, 4)
    up_tor_fig.suptitle("Upstream ToR queue lengths")
    up_tor_fig.tight_layout()

    for tor in range(tors):
        x_idx = tor / 4
        y_idx = tor % 4
        title = "ToR " + str(tor)
        up_tor_axs[x_idx, y_idx].set_title(title)
        for spine in range(spines):
            up_tor_axs[x_idx, y_idx].plot(times, upstream_tor_ts[tor][spine])

    for ax in up_tor_axs.flat:
        ax.set(xlabel='Time (s)', ylabel='Queue Length (KB)')

    plt.figure(2)

    down_tor_fig, down_tor_axs = plt.subplots(tors / 4, 4)
    down_tor_fig.suptitle("Downstream ToR queue lengths")
    down_tor_fig.tight_layout()

    for tor in range(tors):
        x_idx = tor / 4
        y_idx = tor % 4
        title = "ToR " + str(tor)
        down_tor_axs[x_idx, y_idx].set_title(title)
        for host in range(hosts):
            down_tor_axs[x_idx, y_idx].plot(times, downstream_tor_ts[tor][host])

    for ax in down_tor_axs.flat:
        ax.set(xlabel='Time (s)', ylabel='Queue Length (KB)')
    
    plt.figure(3)
    plt.show()

if __name__ == '__main__' : main()
