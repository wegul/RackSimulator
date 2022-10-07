#include "tor.h"

tor_t create_tor(int16_t tor_index)
{
    tor_t self = (tor_t) malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);

    self->tor_index = tor_index;

    for (int i = 0; i < NODES_PER_RACK; ++i) {
        self->downstream_pkt_buffer[i]
            = create_buffer(TOR_DOWNSTREAM_BUFFER_LEN);
        self->downstream_queue_stat[i] = create_timeseries();
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        self->upstream_pkt_buffer[i]
            = create_buffer(TOR_UPSTREAM_BUFFER_LEN);
        self->upstream_queue_stat[i] = create_timeseries();
    }
    
    create_routing_table(self->routing_table);
    
    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL) {

        for (int i = 0; i < NODES_PER_RACK; ++i) {
            free_buffer(self->downstream_pkt_buffer[i]);
            free_timeseries(self->downstream_queue_stat[i]);
        }

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            free_buffer(self->upstream_pkt_buffer[i]);
            free_timeseries(self->upstream_queue_stat[i]);
        }

        free(self);
    }
}

packet_t send_to_spine(tor_t tor, int16_t spine_id)
{
    packet_t pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);

    return pkt;
}

packet_t send_to_host(tor_t tor, int16_t host_within_tor)
{
    packet_t pkt = (packet_t)
        buffer_get(tor->downstream_pkt_buffer[host_within_tor]);

    return pkt;
}
