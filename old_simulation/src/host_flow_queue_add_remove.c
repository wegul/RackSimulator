#include "host_flow_queue_add_remove.h"

void add_flow_start_to_flow_notf_buffer(node_t node, int16_t flow_dst_index)
{
    flow_notification_t x = (flow_notification_t)
        malloc(sizeof(struct flow_notification));
    MALLOC_TEST(x, __LINE__);
    x->flow_id = flow_dst_index;
    x->start = 1;
    assert(node->flow_notification_buffer != NULL);
    assert(bounded_buffer_put(node->flow_notification_buffer, x) != -1);
}

void add_flow_finish_to_flow_notf_buffer(node_t node, int16_t flow_dst_index)
{
    flow_notification_t x = (flow_notification_t)
        malloc(sizeof(struct flow_notification));
    MALLOC_TEST(x, __LINE__);
    x->flow_id = flow_dst_index;
    x->start = 0;
    assert(node->flow_notification_buffer != NULL);
    assert(bounded_buffer_put(node->flow_notification_buffer, x) != -1);
//    if (node->node_index == 46 && flow_dst_index == 8) {
//        printf("[%ld/%ld] Flow (46->8) finished\n", curr_timeslot, curr_epoch);
//    }
}
