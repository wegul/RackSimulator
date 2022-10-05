#ifndef __FLOW_H__
#define __FLOW_H__

#include "params.h"
#include "arraylist.h"

typedef struct flow {
    int8_t active;
    int8_t finished;
    int64_t flow_id;
    int64_t flow_size;
    int16_t src;
    int16_t dst;
    int64_t timeslot;
    int64_t pkts_sent;
    int64_t pkts_received;
} flow_t;

flow_t * create_flow(int64_t, int64_t, int16_t, int16_t, int64_t);
void free_flow(flow_t *);

#endif
