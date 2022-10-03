#include "flow.h"

flow_t * create_flow(int64_t flow_id, int64_t flow_size, int16_t host_index, int16_t dst_index) {
    flow_t * self = (flow_t *) malloc(sizeof(flow_t));
    MALLOC_TEST(self, __LINE__);
    self->active = 0;
    self->flow_id = flow_id;
    self->flow_size = flow_size;
    self->src = host_index;
    self->dst = dst_index;
    self->pkts_sent = 0;
    self->pkts_received = 0;

    return self;
}

void free_flow(flow_t * self) {
    if (self != NULL) {
        free(self);
    }
}
