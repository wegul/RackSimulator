#ifndef __SPINE_H__
#define __SPINE_H__

#include "params.h"
#include "bounded_buffer.h"
#include "packet.h"
#include "links.h"
#include "arraylist.h"

typedef struct {
    int64_t queue_len_histogram[SPINE_PORT_BUFFER_LEN+1];
} spine_queue_stats_t;

typedef struct {
    int64_t status;
} flow_schedule_t;

struct spine {
    int16_t spine_index;
    //packet storage datastructure
    bounded_buffer_t pkt_buffer[NUM_OF_NODES];
    bounded_buffer_t dst_to_send_rr[SPINE_PORT_COUNT];
    //scheduling datastructure
    flow_schedule_t flow_schedule[NUM_OF_NODES][NUM_OF_NODES];
    int8_t src_schedulable[NUM_OF_NODES];
    int8_t dst_schedulable[NUM_OF_NODES];
    int8_t tor_src_schedulable[NUM_OF_TORS];
    int8_t tor_dst_schedulable[NUM_OF_TORS];
    //stats datastructure
    spine_queue_stats_t queue_stat;
    int64_t time_last_pull_sent[NUM_OF_SPINES][NUM_OF_NODES][3];
};
typedef struct spine* spine_t;

extern spine_t* spines;

spine_t create_spine(int16_t);
void free_spine(spine_t);
packet_t pkt_to_send_from_spine_port(spine_t, int16_t, int16_t);
void spine_ingress_processing(spine_t, packet_t);

#endif
