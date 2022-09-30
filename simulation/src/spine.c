#include "spine.h"

spine_t create_spine(int16_t spine_index)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < NUM_OF_TORS; ++i) {
        self->pkt_buffer[i] = create_bounded_buffer(SPINE_PORT_BUFFER_LEN);
    }

    for (int i = 0; i < SPINE_PORT_BUFFER_LEN+1; ++i) {
        self->queue_stat.queue_len_histogram[i] = 0;
    }

    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < NUM_OF_TORS; ++i) {
            free_bounded_buffer(self->pkt_buffer[i]);
        }

        free(self);
    }
}

packet_t send_to_tor(spine_t spine, int16_t target_node, int16_t tor_num)
{
    //Grab the top packet in the correct virtual queue
    packet_t pkt = NULL;

    pkt = (packet_t) bounded_buffer_get(spine->pkt_buffer[tor_num]);

    if (pkt != NULL) {
        pkt->spine_id = spine->spine_index;
    }

    return pkt;
}
