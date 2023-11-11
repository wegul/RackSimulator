#ifndef __PACKET_H__
#define __PACKET_H__

#include "params.h"

struct packet
{
    int pktType;
    /*
        -1 = invalid
        1: RREQ (only the firstBLK)              = RREQ
        2: RRESP
        3: WREQ                                 = WREQ
        4+: Notification
        5+: Grant
        100: net                                  = NET
    */
    // int isMemPkt;
    // int memType; // -1 for invalid;
    // /*             0a=RREQ_1st,              1a=RREQ, 1b=RRESP, 1c=WREQ;
    // FOR WREQ ONLY: 0x0c=Notif, 200=Grant*/
    int reqLen; // If it is a nofitication or grant, it'll have a req_len.

    int16_t src_node;
    int16_t dst_node;
    int64_t flow_id;
    int64_t size;

    int64_t time_when_transmitted_from_src;
    int64_t time_to_dequeue_from_link; // simulating propagation delay

    int64_t seq_num;
    int64_t ack_num;

    int64_t pkt_id;
    int16_t control_flag; // To distinguish control packets from data packets
    int16_t ecn_flag;     // DCTCP Explicit Congestion Notification
};
typedef struct packet *packet_t;

packet_t create_packet(int16_t, int16_t, int64_t, int64_t, int64_t, int64_t);
packet_t ack_packet(packet_t, int64_t);
void print_packet(packet_t);
void free_packet(packet_t);

#endif
