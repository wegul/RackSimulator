#include "packet.h"

packet_t create_packet(int16_t src_node, int16_t dst_node, int64_t flow_id,
        int64_t seq_num)
{
    packet_t self = (packet_t) malloc(sizeof(struct packet));
    MALLOC_TEST(self, __LINE__);
    self->src_node = src_node;
    self->dst_node = dst_node;
    self->flow_id = flow_id;
    self->seq_num = seq_num;
    return self;
}

void print_packet(packet_t self) {
    printf("pkt: %d %d %d %d\n", (int) self->src_node, (int) self->dst_node, (int) self->flow_id, (int) self->seq_num);
}

void free_packet(packet_t self)
{
    if (self != NULL) free(self);
}
