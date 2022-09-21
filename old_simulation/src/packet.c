#include "packet.h"

packet_t create_packet(int16_t src_node, int16_t dst_node, int64_t flow_id,
        int64_t seq_num)
{
    packet_t self = (packet_t) malloc(sizeof(struct packet));
    MALLOC_TEST(self, __LINE__);
    self->src_node = src_node;
    self->dst_node = dst_node;
    self->flow_id = flow_id;
    self->time_when_transmitted_from_src = -1;
    self->seq_num = seq_num;
    return self;
}

void free_packet(packet_t self)
{
    if (self != NULL) free(self);
}

void clear_protocol_fields(packet_t pkt)
{
    assert(pkt != NULL);
    pkt->data_spine_id = -1;
    pkt->notf_spine_id = -1;
    for (int i = 0; i < NOTF_SIZE; ++i) {
        pkt->new_flow_src_id[i] = -1;
        pkt->new_flow_dst_id[i] = -1;
    }
}
