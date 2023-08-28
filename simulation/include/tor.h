#ifndef __TOR_H__
#define __TOR_H__

#include "params.h"
#include "buffer.h"
#include "packet.h"
#include "routing_table.h"
#include "timeseries.h"
#include "snapshot.h"
#include "memory.h"

struct tor {
    int16_t tor_index;
    rnode_t routing_table[RTABLE_SIZE];
    //packet buffer data structures
    buffer_t * downstream_pkt_buffer[NUM_OF_SPINES];
    buffer_t * upstream_pkt_buffer[NODES_PER_RACK];
    //packet sending data structures
    buffer_t * downstream_send_buffer[NODES_PER_RACK];
    buffer_t * upstream_send_buffer[NUM_OF_SPINES];
    //stats datastructure
    timeseries_t * downstream_queue_stat[NUM_OF_SPINES];
    timeseries_t * upstream_queue_stat[NODES_PER_RACK];
    int64_t cache_hits;
    int64_t cache_misses;
    //memory datastructure
    sram_t * sram;
    lfu_sram_t * lfu_sram;
    arc_sram_t * arc_sram;
    dm_sram_t * dm_sram;
    dram_t * dram;
    //separate snapshot lists for each upstream and downstream port
    buffer_t * upstream_snapshot_list[NODES_PER_RACK];
    buffer_t * downstream_snapshot_list[NUM_OF_SPINES];
    //checks if a memory access was done on this timeslot
    int access_on_this_timeslot;
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t, int32_t, int16_t);
void free_tor(tor_t);
packet_t process_packets_up(tor_t tor, int16_t port, int64_t * cache_misses, int64_t * cache_hits, int sram_type);
packet_t process_packets_down(tor_t tor, int16_t port, int64_t * cache_misses, int64_t * cache_hits, int sram_type);
packet_t move_to_up_send_buffer(tor_t tor, int16_t port);
packet_t move_to_down_send_buffer(tor_t tor, int16_t port);
packet_t send_to_spine_baseline(tor_t, int16_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_spine_dm(tor_t, int16_t, int64_t *, int64_t *);
packet_t send_to_spine_dram_only(tor_t tor, int16_t spine_id, int64_t * cache_misses);
packet_t send_to_host_baseline(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);
packet_t send_to_host_dram_only(tor_t tor, int16_t host_within_tor, int64_t * cache_misses);
snapshot_t * snapshot_to_spine(tor_t, int16_t);
snapshot_t ** snapshot_array_tor(tor_t);
int64_t tor_up_buffer_bytes(tor_t, int);
int64_t tor_down_buffer_bytes(tor_t, int);
int64_t * linearize_tor_downstream_queues(tor_t, int *);

#endif
