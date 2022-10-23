#include "spine.h"

spine_t create_spine(int16_t spine_index)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < SPINE_PORT_COUNT; i++) {
        self->pkt_buffer[i] = create_buffer(SPINE_PORT_BUFFER_LEN);
        self->queue_stat[i] = create_timeseries();
    }

    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < SPINE_PORT_COUNT; ++i) {
            if (self->pkt_buffer[i] != NULL) {
                free_buffer(self->pkt_buffer[i]);
            }
            free_timeseries(self->queue_stat[i]);
        }

        free(self);
    }
}

packet_t send_to_tor(spine_t spine, int16_t tor_num)
{
    //Grab the top packet in the correct virtual queue
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);

    return pkt;
}

int64_t spine_buffer_bytes(spine_t spine, int port)
{
    int64_t bytes = 0;
    for (int i = 0; i < spine->pkt_buffer[port]->num_elements; i++) {
        packet_t pkt = buffer_peek(spine->pkt_buffer[port], i);
        bytes += pkt->size;
    }
    return bytes;
}