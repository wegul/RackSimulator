#ifndef __SPINE_H__
#define __SPINE_H__

#include "params.h"
#include "buffer.h"
#include "packet.h"
#include "links.h"
#include "arraylist.h"
#include "timeseries.h"
#include "snapshot.h"
#include "memory.h"

struct spine {
    int16_t spine_index;
    //packet storage datastructure
    buffer_t * pkt_buffer[NUM_OF_TORS];
    //track where in pkt_buffer we have sent snapshots up to
    int16_t snapshot_idx[NUM_OF_TORS];
    //stats datastructure
    timeseries_t * queue_stat[SPINE_PORT_COUNT];
    //memory datastructure
    sram_t * sram;
    dm_sram_t * dm_sram;
    dram_t * dram;
};
typedef struct spine* spine_t;

extern spine_t* spines;

spine_t create_spine(int16_t, int32_t, int16_t);
void free_spine(spine_t);
packet_t send_to_tor(spine_t, int16_t, int64_t *, int64_t *, int);
packet_t send_to_tor_dm(spine_t, int16_t, int64_t *, int64_t *);
packet_t send_to_tor_dram_only(spine_t spine, int16_t tor_num, int64_t * cache_misses);
snapshot_t * snapshot_to_tor(spine_t, int16_t);
int64_t spine_buffer_bytes(spine_t, int);

#endif
