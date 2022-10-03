#ifndef __SPINE_H__
#define __SPINE_H__

#include "params.h"
#include "buffer.h"
#include "packet.h"
#include "links.h"
#include "arraylist.h"

typedef struct {
    int64_t queue_len_histogram[SPINE_PORT_BUFFER_LEN+1];
} spine_queue_stats_t;

struct spine {
    int16_t spine_index;
    //packet storage datastructure
    buffer_t * pkt_buffer[NUM_OF_TORS];
    //stats datastructure
    spine_queue_stats_t queue_stat;
};
typedef struct spine* spine_t;

extern spine_t* spines;

spine_t create_spine(int16_t);
void free_spine(spine_t);
packet_t send_to_tor(spine_t, int16_t);

#endif
