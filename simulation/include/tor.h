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
    buffer_t * downstream_pkt_buffer[NODES_PER_RACK];
    buffer_t * upstream_pkt_buffer[NUM_OF_SPINES];
    //track where in upstream_pkt_buffer we have sent snapshots up to
    int16_t snapshot_idx[NUM_OF_SPINES];
    //stats datastructure
    timeseries_t * downstream_queue_stat[NODES_PER_RACK];
    timeseries_t * upstream_queue_stat[NUM_OF_SPINES];
    //memory datastructure
    sram_t * sram;
    dm_sram_t * dm_sram;
    dram_t * dram;
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t);
void free_tor(tor_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);
snapshot_t * snapshot_to_spine(tor_t, int16_t);
int64_t tor_up_buffer_bytes(tor_t, int);
int64_t tor_down_buffer_bytes(tor_t, int);

#endif
