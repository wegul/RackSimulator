#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include <stdint.h>
#include "buffer.h"
#include "packet.h"
#include "params.h"

// Snapshots must fit within INTERPACKET_GAP worth of bits space
// 96 bit interpacket gap / 16 bit flow_id + 8 bit queue_delay = 4 pkts
typedef struct snapshot {
    int flow_id[SNAPSHOT_SIZE];
    //uint8_t queue_delay_kb[SNAPSHOT_SIZE]; // KB of data ahead of pkt in queue plus pkt size
    int64_t time_to_dequeue_from_link; //simulating propagation delay
} snapshot_t;

snapshot_t * create_snapshot(buffer_t * pkt_buffer, int16_t * pkts_recorded);

#endif