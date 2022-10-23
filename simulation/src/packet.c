#include "packet.h"

packet_t create_packet(int16_t src_node, int16_t dst_node, int64_t flow_id, int64_t size,
        int64_t seq_num, int64_t pkt_id)
{
    packet_t self = (packet_t) malloc(sizeof(struct packet));
    MALLOC_TEST(self, __LINE__);
    self->src_node = src_node;
    self->dst_node = dst_node;
    self->flow_id = flow_id;
    self->size = size;
    self->seq_num = seq_num;
    self->ack_num = 0;
    self->pkt_id = pkt_id;
    self->control_flag = 0;
    self->ecn_flag = 0;
    return self;
}

packet_t ack_packet(packet_t pkt, int64_t ack_num) {
    packet_t ack = (packet_t) malloc(sizeof(struct packet));
    MALLOC_TEST(ack, __LINE__);
    ack->src_node = pkt->dst_node;
    ack->dst_node = pkt->src_node;
    ack->flow_id = pkt->flow_id;
    ack->size = 0;
    ack->seq_num = pkt->seq_num;
    ack->ack_num = ack_num;
    ack->pkt_id = -1;
    ack->control_flag = 1;
    ack->ecn_flag = pkt->ecn_flag;
    return ack;
}

void print_packet(packet_t self) {
    printf("pkt: src %d dst %d flow %d size %d seq %d ack %d ctrl %d\n", (int) self->src_node, (int) self->dst_node, (int) self->flow_id, (int) self->size, (int) self->seq_num, (int) self->ack_num, (int) self->control_flag);
}

void free_packet(packet_t self)
{
    if (self != NULL) free(self);
}
