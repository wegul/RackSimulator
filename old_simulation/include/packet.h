#ifndef __PACKET_H__
#define __PACKET_H__

#include "params.h"

struct packet {
    int16_t src_node;
    int16_t dst_node;
    int64_t flow_id;
    int64_t app_id;

    //Protocol fields
    int16_t data_spine_id; //the spine to/from data pkt is destined/coming

    int16_t notf_spine_id; //the spine to/from cntrl msg is destined/coming
    int16_t new_flow_src_id[NOTF_SIZE]; //also re-purposed for sending the PULL req
    int16_t new_flow_dst_id[NOTF_SIZE]; //also re-purposed for sending the PULL req
    int8_t flow_start[NOTF_SIZE]; //1=flow start; 0=flow finish

    int64_t time_when_added_to_spine_queue;
    int64_t time_when_transmitted_from_src;
    int64_t seq_num;

    int64_t time_to_dequeue_from_link; //simulating propagation delay
};
typedef struct packet* packet_t;

packet_t create_packet(int16_t, int16_t, int64_t, int64_t);
void free_packet(packet_t);
void clear_protocol_fields(packet_t);

#endif
