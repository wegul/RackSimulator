#include "node.h"

node_t create_node(int16_t node_index)
{
    node_t self = (node_t) malloc(sizeof(struct node));
    MALLOC_TEST(self, __LINE__);

    self->node_index = node_index;
    
    self->active_flows = create_buffer(MAX_FLOW_ID);

    for (int i = 0; i < NUM_OF_NODES; ++i) {
        self->seq_num[i] = 0;
        self->curr_seq_num[i] = 0;
    }

    return self;
}


void free_node(node_t self)
{
    if (self != NULL) {
        free_buffer(self->active_flows);
        free(self);
    }
}
