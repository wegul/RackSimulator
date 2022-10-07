#ifndef __TOR_H__
#define __TOR_H__

#include "params.h"
#include "buffer.h"
#include "packet.h"
#include "routing_table.h"
#include "timeseries.h"

struct tor {
    int16_t tor_index;
    rnode_t routing_table[RTABLE_SIZE];
    buffer_t * downstream_pkt_buffer[NODES_PER_RACK];
    buffer_t * upstream_pkt_buffer[NUM_OF_SPINES];

    timeseries_t * downstream_queue_stat[NODES_PER_RACK];
    timeseries_t * upstream_queue_stat[NUM_OF_SPINES];
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t);
void free_tor(tor_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);

#endif
