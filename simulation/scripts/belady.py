import sys
import csv
import os
import argparse
import pandas as pd
import numpy as np

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', required=True)
    parser.add_argument('-d', default=1)
    parser.add_argument('-m', default=100000)
    parser.add_argument('-n', default=10)
    parser.add_argument('-x', default=100)
    parser.add_argument('-s', default=10)
    parser.add_argument('-p', default=16)
    parser.add_argument('-b', action='store_true')
    args = parser.parse_args()

    filepath = args.f 
    cache_size = int(args.s) # number of flow_ids the SRAM can hold
    dram_access_time = int(args.d) # how many timeslots a single DRAM access takes
    max_timeslot = int(args.m) # what timeslot to end running Belady on
    min_sram_size = int(args.n) # minimum SRAM size
    max_sram_size = int(args.x) # maximum SRAM size
    step_size = int(args.s) # how much to increase SRAM size per experiment
    num_of_ports = int(args.p) # max number of ports the switch has. Will still work if exceeds actual amount, but cannot be under
    practical_belady = args.b # True: use realistic version of Belady that will only use packets after their creation

    for sram_size in range(min_sram_size, max_sram_size, step_size):
        total_cache_hits = 0
        total_cache_misses = 0
        print("Running for SRAM of size ", sram_size)
        for filename in os.listdir(filepath): # open each file in provided filepath
            if (os.path.isfile(os.path.join(filepath, filename))):
                with open(os.path.join(filepath, filename), 'r') as file:
                    print("Opening file: ", filename)

                    is_tor = False
                    if filename.find("tor") < 0: # Spine file
                        df = pd.read_csv(file, sep=",", header=0, usecols=[0, 3, 4, 5], names=['flow_id', 'port', 'arrival_time', 'creation_time'])
                    else: # tor file
                        is_tor = True
                        df = pd.read_csv(file, sep=",", header=0, usecols=[0, 3, 4, 5, 6], names=['flow_id', 'port', 'arrival_time', 'creation_time', 'direction'])
                        direction = df.direction.tolist()

                    flow_id = df.flow_id.tolist()
                    port = df.port.tolist()
                    arrival = df.arrival_time.tolist()
                    creation = df.creation_time.tolist()

                    if is_tor: # for ToR switches, remove upstream packets (tagged with up in direction list)
                        for i in reversed(range(len(direction))):
                            if direction[i].find("up") > -1:
                                flow_id.pop(i)
                                port.pop(i)
                                arrival.pop(i)
                                direction.pop(i)
                                creation.pop(i)

                    sram = list(range(sram_size)) # initialize SRAM
                    sram_lock = 0
                    dram_accessing = 0
                    placement_idx = 0

                    port_queues = [] # initialize switch packet queues
                    for i in range(num_of_ports):
                        port_list = []
                        port_queues.append(port_list)

                    cache_hits = 0
                    prefetches = 0
                    cache_misses = 0

                    curr_idx = 0

                    end_time = max_timeslot
                    
                    if (len(arrival) ==  0):
                        end_time = 0
                    elif arrival[-1] < max_timeslot:
                        end_time = arrival[-1]

                    for t in range(end_time):
                        ## Access packets that have arrived
                        while curr_idx < len(flow_id) and arrival[curr_idx] <= t:
                            arrival_port = port[curr_idx]
                            port_queues[arrival_port].append(flow_id[curr_idx])
                            curr_idx += 1

                        # Attempt to access packet
                        for p in range(num_of_ports):
                            if len(port_queues[p]) > 0:
                                if port_queues[p][0] in sram: # cache hit
                                    #print("Cache hit flow ", port_queues[p][0])
                                    port_queues[p].pop(0)
                                    cache_hits += 1
                                else: # cache miss
                                    if not sram_lock:
                                        #cache_hits -= 1 # to make up for subsequent "cache hit" when it is brought up to memory
                                        cache_misses += 1
                                        sram_lock = 1 # lock SRAM
                                        dram_accessing = port_queues[p][0] # flow ID that has cache missed
                                        placement_idx = 0 # place in front of SRAM as it is immediately being accessed
                                        #print("Cache miss flow ", dram_accessing)
                                    break

                        ## Prefetch flows
                        if not sram_lock: # SRAM is not currently being accessed
                            # determine if current queue occupancy is sufficient to prefetch from

                            # check number of unique flows in port queues
                            unique_ids = []
                            depth = 0
                            while len(unique_ids) < sram_size and not sram_lock:
                                found = False # determines if all pkts in queues have been checked
                                for i in range(num_of_ports): # iterate through ports round-robin
                                    if len(unique_ids) >= sram_size:
                                        break

                                    if depth < len(port_queues[i]): # checks if queue has packets left to be checked
                                        found = True
                                        check_id = port_queues[i][depth]

                                        if check_id not in sram and not sram_lock: # fetch soonest flow not in SRAM
                                            prefetches += 1
                                            sram_lock = 1
                                            dram_accessing = check_id
                                            placement_idx = len(unique_ids)
                                            break

                                        if check_id not in unique_ids:
                                            unique_ids.append(check_id)
                                
                                depth += 1

                                if not found:
                                    break
                            
                            # if unique flows in port queues does not fill SRAM, continue through to future pkts
                            if not sram_lock:
                                temp_idx = curr_idx
                                # if balanced belady is enabled, then only consider packets with creation time before curr
                                while temp_idx < len(flow_id) and (not practical_belady or creation[temp_idx] <= t) and flow_id[temp_idx] in sram and len(unique_ids) < sram_size:
                                    if flow_id[temp_idx] not in unique_ids:
                                        sram.insert(len(unique_ids), sram.pop(sram.index(flow_id[temp_idx]))) # move flow id to order in SRAM
                                        unique_ids.append(flow_id[temp_idx])
                                    temp_idx += 1
                                
                                # if next flow is not in SRAM, prefetch it
                                if len(unique_ids) < cache_size and temp_idx < len(flow_id) and (not practical_belady or creation[temp_idx] <= t) and flow_id[temp_idx] not in sram:
                                    prefetches += 1
                                    sram_lock = 1 # lock SRAM
                                    dram_accessing = flow_id[temp_idx] # flow_id being accessed
                                    placement_idx = len(unique_ids)

                        else: # DRAM is currently being accessed
                            if sram_lock >= dram_access_time: # complete DRAM access
                                sram.pop() # evict last entry (latest to be accessed)
                                sram.insert(placement_idx, dram_accessing)
                                sram_lock = 0 # unlock SRAM
                                #print("Pulled flow ", dram_accessing)
                            else: # continue DRAM access
                                sram_lock += 1
                    
                    total_cache_hits += cache_hits
                    total_cache_misses += cache_misses

                    print("Cache hits: ", cache_hits)
                    print("Cache misses: ", cache_misses)
                    if cache_hits + cache_misses > 0:
                        print("Cache miss %: ", round(cache_misses * 100 / (cache_hits + cache_misses), 2), "%")
        print("Total cache hits: ", total_cache_hits)
        print("Total cache misses: ", total_cache_misses)

if __name__ == '__main__' : main()