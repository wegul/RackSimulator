#include "spine.h"

spine_t create_spine(int16_t spine_index)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < NUM_OF_TORS; ++i) {
        self->recv_buffer[i] = create_bounded_buffer(SPINE_PORT_BUFFER_LEN);
        self->send_buffer[i] = create_bounded_buffer(SPINE_PORT_BUFFER_LEN)
        for (int j = 0; j < NUM_OF_NODES; ++j) {
            self->flow_schedule[i][j].status = 0;
        }
    }
    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < NUM_OF_TORS; ++i) {
            free_bounded_buffer(self->recv_buffer[i]);
            free_bounded_buffer(self->send_buffer[i]);
        }

        free(self);
    }
}

packet_t send_to_tor(spine_t spine,
                                    int16_t target_node,
                                    int16_t spine_port)
{
    //Grab the top packet in the correct virtual queue
    packet_t pkt = NULL;

    int16_t* dst_to_send = (int16_t*)
        bounded_buffer_get(spine->dst_to_send_rr[spine_port]);

    if (dst_to_send != NULL) {

        assert(*dst_to_send >= spine_port*NODES_PER_RACK
                && *dst_to_send < spine_port*NODES_PER_RACK + NODES_PER_RACK);

        pkt = (packet_t) bounded_buffer_get(spine->pkt_buffer[*dst_to_send]);

        assert(pkt != NULL);

        if (bounded_buffer_num_of_elements(spine->pkt_buffer[*dst_to_send]) != 0) {
            assert(bounded_buffer_put
                (spine->dst_to_send_rr[spine_port], dst_to_send) != -1);
        } else {
            free(dst_to_send);
        }
    }

    if (pkt == NULL) {
        pkt = create_packet(-1, -1, -1, -1);
        clear_protocol_fields(pkt);
    } else {
        pkt->data_spine_id = spine->spine_index;
    }

    //send the PULL req
    int16_t node = -1;
    if (spine->tor_src_schedulable[spine_port] != 0) {
        node = choose_random_dst(spine, target_node);
    }

    return pkt;
}
