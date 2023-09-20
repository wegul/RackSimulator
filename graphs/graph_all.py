import sys
import csv
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def main():
    plt.rcParams.update({'font.size': 20})

    graph_incast_cm_by_pktsize()
    graph_permutation_cm_by_pktsize()

    graph_datamining_06_cm_by_pktsize()
    graph_datamining_06_fct_by_pktsize()

    graph_websearch_06_cm_by_pktsize()
    graph_websearch_06_fct_by_pktsize()

    graph_incast_64B_cm_by_sram()
    graph_incast_1500B_cm_by_sram()

    graph_websearch_06_64B_cm_by_sram()
    graph_websearch_06_1500B_cm_by_sram()

    graph_incast_64B_cm_by_alg()
    graph_incast_1500B_cm_by_alg()

    graph_websearch_06_64B_cm_by_alg()
    graph_websearch_06_1500B_cm_by_alg()

def graph_incast_cm_by_pktsize():
    with open("data_new/incast_cm_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.Pktsize
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Normalized Cache Miss Rate")
        print("Incast Cache Misses: Capacity = 20%")

        plt.xticks(ind+width, pktsize, rotation=30)
        plt.ylim(0, 2.4)
        plt.yticks(np.arange(0, 2.4, 0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_permutation_cm_by_pktsize():
    with open("data_new/permutation_cm_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.Pktsize
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Normalized Cache Miss Rate")
        print("Permutation Cache Misses: Capacity = 20%")

        plt.xticks(ind+width, pktsize, rotation=30)
        plt.ylim(0,1.8)
        plt.yticks(np.arange(0,1.8,0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"), loc='upper right')
  
        plt.show()

def graph_datamining_06_cm_by_pktsize():
    with open("data_new/datamining_06_cm_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.pktsize
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Normalized Cache Miss Rate")
        print("Datamining Cache Misses: Load = 0.6, SRAM Capacity = 20%")

        plt.xticks(ind+width, pktsize, rotation=30)
        plt.ylim(0,2.4)
        plt.yticks(np.arange(0,2.4,0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_datamining_06_fct_by_pktsize():
    with open("data_new/datamining_06_fct_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.pktsize
        lru = df.LRU
        seer = df.Seer

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind-width/2, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width/2, seer, width, color='cyan', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Norm. Flow Completion Time")
        print("Datamining Flow Completion: Load = 0.6, SRAM Capacity = 20%")

        plt.xticks(ind, pktsize)
        plt.ylim(0,1.8)
        plt.yticks(np.arange(0,1.8,0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2), ("LRU", "Seer"))
  
        plt.show()

def graph_websearch_06_cm_by_pktsize():
    with open("data_new/websearch_06_cm_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.Pktsize
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Normalized Cache Miss Rate")
        print("Websearch Cache Misses: Load = 0.6, SRAM Capacity = 20%")

        plt.xticks(ind+width, pktsize, rotation=30)
        plt.ylim(0,2.0)
        plt.yticks(np.arange(0, 2.0, 0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_websearch_06_fct_by_pktsize():
    with open("data_new/websearch_06_fct_by_pktsize.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        pktsize = df.Pktsize
        lru = df.LRU
        seer = df.Seer

        N = pktsize.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind-width/2, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width/2, seer, width, color='cyan', edgecolor='black', zorder=3)

        plt.xlabel("Packet Size")
        plt.ylabel("Norm. Flow Completion Time")
        print("Websearch Flow Completion: Load = 0.6, SRAM Capacity = 20%")

        plt.xticks(ind, pktsize)
        plt.ylim(0, 1.8)
        plt.yticks(np.arange(0, 1.8, 0.2))
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2), ("LRU", "Seer"))
  
        plt.show()

def graph_incast_64B_cm_by_sram():
    with open("data_new/incast_64B_cm_by_sram.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        capacity = df.Capacity
        pct = df.Pct
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = capacity.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("SRAM Capacity")
        plt.ylabel("Number of Cache Misses")
        print("Incast Cache Misses: Packets = 64B")

        plt.xticks(ind+width, pct, rotation=30)
        plt.yticks(np.arange(0,9000,1000))

        current_values = plt.gca().get_yticks()
        plt.gca().set_yticklabels(["{:,.0f}K".format(x/1000) for x in current_values])
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_incast_1500B_cm_by_sram():
    with open("data_new/incast_1500B_cm_by_sram.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        capacity = df.Capacity
        pct = df.Pct
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = capacity.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("SRAM Capacity")
        plt.ylabel("Number of Cache Misses")
        print("Incast Cache Misses: Packets = 1500B")

        plt.xticks(ind+width, pct, rotation=30)
        plt.yticks(np.arange(0,5000,500))

        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_websearch_06_64B_cm_by_sram():
    with open("data_new/websearch_06_64B_cm_by_sram.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        capacity = df.Capacity
        pct = df.Pct
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = capacity.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("SRAM Capacity")
        plt.ylabel("Number of Cache Misses")
        print("Websearch Cache Misses: Packets = 64B, Load = 0.6")

        plt.xticks(ind+width, pct, rotation=30)
        plt.yticks(np.arange(0,8000,1000))

        current_values = plt.gca().get_yticks()
        plt.gca().set_yticklabels(["{:,.0f}K".format(x/1000) for x in current_values])
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_websearch_06_1500B_cm_by_sram():
    with open("data_new/websearch_06_1500B_cm_by_sram.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        capacity = df.Capacity
        pct = df.Pct
        lru = df.LRU
        seer = df.Seer
        belady = df.Belady

        N = capacity.size
        ind = np.arange(N)
        width = 0.25

        bar1 = plt.bar(ind, lru, width, color='red', edgecolor='black', zorder=3)
        bar2 = plt.bar(ind+width, seer, width, color='cyan', edgecolor='black', zorder=3)
        bar3 = plt.bar(ind+width*2, belady, width, color='yellow', edgecolor='black', zorder=3)

        plt.xlabel("SRAM Capacity")
        plt.ylabel("Number of Cache Misses")
        print("Websearch Cache Misses: Packets = 1500B, Load = 0.6")

        plt.xticks(ind+width, pct, rotation=30)
        plt.yticks(np.arange(0,18000,2000))

        current_values = plt.gca().get_yticks()
        plt.gca().set_yticklabels(["{:,.0f}K".format(x/1000) for x in current_values])
        plt.grid(axis='y', zorder=0)
        plt.legend((bar1, bar2, bar3), ("LRU", "Seer", "Belady"))
  
        plt.show()

def graph_incast_64B_cm_by_alg():
    with open("data_new/incast_64B_cm_by_alg.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        lru = df.LRU[0]
        lfu = df.LFU[0]
        arc = df.ARC[0]
        s3fifo = df.S3FIFO[0]
        sieve = df.SIEVE[0]
        seer = df.Seer[0]
        belady = df.Belady[0]

        x_vals = pd.Series([lru, lfu, arc, s3fifo, sieve, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "SIEVE", "Seer", "Belady"])

        fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

        ax1.spines['bottom'].set_visible(False)
        ax1.tick_params(axis='x', which='both', bottom=False)
        ax2.spines['top'].set_visible(False)

        bs = 1.5
        ts = 8.2

        ax2.set_ylim(0,bs)
        ax2.set_yticks(np.arange(0, bs, 0.2))
        ax1.set_ylim(ts-0.01,8.8)
        ax1.set_yticks(np.arange(ts, 8.81, 0.2))

        ax1.grid(axis='y', zorder=0)
        ax2.grid(axis='y', zorder=0)

        bar1 = ax1.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor='black', zorder=3)
        bar2 = ax2.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor='black', zorder=3)

        for tick in ax2.get_xticklabels():
            tick.set_rotation(30)
        d = 0.015
        kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
        ax1.plot((-d, +d), (-d, +d), **kwargs)
        ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
        kwargs.update(transform=ax2.transAxes)
        ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
        ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

        ax2.set_xlabel("Caching Algorithm")
        ax2.set_ylabel("Normalized Cache Miss Rate", loc='bottom')
        print("Incast Cache Misses: Packets = 64B, SRAM Capacity = 20%")

        plt.show()

def graph_incast_1500B_cm_by_alg():
    with open("data_new/incast_1500B_cm_by_alg.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        lru = df.LRU[0]
        lfu = df.LFU[0]
        arc = df.ARC[0]
        s3fifo = df.S3FIFO[0]
        sieve = df.SIEVE[0]
        seer = df.Seer[0]
        belady = df.Belady[0]

        x_vals = pd.Series([lru, lfu, arc, s3fifo, sieve, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "SIEVE", "Seer", "Belady"])

        fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

        ax1.spines['bottom'].set_visible(False)
        ax1.tick_params(axis='x', which='both', bottom=False)
        ax2.spines['top'].set_visible(False)

        bs = 1.6
        ts = 5.6

        ax1.grid(axis='y', zorder=0)
        ax2.grid(axis='y', zorder=0)

        ax2.set_ylim(0,bs)
        ax2.set_yticks(np.arange(0, bs, 0.2))
        ax1.set_ylim(ts-0.01,6.2)
        ax1.set_yticks(np.arange(ts, 6.21, 0.2))

        bar1 = ax1.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor='black', zorder=3)
        bar2 = ax2.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor='black', zorder=3)

        for tick in ax2.get_xticklabels():
            tick.set_rotation(30)

        d = 0.015
        kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
        ax1.plot((-d, +d), (-d, +d), **kwargs)
        ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
        kwargs.update(transform=ax2.transAxes)
        ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
        ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

        ax2.set_xlabel("Caching Algorithm")
        ax2.set_ylabel("Normalized Cache Miss Rate", loc='bottom')
        print("Incast Cache Misses: Packets = 1500B, SRAM Capacity = 20%")

        plt.show()

def graph_websearch_06_64B_cm_by_alg():
    with open("data_new/websearch_06_64B_cm_by_alg.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        lru = df.LRU[0]
        lfu = df.LFU[0]
        arc = df.ARC[0]
        s3fifo = df.S3FIFO[0]
        sieve = df.SIEVE[0]
        seer = df.Seer[0]
        belady = df.Belady[0]

        x_vals = pd.Series([lru, lfu, arc, s3fifo, sieve, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "SIEVE", "Seer", "Belady"])

        bar1 = plt.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor='black', zorder=3)

        plt.xticks(rotation=30)
        plt.grid(axis='y', zorder=0)

        plt.xlabel("Caching Algorithm")
        plt.ylabel("Normalized Cache Miss Rate", va='bottom')

        print("Websearch Cache Misses: Load = 0.6, Packets = 64B, SRAM Capacity = 20%")

        plt.show()

def graph_websearch_06_1500B_cm_by_alg():
    with open("data_new/websearch_06_1500B_cm_by_alg.csv", 'r') as csvfile:
        df = pd.read_csv(csvfile)
        lru = df.LRU[0]
        lfu = df.LFU[0]
        arc = df.ARC[0]
        s3fifo = df.S3FIFO[0]
        sieve = df.SIEVE[0]
        seer = df.Seer[0]
        belady = df.Belady[0]

        x_vals = pd.Series([lru, lfu, arc, s3fifo, sieve, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "SIEVE", "Seer", "Belady"])

        fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

        ax1.spines['bottom'].set_visible(False)
        ax1.tick_params(axis='x', which='both', bottom=False)
        ax2.spines['top'].set_visible(False)

        bs = 1.2
        ts = 2.6

        ax2.set_ylim(0,bs)
        ax2.set_yticks(np.arange(0,bs,0.2))
        ax1.set_ylim(ts-0.01,3.0)
        ax1.set_yticks(np.arange(ts, 3.01, 0.2))

        ax1.grid(axis='y', zorder=0)
        ax2.grid(axis='y', zorder=0)

        bar1 = ax1.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor="black", zorder=3)
        bar2 = ax2.bar(x_vals.index, x_vals.values, color=['red', 'blue', 'green', 'magenta', 'orange', 'cyan', 'yellow'], edgecolor="black", zorder=3)

        for tick in ax2.get_xticklabels():
            tick.set_rotation(30)
        d = 0.015
        kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
        ax1.plot((-d, +d), (-d, +d), **kwargs)
        ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
        kwargs.update(transform=ax2.transAxes)
        ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
        ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

        ax2.set_xlabel("Caching Algorithm")
        ax2.set_ylabel("Normalized Cache Miss Rate", loc='bottom')
        print("Websearch Cache Misses: Load = 0.6, Packets = 1500B, SRAM Capacity = 20%")

        plt.show()

# def main():
#     # Microbencmarks with default parameters (1500B 0.6 Load)
#     graph_incast_default()
#     graph_all_to_all_default()
#     graph_permutation_default()
#     # Microbenchmarks for only Seer data, varying by pkt size and load
#     graph_incast_seer_by_pktsize()
#     graph_all_to_all_seer_by_pktsize()
#     graph_permutation_seer_by_pktsize()
#     graph_all_to_all_seer_by_load()
#     # Realistic workloads with default parameters (1500B 0.6 Load)
#     graph_mixed_default_cm()
#     graph_mixed_default_fct()
#     # Realistic workloads for only Seer data, varying by load
#     graph_mixed_seer_1500B_cm_by_load()
#     graph_mixed_seer_1500B_fct_by_load()
#     graph_mixed_seer_64B_cm_by_load()
#     graph_mixed_seer_64B_fct_by_load()
#     # Zoomed into selected entries of microbenchmarks
#     graph_incast_selected_bar()
#     graph_all_to_all_selected_bar()
#     graph_permutation_selected_bar()
#     # Zoomed into selected entries of realistic workloads
#     graph_mixed_selected_bar_cm()
#     graph_mixed_selected_bar_fct()

# # Graphs incast workload 1500B packets 0.6 load cache misses
# def graph_incast_default():
#     with open("data/incast_1500B_06_cm.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         lru = df.lru
#         lfu = df.lfu
#         arc = df.arc
#         s3fifo = df.s3fifo
#         seer = df.seer
#         belady = df.belady

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.1

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [4, 5]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 5000
#         ts = 7000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,16000)
#         ax1.set_yticks(np.arange(ts, 16001, 2000))

#         bar1 = ax1.bar(ind, df.lru, width = 0.1, align='center')
#         bar2 = ax1.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar3 = ax1.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar4 = ax1.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar5 = ax1.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar6 = ax1.bar(ind+width*5, df.belady, width = 0.1, align='center')
#         bar7 = ax2.bar(ind, df.lru, width = 0.1, align='center')
#         bar8 = ax2.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar9 = ax2.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar10 = ax2.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar11 = ax2.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar12 = ax2.bar(ind+width*5, df.belady, width = 0.1, align='center')

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("Incast Workload Cache Misses (1500B)")
#         ax2.set_xticks(ind+2*width)
#         ax2.set_xticklabels(df.pct, rotation=45)
#         ax1.legend((bar1, bar2, bar3, bar4, bar5, bar6), ("LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"))

#         plt.show()

# # Graphs all-to-all workload 1500B packets 0.6 load cache misses
# def graph_all_to_all_default():
#     with open("data/all_to_all_1500B_06_cm.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         lru = df.lru
#         lfu = df.lfu
#         arc = df.arc
#         s3fifo = df.s3fifo
#         seer = df.seer
#         belady = df.belady

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.1

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [4, 5]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 5000
#         ts = 11000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,19000)
#         ax1.set_yticks(np.arange(ts, 19000, 2000))

#         bar1 = ax1.bar(ind, df.lru, width = 0.1, align='center')
#         bar2 = ax1.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar3 = ax1.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar4 = ax1.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar5 = ax1.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar6 = ax1.bar(ind+width*5, df.belady, width = 0.1, align='center')
#         bar7 = ax2.bar(ind, df.lru, width = 0.1, align='center')
#         bar8 = ax2.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar9 = ax2.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar10 = ax2.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar11 = ax2.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar12 = ax2.bar(ind+width*5, df.belady, width = 0.1, align='center')

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("All-to-All Workload Cache Misses (1500B)")
#         ax2.set_xticks(ind+2*width)
#         ax2.set_xticklabels(df.pct, rotation=45)
#         ax1.legend((bar1, bar2, bar3, bar4, bar5, bar6), ("LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"))

#         plt.show()

# # Graphs permutation workload 1500B packets 0.6 load cache misses
# def graph_permutation_default():
#     with open("data/permutation_1500B_06_cm.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         lru = df.lru
#         lfu = df.lfu
#         arc = df.arc
#         s3fifo = df.s3fifo
#         seer = df.seer
#         belady = df.belady

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.1

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [4, 5]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 20000
#         ts = 21000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,40000)
#         ax1.set_yticks(np.arange(ts, 40000, 4000))

#         bar1 = ax1.bar(ind, df.lru, width = 0.1, align='center')
#         bar2 = ax1.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar3 = ax1.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar4 = ax1.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar5 = ax1.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar6 = ax1.bar(ind+width*5, df.belady, width = 0.1, align='center')
#         bar7 = ax2.bar(ind, df.lru, width = 0.1, align='center')
#         bar8 = ax2.bar(ind+width, df.lfu, width = 0.1, align='center')
#         bar9 = ax2.bar(ind+width*2, df.arc, width = 0.1, align='center')
#         bar10 = ax2.bar(ind+width*3, df.s3fifo, width = 0.1, align='center')
#         bar11 = ax2.bar(ind+width*4, df.seer, width = 0.1, align='center')
#         bar12 = ax2.bar(ind+width*5, df.belady, width = 0.1, align='center')

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("Permutation Workload Cache Misses (1500B)")
#         ax2.set_xticks(ind+2*width)
#         ax2.set_xticklabels(pct, rotation=45)
#         ax1.legend((bar1, bar2, bar3, bar4, bar5, bar6), ("LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"))

#         plt.show()

# # Graphs incast workload seer performance based on packet size
# def graph_incast_seer_by_pktsize():
#     with open("data/incast_seer_cm_pktsize.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         pkt_1500 = df["1500B"]
#         pkt_512 = df["512B"]
#         pkt_256 = df["256B"]
#         pkt_128 = df["128B"]
#         pkt_64 = df["64B"]

#         norm_1500 = [x / pkt_1500[2] for x in pkt_1500]
#         norm_512 = [x / pkt_512[2] for x in pkt_512]
#         norm_256 = [x / pkt_256[2] for x in pkt_256]
#         norm_128 = [x / pkt_128[2] for x in pkt_128]
#         norm_64 = [x / pkt_64[2] for x in pkt_64]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.15

#         bar1 = plt.bar(ind, norm_1500, width)
#         bar2 = plt.bar(ind+width, norm_512, width)
#         bar3 = plt.bar(ind+width*2, norm_256, width)
#         bar4 = plt.bar(ind+width*3, norm_128, width)
#         bar5 = plt.bar(ind+width*4, norm_64, width)


#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("Incast Workload Seer Cache Misses by Packet Size")

#         plt.xticks(ind+width*2, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3, bar4, bar5), ("1500B", "512B", "256B", "128B", "64B"))
  
#         plt.show()

# # Graphs all-to-all workload seer performance based on packet size
# def graph_all_to_all_seer_by_pktsize():
#     with open("data/all_to_all_seer_cm_pktsize.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         pkt_1500 = df["1500B"]
#         pkt_512 = df["512B"]
#         pkt_256 = df["256B"]
#         pkt_128 = df["128B"]
#         pkt_64 = df["64B"]

#         norm_1500 = [x / pkt_1500[2] for x in pkt_1500]
#         norm_512 = [x / pkt_512[2] for x in pkt_512]
#         norm_256 = [x / pkt_256[2] for x in pkt_256]
#         norm_128 = [x / pkt_128[2] for x in pkt_128]
#         norm_64 = [x / pkt_64[2] for x in pkt_64]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.15

#         bar1 = plt.bar(ind, norm_1500, width)
#         bar2 = plt.bar(ind+width, norm_512, width)
#         bar3 = plt.bar(ind+width*2, norm_256, width)
#         bar4 = plt.bar(ind+width*3, norm_128, width)
#         bar5 = plt.bar(ind+width*4, norm_64, width)


#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("All-to-All Workload Seer Cache Misses by Packet Size")

#         plt.xticks(ind+width*2, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3, bar4, bar5), ("1500B", "512B", "256B", "128B", "64B"))
  
#         plt.show()

# # Graphs permutation workload seer performance based on packet size
# def graph_permutation_seer_by_pktsize():
#     with open("data/permutation_seer_cm_pktsize.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         pkt_1500 = df["1500B"]
#         pkt_512 = df["512B"]
#         pkt_256 = df["256B"]
#         pkt_128 = df["128B"]
#         pkt_64 = df["64B"]

#         norm_1500 = [x / pkt_1500[2] for x in pkt_1500]
#         norm_512 = [x / pkt_512[2] for x in pkt_512]
#         norm_256 = [x / pkt_256[2] for x in pkt_256]
#         norm_128 = [x / pkt_128[2] for x in pkt_128]
#         norm_64 = [x / pkt_64[2] for x in pkt_64]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.15

#         bar1 = plt.bar(ind, norm_1500, width)
#         bar2 = plt.bar(ind+width, norm_512, width)
#         bar3 = plt.bar(ind+width*2, norm_256, width)
#         bar4 = plt.bar(ind+width*3, norm_128, width)
#         bar5 = plt.bar(ind+width*4, norm_64, width)


#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("Permutation Workload Seer Cache Misses by Packet Size")

#         plt.xticks(ind+width*2, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3, bar4, bar5), ("1500B", "512B", "256B", "128B", "64B"))
  
#         plt.show()

# # Graphs all-to-all workload seer performance based on network load
# def graph_all_to_all_seer_by_load():
#     with open("data/all_to_all_seer_cm_load.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         load3 = df["load0.3"]
#         load6 = df["load0.6"]
#         load9 = df["load0.9"]

#         norm_load3 = [x / load3[2] for x in load3]
#         norm_load6 = [x / load6[2] for x in load6]
#         norm_load9 = [x / load9[2] for x in load9]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.25

#         bar1 = plt.bar(ind, norm_load3, width)
#         bar2 = plt.bar(ind+width, norm_load6, width)
#         bar3 = plt.bar(ind+width*2, norm_load9, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("All-to-All Workload Seer Cache Misses by Network Load")

#         plt.xticks(ind+width, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3), ("Load 0.3", "Load 0.6", "Load 0.9"))
  
#         plt.show()

# # Graphs Aditya/mixed workload 1500B packets 0.6 load cache misses
# def graph_mixed_default_cm():
#     with open("data/mixed_1500B_06_cm.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         lru = df.lru
#         lfu = df.lfu
#         arc = df.arc
#         s3fifo = df.s3fifo
#         seer = df.seer
#         belady = df.belady

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.15

#         bar1 = plt.bar(ind, lru, width)
#         bar2 = plt.bar(ind+width, lfu, width)
#         bar3 = plt.bar(ind+width*2, arc, width)
#         bar4 = plt.bar(ind+width*3, s3fifo, width)
#         bar5 = plt.bar(ind+width*4, seer, width)
#         bar6 = plt.bar(ind+width*5, belady, width)


#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Cache Misses")
#         plt.title("Mixed Workload Cache Misses (1500B, Load 0.6)")

#         plt.xticks(ind+width*2, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3, bar4, bar5, bar6), ("LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"))
  
#         plt.show()

# # Graphs Aditya/mixed workload 1500B packets 0.6 load flow completion times
# def graph_mixed_default_fct():
#     with open("data/mixed_1500B_06_fct.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         lru = [x / 50 for x in df.lru]
#         lfu = [x / 50 for x in df.lfu]
#         arc = [x / 50 for x in df.arc]
#         s3fifo = [x / 50 for x in df.s3fifo]
#         seer = [x / 50 for x in df.seer]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.15

#         bar1 = plt.bar(ind, lru, width)
#         bar2 = plt.bar(ind+width, lfu, width)
#         bar3 = plt.bar(ind+width*2, arc, width)
#         bar4 = plt.bar(ind+width*3, s3fifo, width)
#         bar5 = plt.bar(ind+width*4, seer, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Slowdown")
#         plt.title("Mixed Workload Flow Completion Time (1500B, Load 0.6)")

#         plt.xticks(ind+width*2, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3, bar4, bar5), ("LRU", "LFU", "ARC", "S3-FIFO", "Seer"))
  
#         plt.show()

# # Graphs Aditya/mixed workload seer performance based on network load at 1500B pkts
# def graph_mixed_seer_1500B_cm_by_load():
#     with open("data/mixed_1500B_cm_load.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         load3 = df["load0.3"]
#         load6 = df["load0.6"]
#         load9 = df["load0.9"]

#         norm_load3 = [x / load3[2] for x in load3]
#         norm_load6 = [x / load6[2] for x in load6]
#         norm_load9 = [x / load9[2] for x in load9]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.25

#         bar1 = plt.bar(ind, norm_load3, width)
#         bar2 = plt.bar(ind+width, norm_load6, width)
#         bar3 = plt.bar(ind+width*2, norm_load9, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("Mixed Workload Seer Cache Misses 1500B by Network Load")

#         plt.xticks(ind+width, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3), ("Load 0.3", "Load 0.6", "Load 0.9"))
  
#         plt.show()

# # Graphs Aditya/mixed workload seer performance based on network load at 1500B pkts
# def graph_mixed_seer_1500B_fct_by_load():
#     with open("data/mixed_1500B_fct_load.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         load3 = df["load0.3"] / 50
#         load6 = df["load0.6"] / 50
#         load9 = df["load0.9"] / 50

#         norm_load3 = [x / load3[2] for x in load3]
#         norm_load6 = [x / load6[2] for x in load6]
#         norm_load9 = [x / load9[2] for x in load9]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.25

#         bar1 = plt.bar(ind, norm_load3, width)
#         bar2 = plt.bar(ind+width, norm_load6, width)
#         bar3 = plt.bar(ind+width*2, norm_load9, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Slowdown")
#         plt.title("Mixed Workload Seer Flow Completion Time 1500B by Network Load")

#         plt.xticks(ind+width, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3), ("Load 0.3", "Load 0.6", "Load 0.9"))
  
#         plt.show()

# # Graphs Aditya/mixed workload seer performance based on network load at 64B pkts
# def graph_mixed_seer_64B_cm_by_load():
#     with open("data/mixed_64B_cm_load.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         load3 = df["load0.3"]
#         load6 = df["load0.6"]
#         load9 = df["load0.9"]

#         norm_load3 = [x / load3[2] for x in load3]
#         norm_load6 = [x / load6[2] for x in load6]
#         norm_load9 = [x / load9[2] for x in load9]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.25

#         bar1 = plt.bar(ind, norm_load3, width)
#         bar2 = plt.bar(ind+width, norm_load6, width)
#         bar3 = plt.bar(ind+width*2, norm_load9, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Cache Misses")
#         plt.title("Mixed Workload Seer Cache Misses 64B by Network Load")

#         plt.xticks(ind+width, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3), ("Load 0.3", "Load 0.6", "Load 0.9"))
  
#         plt.show()

# # Graphs Aditya/mixed workload seer performance based on network load at 64B pkts
# def graph_mixed_seer_64B_fct_by_load():
#     with open("data/mixed_64B_fct_load.csv", 'r') as csvfile:
#         df = pd.read_csv(csvfile)
#         capacity = df.capacity
#         pct = df.pct
#         load3 = df["load0.3"] / 50
#         load6 = df["load0.6"] / 50
#         load9 = df["load0.9"] / 50

#         norm_load3 = [x / load3[2] for x in load3]
#         norm_load6 = [x / load6[2] for x in load6]
#         norm_load9 = [x / load9[2] for x in load9]

#         N = capacity.size
#         ind = np.arange(N)
#         width = 0.25

#         bar1 = plt.bar(ind, norm_load3, width)
#         bar2 = plt.bar(ind+width, norm_load6, width)
#         bar3 = plt.bar(ind+width*2, norm_load9, width)

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Normalized Slowdown")
#         plt.title("Mixed Workload Seer Flow Completion Time 64B by Network Load")

#         plt.xticks(ind+width, pct, rotation=45)
#         plt.legend((bar1, bar2, bar3), ("Load 0.3", "Load 0.6", "Load 0.9"))
  
#         plt.show()

# # Graphs incast workload 1500B packets 0.6 load cache misses zoomed in on 22% SRAM usage 
# def graph_incast_selected_bar():
#     with open("data/incast_1500B_06_cm.csv", 'r') as csvfile:
#         selected = 1

#         df = pd.read_csv(csvfile)
#         capacity = df.capacity[selected]
#         pct = df.pct[selected]
#         lru = df.lru[selected]
#         lfu = df.lfu[selected]
#         arc = df.arc[selected]
#         s3fifo = df.s3fifo[selected]
#         seer = df.seer[selected]
#         belady = df.belady[selected]

#         x_vals = pd.Series([lru, lfu, arc, s3fifo, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"])

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 4000
#         ts = 12000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,14000)
#         ax1.set_yticks(np.arange(ts, 14001, 1000))

#         bar1 = ax1.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])
#         bar2 = ax2.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("Incast Workload Cache Misses (1500B) at 22% SRAM usage")

#         plt.show()

# # Graphs all-to-all workload 1500B packets 0.6 load cache misses zoomed in on 12% SRAM usage 
# def graph_all_to_all_selected_bar():
#     with open("data/all_to_all_1500B_06_cm.csv", 'r') as csvfile:
#         selected = 1

#         df = pd.read_csv(csvfile)
#         capacity = df.capacity[selected]
#         pct = df.pct[selected]
#         lru = df.lru[selected]
#         lfu = df.lfu[selected]
#         arc = df.arc[selected]
#         s3fifo = df.s3fifo[selected]
#         seer = df.seer[selected]
#         belady = df.belady[selected]

#         x_vals = pd.Series([lru, lfu, arc, s3fifo, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"])

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 5000
#         ts = 16000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,18000)
#         ax1.set_yticks(np.arange(ts, 18001, 1000))

#         bar1 = ax1.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])
#         bar2 = ax2.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("All-to-All Workload Cache Misses (1500B) at 12% SRAM usage")

#         plt.show()

# # Graphs permutation workload 1500B packets 0.6 load cache misses zoomed in on 20% SRAM usage 
# def graph_permutation_selected_bar():
#     with open("data/permutation_1500B_06_cm.csv", 'r') as csvfile:
#         selected = 1

#         df = pd.read_csv(csvfile)
#         capacity = df.capacity[selected]
#         pct = df.pct[selected]
#         lru = df.lru[selected]
#         lfu = df.lfu[selected]
#         arc = df.arc[selected]
#         s3fifo = df.s3fifo[selected]
#         seer = df.seer[selected]
#         belady = df.belady[selected]

#         x_vals = pd.Series([lru, lfu, arc, s3fifo, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"])

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 7000
#         ts = 25000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,27000)
#         ax1.set_yticks(np.arange(ts, 27001, 1000))

#         bar1 = ax1.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])
#         bar2 = ax2.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("Permutation Workload Cache Misses (1500B) at 20% SRAM usage")

#         plt.show()

# # Graphs Aditya/mixed workload 1500B packets 0.6 load cache misses zoomed in on 20% SRAM usage 
# def graph_mixed_selected_bar_cm():
#     with open("data/mixed_1500B_06_cm.csv", 'r') as csvfile:
#         selected = 1

#         df = pd.read_csv(csvfile)
#         capacity = df.capacity[selected]
#         pct = df.pct[selected]
#         lru = df.lru[selected]
#         lfu = df.lfu[selected]
#         arc = df.arc[selected]
#         s3fifo = df.s3fifo[selected]
#         seer = df.seer[selected]
#         belady = df.belady[selected]

#         x_vals = pd.Series([lru, lfu, arc, s3fifo, seer, belady], index=["LRU", "LFU", "ARC", "S3-FIFO", "Seer", "Belady"])

#         fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 3]})

#         ax1.spines['bottom'].set_visible(False)
#         ax1.tick_params(axis='x', which='both', bottom=False)
#         ax2.spines['top'].set_visible(False)

#         bs = 6000
#         ts = 13000

#         ax2.set_ylim(0,bs)
#         ax1.set_ylim(ts,15000)
#         ax1.set_yticks(np.arange(ts, 15001, 1000))

#         bar1 = ax1.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])
#         bar2 = ax2.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple', 'tab:brown'])

#         for tick in ax2.get_xticklabels():
#             tick.set_rotation(0)
#         d = 0.015
#         kwargs = dict(transform = ax1.transAxes, color='k', clip_on=False)
#         ax1.plot((-d, +d), (-d, +d), **kwargs)
#         ax1.plot((1-d, 1+d), (-d, +d), **kwargs)
#         kwargs.update(transform=ax2.transAxes)
#         ax2.plot((-d, +d), (1-d, 1+d), **kwargs)
#         ax2.plot((1-d, 1+d), (1-d, 1+d), **kwargs)

#         ax2.set_xlabel("% of flows in SRAM")
#         ax2.set_ylabel("Cache Misses")
#         ax1.set_title("Mixed Workload Cache Misses (1500B) at 20% SRAM usage")

#         plt.show()

# # Graphs Aditya/mixed workload 1500B packets 0.6 load flow completion time zoomed in on 20% SRAM usage 
# def graph_mixed_selected_bar_fct():
#     with open("data/mixed_1500B_06_fct.csv", 'r') as csvfile:
#         selected = 1

#         df = pd.read_csv(csvfile)
#         capacity = df.capacity[selected]
#         pct = df.pct[selected]
#         lru = df.lru[selected] / 50
#         lfu = df.lfu[selected] / 50
#         arc = df.arc[selected] / 50
#         s3fifo = df.s3fifo[selected] / 50
#         seer = df.seer[selected] / 50

#         x_vals = pd.Series([lru, lfu, arc, s3fifo, seer], index=["LRU", "LFU", "ARC", "S3-FIFO", "Seer"])

#         bar1 = plt.bar(x_vals.index, x_vals.values, color=['tab:blue', 'tab:orange', 'tab:green', 'tab:red', 'tab:purple'])

#         plt.xlabel("% of flows in SRAM")
#         plt.ylabel("Slowdown")
#         plt.title("Mixed Workload Flow Completion Time (1500B) at 20% SRAM usage")

#         plt.show()

if __name__ == '__main__' : main()
