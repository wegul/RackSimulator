#include "tor.h"

tor_t create_tor(int16_t tor_index)
{
    tor_t self = (tor_t)malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);
    memset(self, 0, sizeof(struct tor));
    self->tor_index = tor_index;

    for (int i = 0; i < NODES_PER_RACK; ++i)
    {
        self->upstream_pkt_buffer[i] = create_buffer(TOR_UPSTREAM_BUFFER_LEN);      // recved from host
        self->downstream_send_buffer[i] = create_buffer(TOR_DOWNSTREAM_BUFFER_LEN); // pkts that will be sent to host

        self->upstream_mem_buffer[i] = create_buffer(1024);
        self->downstream_mem_buffer[i] = create_buffer(1024);
    }

    create_routing_table(self->routing_table);
    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL)
    {

        for (int i = 0; i < NODES_PER_RACK; ++i)
        {
            free_buffer(self->upstream_pkt_buffer[i]);
            free_buffer(self->downstream_send_buffer[i]);
            free_buffer(self->upstream_mem_buffer[i]);
            free_buffer(self->downstream_mem_buffer[i]);
        }
        free(self);
    }
}
