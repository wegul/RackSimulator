#include "node.h"

node_t create_node(int16_t node_index)
{
    node_t self = (node_t) malloc(sizeof(struct node));
    MALLOC_TEST(self, __LINE__);

    self->node_index = node_index;

   /*  self->num_of_active_host_flows = 0;
    self->num_of_active_network_host_flows = 0;
    self->curr_num_of_sending_nodes = 0;
*/
    for (int i = 0; i < NUM_OF_NODES; ++i) {
        self->seq_num[i] = 0;
        self->curr_seq_num[i] = 0;
    }
    
    /*
    self->max_re_order_buffer_size = 0;

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NUM_OF_NODES; ++j) {
            for (int k = 0; k < 3; ++k) {
                self->time_last_pkt_sent[i][j][k] = -1;
            }
        }
    } */

    return self;
}

void free_node(node_t self)
{
    if (self != NULL) {
        /*for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
            free_host_flow(self->host_flows[i]);
            free_dst_flow(self->dst_flows[i]);
            free_arraylist(self->re_order_buffer[i]);
        }

        free_arraylist(self->stat.flow_stat_list); */

        free(self);
    }
}
