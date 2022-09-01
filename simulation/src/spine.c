#include "spine.h"

spine_t create_spine(int16_t spine_index)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < NUM_OF_NODES; ++i) {
        self->pkt_buffer[i] = create_bounded_buffer(NODES_PER_RACK);
        for (int j = 0; j < NUM_OF_NODES; ++j) {
            self->flow_schedule[i][j].status = 0;
        }
        self->src_schedulable[i] = OVERSUBSCRIPTION;
        self->dst_schedulable[i] = OVERSUBSCRIPTION;
    }

    for (int i = 0; i < NUM_OF_TORS; ++i) {
        self->tor_src_schedulable[i] = epoch_len;
        self->tor_dst_schedulable[i] = epoch_len;
    }

    for (int i = 0; i < SPINE_PORT_COUNT; ++i) {
        self->dst_to_send_rr[i] = create_bounded_buffer(NODES_PER_RACK);
    }

    for (int i = 0; i < SPINE_PORT_BUFFER_LEN+1; ++i) {
        self->queue_stat.queue_len_histogram[i] = 0;
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NUM_OF_NODES; ++j) {
            for (int k = 0; k < 3; ++k) {
                self->time_last_pull_sent[i][j][k] = -1;
            }
        }
    }
    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < NUM_OF_NODES; ++i) {
            free_bounded_buffer(self->pkt_buffer[i]);
        }

        for (int i = 0; i < SPINE_PORT_COUNT; ++i) {
            free(self->dst_to_send_rr[i]);
        }

        free(self);
    }
}

static int16_t choose_random_dst(spine_t spine, int16_t index)
{
    int src_tor_idx = index / NODES_PER_RACK;
    int16_t temp[NUM_OF_NODES] = {-1};
    int count = 0;
    for (int i = 0; i < NUM_OF_NODES; ++i) {
        int dst_tor_idx = i / NODES_PER_RACK;
        if (spine->dst_schedulable[i] != 0
        && spine->tor_dst_schedulable[dst_tor_idx] != 0
        && spine->flow_schedule[index][i].status == 1) {
           temp[count] = i;
           ++count;
        }
    }
    if (count != 0) {
        int random_num = rand();
        int16_t random_index = random_num % count;
        int16_t selected_node = temp[random_index];
        spine->dst_schedulable[selected_node] -= OVERSUBSCRIPTION;
        int dst_tor_idx = selected_node / NODES_PER_RACK;
        if (dst_tor_idx != src_tor_idx) {
            spine->tor_dst_schedulable[dst_tor_idx] -= OVERSUBSCRIPTION;
            spine->tor_src_schedulable[src_tor_idx] -= OVERSUBSCRIPTION;
        }
        return selected_node;
    }

    return -1;
}

packet_t pkt_to_send_from_spine_port(spine_t spine,
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

    if (node != -1) {
        pkt->new_flow_src_id[0] = node; //node sending the PULL req
        pkt->new_flow_dst_id[0] = target_node; //node receiving the PULL req
        pkt->notf_spine_id = spine->spine_index;
        //int s = spine->spine_index;
        //int d = node;
        //spine->time_last_pull_sent[s][d][2]
        //    = spine->time_last_pull_sent[s][d][1];
        //spine->time_last_pull_sent[s][d][1]
        //    = spine->time_last_pull_sent[s][d][0];
        //spine->time_last_pull_sent[s][d][0]
        //    = curr_timeslot;
        //if (spine->time_last_pull_sent[s][d][2] != -1) {
        //    if (spine->time_last_pull_sent[s][d][0]
        //        - spine->time_last_pull_sent[s][d][2] < epoch_len) {
        //        printf("(s=%d d=%d) %d %d %d\n", s, d,
        //                spine->time_last_pull_sent[s][d][0],
        //                spine->time_last_pull_sent[s][d][1],
        //                spine->time_last_pull_sent[s][d][2]);
        //    }
        //    assert(spine->time_last_pull_sent[s][d][0]
        //        - spine->time_last_pull_sent[s][d][2]
        //        >= epoch_len);
        //}
        //if (spine->spine_index==0 && target_node==92) {
        //    printf("[%ld/%ld] SENT PULL REQ (%d,%d,%d)\n",
        //            curr_timeslot, curr_epoch, spine->spine_index,
        //            target_node, node);
        //}
    } else {
        pkt->new_flow_src_id[0] = -1;
        pkt->new_flow_dst_id[0] = -1;
        pkt->notf_spine_id = -1;
    }

#ifdef DEBUG_SPINE
    if (spine->spine_index == 0 && curr_epoch >= 61820 && curr_epoch <= 61880) {
        if (target_node == 9) {
            printf("[%ld/%ld] Sent PULL req (%d->%d) from spine %d\n",
                    curr_timeslot, curr_epoch, target_node, node, spine->spine_index);
        }
        if (node == 73) {
            printf("[%ld/%ld] Sent PULL req (%d->%d) from spine %d\n",
                    curr_timeslot, curr_epoch, target_node, node, spine->spine_index);
        }
    }
#endif
    return pkt;
}

void spine_ingress_processing(spine_t spine, packet_t pkt)
{
    for (int i = 0; i < NOTF_SIZE; ++i) {
        //check for new flow starts
        if (pkt->new_flow_src_id[i] != -1) {
            assert(pkt->notf_spine_id == spine->spine_index);
            int src = pkt->new_flow_src_id[i];
            int dest = pkt->new_flow_dst_id[i];
            assert(src >= 0 && src < NUM_OF_NODES);
            assert(dest >= 0 && dest < NUM_OF_NODES);
            if (pkt->flow_start[i] == 1) {
                spine->flow_schedule[src][dest].status = 1;
            } else if (pkt->flow_start[i] == 0) {
                spine->flow_schedule[src][dest].status = 0;
            }
        }
    }
}
