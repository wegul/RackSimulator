#include "node.h"

node_t create_node(int16_t node_index)
{
    node_t self = (node_t) malloc(sizeof(struct node));
    MALLOC_TEST(self, __LINE__);

    self->node_index = node_index;
    
    self->active_flows = create_buffer(MAX_FLOW_ID);

    for (int i = 0; i < NUM_OF_NODES; ++i) {
        self->seq_num[i] = 0;
        self->ack_num[i] = 0;
        self->last_acked[i] = 0;
        self->dup_acks[i] = 0;
        for (int j = 0; j < ECN_WIDTH; j++) {
            self->ecn_marks[i][j] = 0;
        }
        self->ecn_idx[i] = 0;
        self->cwnd[i] = 1;
    }
    
    return self;
}

void track_ecn(node_t self, int16_t node, int16_t ecn_mark) {
    self->ecn_marks[node][self->ecn_idx[node]] = ecn_mark;

    self->ecn_idx[node]++;
    if (self->ecn_idx[node] >= ECN_WIDTH) {
        self->ecn_idx[node] = 0;
    }
}

void update_cwnd_3_dup(node_t self, int16_t node) {
    int count = 0;
    for (int i = 0; i < ECN_WIDTH; i++) {
        count += self->ecn_marks[node][i];
    }
    self->cwnd[node] -= self->cwnd[node] * count / ECN_WIDTH;
    if (self->cwnd[node] < 1) {
        self->cwnd[node] = 1;
    }
}


void free_node(node_t self)
{
    if (self != NULL) {
        free_buffer(self->active_flows);
        free(self);
    }
}
