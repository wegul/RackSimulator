#ifndef __TOR_H__
#define __TOR_H__

#include "params.h"
#include "bounded_buffer.h"
#include "packet.h"

typedef struct {
    int64_t upstream_queue_len_histogram[TOR_UPSTREAM_BUFFER_LEN+1];
    int64_t downstream_queue_len_histogram[TOR_DOWNSTREAM_BUFFER_LEN+1];
} tor_queue_stats_t;

struct pull_req {
    int16_t source;
    int16_t destination;
    int16_t spine_id;
};
typedef struct pull_req* pull_req_t;

struct tor_flow_notification {
    int16_t new_flow_src_id;
    int16_t new_flow_dst_id;
    int8_t flow_start;
};
typedef struct tor_flow_notification* tor_flow_notification_t;

struct tor {
    int16_t tor_index;
    bounded_buffer_t downstream_pkt_buffer[NODES_PER_RACK];
    bounded_buffer_t pull_req_queue[NODES_PER_RACK];
    bounded_buffer_t upstream_pkt_buffer[NUM_OF_SPINES];

    bounded_buffer_t flow_notification_queue[NODES_PER_RACK];
    int16_t start_pointer;
    int16_t temp_new_flow_src_id[NOTF_SIZE];
    int16_t temp_new_flow_dst_id[NOTF_SIZE];
    int8_t temp_flow_start[NOTF_SIZE];

    tor_queue_stats_t queue_stat;
};
typedef struct tor* tor_t;

extern tor_t* tors;

tor_t create_tor(int16_t);
void free_tor(tor_t);
void extract_pull_req(tor_t, packet_t);
void extract_flow_notification(tor_t, packet_t);
packet_t send_to_spine(tor_t, int16_t);
packet_t send_to_host(tor_t, int16_t);

#endif
