#ifndef __FLOW_H__
#define __FLOW_H__

#include "params.h"
#include "arraylist.h"

typedef struct flow
{
    int isMemFlow;
    int memType; // -1 for invalid; 0=RREQ, 1=RRESP, 2=WREQ_granted, 999=WREQ_ungranted, 998=WREQ_ungranted_but_notified
    int rreq_bytes;

    int8_t active;
    int8_t finished;
    int64_t flow_id;
    int64_t flow_size_bytes;
    int16_t src;
    int16_t dst;
    int64_t timeslot; // Written in tracefile, deciding when the flow should be activated

    int64_t start_timeslot; // The actual time of the first pkt from this flow being sent.
    int64_t finish_timeslot;
    int64_t timeslots_active; // The number of active timeslots
    int64_t pkts_sent;
    int64_t pkts_received;
    int64_t bytes_sent;
    int64_t bytes_received;
    int64_t expected_runtime;
} flow_t;

flow_t *create_flow(int64_t, int64_t, int16_t, int16_t, int64_t);
void free_flow(flow_t *);

#endif
