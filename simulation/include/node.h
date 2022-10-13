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
    int64_t curr_seq_num[NUM_OF_NODES];
};

typedef struct node* node_t;

extern node_t* nodes;

node_t create_node(int16_t);
void free_node(node_t);

#endif
