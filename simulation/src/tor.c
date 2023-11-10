#include "tor.h"

tor_t create_tor(int16_t tor_index)
{
    tor_t self = (tor_t)malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);
    memset(self, 0, sizeof(struct tor));
    self->tor_index = tor_index;
    self->ntf_cnt = 0;
    for (int i = 0; i < NODES_PER_RACK; ++i)
    {
        self->upstream_pkt_buffer[i] = create_buffer(TOR_UPSTREAM_BUFFER_LEN);      // recved from host
        self->downstream_send_buffer[i] = create_buffer(TOR_DOWNSTREAM_BUFFER_LEN); // pkts that will be sent to host

        self->upstream_mem_buffer[i] = create_buffer(TOR_DOWNSTREAM_MEMBUF_LEN);
        self->downstream_mem_buffer[i] = create_buffer(TOR_UPSTREAM_MEMBUF_LEN);
        self->downstream_mem_buffer_lock[i] = -1;
    }
    for (int i = 0; i < MAX_FLOW_ID; i++)
    {
        self->notif_queue[i] = malloc(sizeof(notif_t));
    }
    for (int i = 0; i < MAX_FLOW_ID; i++)
    {
        self->notif_queue[i]->req_type = -1;
        self->notif_queue[i]->length = -1;
        self->notif_queue[i]->isGranted = 0;
        self->notif_queue[i]->sender = -1;
        self->notif_queue[i]->receiver = -1;
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
