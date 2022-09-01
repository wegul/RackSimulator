#ifndef __HOST_FLOW_QUEUE_ADD_REMOVE_H__
#define __HOST_FLOW_QUEUE_ADD_REMOVE_H__

#include "params.h"
#include "node.h"
#include "bounded_buffer.h"

void add_flow_start_to_flow_notf_buffer(node_t, int16_t);
void add_flow_finish_to_flow_notf_buffer(node_t, int16_t);

#endif
