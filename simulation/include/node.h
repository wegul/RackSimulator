#ifndef __NODE_H__
#define __NODE_H__

#include "params.h"
#include "arraylist.h"
#include "bounded_buffer.h"
#include "flow.h"

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

    int64_t seq_num[NUM_OF_NODES]; //to be put into the packets

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
