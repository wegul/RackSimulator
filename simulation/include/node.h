#ifndef __NODE_H__
#define __NODE_H__

#include "params.h"
#include "arraylist.h"
#include "buffer.h"
#include "flow.h"

struct node {
    int16_t node_index;

    buffer_t * active_flows;
    flow_t * current_flow;
    
    int64_t seq_num[MAX_FLOW_ID]; //to be put into the packets
    int64_t ack_num[MAX_FLOW_ID];
    int64_t last_acked[MAX_FLOW_ID];
    int16_t ecn_marks[MAX_FLOW_ID][ECN_WIDTH];
    int16_t ecn_idx[MAX_FLOW_ID];
    int64_t cwnd[MAX_FLOW_ID]; // Current congestion window size in MTU
    int64_t acks_since_last_cwnd_increase[MAX_FLOW_ID]; // When CWND acks are received, cwnd can be incrememented
    int64_t ssthresh[MAX_FLOW_ID]; // Slow Start Threshold
};

typedef struct node* node_t;

extern node_t* nodes;

node_t create_node(int16_t);
void track_ecn(node_t, int16_t, int16_t);
void free_node(node_t);

#endif
