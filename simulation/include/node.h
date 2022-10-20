#ifndef __NODE_H__
#define __NODE_H__

#include "params.h"
#include "arraylist.h"
#include "buffer.h"
#include "flow.h"

struct node {
    int16_t node_index;

    buffer_t * active_flows;
    
    int64_t seq_num[NUM_OF_NODES]; //to be put into the packets
    int64_t ack_num[NUM_OF_NODES];
    int64_t last_acked[NUM_OF_NODES];
    int16_t dup_acks[NUM_OF_NODES];
    int16_t ecn_marks[NUM_OF_NODES][ECN_WIDTH];
    int16_t ecn_idx[NUM_OF_NODES];
    int64_t cwnd[NUM_OF_NODES]; // Current congestion window size in MSS
};

typedef struct node* node_t;

extern node_t* nodes;

node_t create_node(int16_t);
void track_ecn(node_t, int16_t, int16_t);
void update_cwnd_3_dup(node_t self, int16_t);
void free_node(node_t);

#endif
