#ifndef __NODE_H__
#define __NODE_H__

#include "params.h"
#include "arraylist.h"
#include "bounded_buffer.h"
#include "flow.h"

struct flow_stats {
    int64_t flow_id;
    int16_t src;
    int16_t dst;
    int64_t flow_size_bytes;
    int64_t flow_size;
    int64_t sender_completion_time_1;//last sent - flow created
    int64_t sender_completion_time_2;//last sent - first sent
    int64_t receiver_completion_time;//last recvd - first recvd
    int64_t sender_receiver_completion_time;//last recvd - first sent
    int64_t actual_completion_time;//last recvd - flow created
};

typedef struct flow_stats* flow_stats_t;

typedef struct stats {
    int64_t host_pkt_received;
    int64_t dummy_pkt_received;
    int64_t total_time_active;
    arraylist_t flow_stat_list;
    int16_t curr_incast_degree;
    int16_t max_incast_degree;
} stats_t;

struct recvd_pull_request {
    int16_t spine_id;
    int16_t destination_id;
};

typedef struct recvd_pull_request* recvd_pull_request_t;

struct flow_notification {
    int16_t flow_id;
    int8_t start;
};

typedef struct flow_notification* flow_notification_t;

struct node {
    int16_t node_index;

    //stores the latest PULL req
    bounded_buffer_t recvd_pull_req_queue;

    //list of all flow start/finish
    bounded_buffer_t flow_notification_buffer;

    //flow stat datastructures
    flow_send_t host_flows[NUM_OF_NODES]; //flows starting at this node
                                          //indexed by flow-dst-index
    flow_recv_t dst_flows[NUM_OF_NODES]; //flows destined to this node
                                         //indexed by flow-src-index

    //for keeping track of experiment progress
    int64_t num_of_active_host_flows;
    int64_t num_of_active_network_host_flows;
    int64_t curr_num_of_sending_nodes;

    stats_t stat;

    int64_t seq_num[NUM_OF_NODES]; //to be put into the packets

    int64_t time_last_pkt_sent[NUM_OF_SPINES][NUM_OF_NODES][3];

    //re-ordering buffer
    arraylist_t re_order_buffer[NUM_OF_NODES];
    int64_t curr_seq_num[NUM_OF_NODES];
    int64_t max_re_order_buffer_size;
};

typedef struct node* node_t;

extern node_t* nodes;

node_t create_node(int16_t);
void free_node(node_t);

#endif
