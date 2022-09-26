#ifndef __TOR_H__
#define __TOR_H__

#include "params.h"
#include "bounded_buffer.h"
#include "packet.h"
#include "routing_table.h"

typedef struct {
    int64_t upstream_queue_len_histogram[TOR_UPSTREAM_BUFFER_LEN+1];
    int64_t downstream_queue_len_histogram[TOR_DOWNSTREAM_BUFFER_LEN+1];
} tor_queue_stats_t;

struct tor {
    int16_t tor_index;
    bounded_buffer_t downstream_pkt_buffer[NODES_PER_RACK];
    bounded_buffer_t upstream_pkt_buffer[NUM_OF_SPINES];

    int16_t start_pointer;

    rnode_t routing_table[RTABLE_SIZE];

    tor_queue_stats_t queue_stat;
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t);
void free_tor(tor_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);

#endif
