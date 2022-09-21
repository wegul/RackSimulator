#ifndef __TOR_H__
#define __TOR_H__

#include "params.h"
#include "bounded_buffer.h"
#include "packet.h"
#include "routing_table.h"

struct tor {
    int16_t tor_index;
    bounded_buffer_t downstream_pkt_buffer[NODES_PER_RACK];
    bounded_buffer_t upstream_pkt_buffer[NUM_OF_SPINES];

    int16_t start_pointer;

    rnode_t routing_table[RTABLE_SIZE];
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t);
void free_tor(tor_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);

#endif
