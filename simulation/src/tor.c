#include "tor.h"

tor_t create_tor(int16_t tor_index)
{
    tor_t self = (tor_t) malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);

    self->tor_index = tor_index;

    for (int i = 0; i < NODES_PER_RACK; ++i) {
        self->downstream_pkt_buffer[i]
            = create_bounded_buffer(TOR_DOWNSTREAM_BUFFER_LEN);
        self->pull_req_queue[i] = create_bounded_buffer(TOR_PULL_QUEUE_LEN);
        self->flow_notification_queue[i]
            = create_bounded_buffer(TOR_NOTIFICATION_QUEUE_LEN);
    }

    self->start_pointer = 0;

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        self->upstream_pkt_buffer[i]
            = create_bounded_buffer(TOR_UPSTREAM_BUFFER_LEN);
    }

    for (int i = 0; i < TOR_UPSTREAM_BUFFER_LEN+1; ++i) {
        self->queue_stat.upstream_queue_len_histogram[i] = 0;
    }

    for (int i = 0; i < TOR_DOWNSTREAM_BUFFER_LEN+1; ++i) {
        self->queue_stat.downstream_queue_len_histogram[i] = 0;
    }

    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL) {

        for (int i = 0; i < NODES_PER_RACK; ++i) {
            free_bounded_buffer(self->downstream_pkt_buffer[i]);
            free_bounded_buffer(self->pull_req_queue[i]);
        }

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            free_bounded_buffer(self->upstream_pkt_buffer[i]);
            free_bounded_buffer(self->flow_notification_queue[i]);
        }

        free(self);
    }
}

void extract_pull_req(tor_t tor, packet_t pkt)
{
    if (pkt->notf_spine_id != -1) {
        pull_req_t req = (pull_req_t) malloc(sizeof(struct pull_req));
        req->source = pkt->new_flow_src_id[0];
        req->destination = pkt->new_flow_dst_id[0];
        req->spine_id = pkt->notf_spine_id;

        assert(req->destination >= tor->tor_index*NODES_PER_RACK
          && req->destination < tor->tor_index*NODES_PER_RACK + NODES_PER_RACK);
        int16_t dst_within_tor = req->destination % NODES_PER_RACK;
        assert(bounded_buffer_num_of_elements
            (tor->pull_req_queue[dst_within_tor]) < TOR_PULL_QUEUE_LEN);
        assert(bounded_buffer_put(tor->pull_req_queue[dst_within_tor], req) != -1);
    }

    if (pkt->data_spine_id != -1) { //not dummy
        assert(pkt->dst_node >= tor->tor_index*NODES_PER_RACK
            && pkt->dst_node < tor->tor_index*NODES_PER_RACK + NODES_PER_RACK);
        int16_t dst_within_tor = pkt->dst_node % NODES_PER_RACK;
        //if (tor->tor_index == 2 && dst_within_tor == 3) {
        //    printf("[%ld/%ld] Adding <%d,%d> from spine %d, count = %d\n",
        //            curr_timeslot, curr_epoch, pkt->src_node, pkt->dst_node,
        //            pkt->data_spine_id, bounded_buffer_num_of_elements(tor->downstream_pkt_buffer[dst_within_tor])+1);
        //}
        //if (bounded_buffer_num_of_elements(tor->downstream_pkt_buffer[dst_within_tor]) >= TOR_DOWNSTREAM_BUFFER_LEN) {
        //    printf("ToR = %d buffer = %d\n", tor->tor_index, dst_within_tor);
        //}
        assert(bounded_buffer_num_of_elements
            (tor->downstream_pkt_buffer[dst_within_tor])
                < TOR_DOWNSTREAM_BUFFER_LEN);
        assert(bounded_buffer_put
            (tor->downstream_pkt_buffer[dst_within_tor], pkt) != -1);
        clear_protocol_fields(pkt);
    } else {
        assert(pkt->src_node == -1 && pkt->dst_node == -1);
        free_packet(pkt);
    }
}

void extract_flow_notification(tor_t tor, packet_t pkt)
{
    if (pkt->notf_spine_id != -1) {
        for (int i = 0; i < NOTF_SIZE; ++i) {
            if (pkt->new_flow_src_id[i] != -1) {
                tor_flow_notification_t notf = (tor_flow_notification_t)
                    malloc(sizeof(struct tor_flow_notification));
                notf->new_flow_src_id = pkt->new_flow_src_id[i];
                notf->new_flow_dst_id = pkt->new_flow_dst_id[i];
                notf->flow_start = pkt->flow_start[i];
                int16_t node_within_tor = pkt->new_flow_src_id[i] % NODES_PER_RACK;
                assert(bounded_buffer_put
                    (tor->flow_notification_queue[node_within_tor],
                        notf) != -1);
            }
        }
    }

    if (pkt->data_spine_id != -1) { //not dummy
        assert(pkt->src_node != -1 && pkt->dst_node != -1);
        if (pkt->dst_node >= tor->tor_index*NODES_PER_RACK
            && pkt->dst_node < tor->tor_index*NODES_PER_RACK + NODES_PER_RACK) {
            int16_t dst_within_tor = pkt->dst_node % NODES_PER_RACK;
            //if (tor->tor_index == 2 && dst_within_tor == 3) {
            //    printf("[%ld/%ld] Adding <%d,%d> from spine %d, count = %d\n",
            //            curr_timeslot, curr_epoch, pkt->src_node, pkt->dst_node,
            //            pkt->data_spine_id, bounded_buffer_num_of_elements(tor->downstream_pkt_buffer[dst_within_tor])+1);
            //}
            //if (bounded_buffer_num_of_elements(tor->downstream_pkt_buffer[dst_within_tor]) >= TOR_DOWNSTREAM_BUFFER_LEN) {
            //    printf("ToR = %d buffer = %d\n", tor->tor_index, dst_within_tor);
            //}
            assert(bounded_buffer_num_of_elements
                (tor->downstream_pkt_buffer[dst_within_tor])
                    < TOR_DOWNSTREAM_BUFFER_LEN);
            assert(bounded_buffer_put
                (tor->downstream_pkt_buffer[dst_within_tor], pkt) != -1);
        } else {
            assert(bounded_buffer_num_of_elements
                (tor->upstream_pkt_buffer[pkt->data_spine_id])
                    < TOR_UPSTREAM_BUFFER_LEN);
            assert(bounded_buffer_put
                (tor->upstream_pkt_buffer[pkt->data_spine_id], pkt) != -1);
        }
        clear_protocol_fields(pkt);
    } else {
        assert(pkt->src_node == -1 && pkt->dst_node == -1);
        free_packet(pkt);
    }
}

packet_t send_to_spine(tor_t tor, int16_t spine_id)
{
    if (spine_id == 0) {
        for(int i = 0; i < NOTF_SIZE; ++i) {
            tor->temp_new_flow_src_id[i] = -1;
            tor->temp_new_flow_dst_id[i] = -1;
            tor->temp_flow_start[i] = -1;
        }
        int count = 0;
        int empty_queue_num = 0;
        int i = tor->start_pointer;
        while (1) {
            tor_flow_notification_t notf = (tor_flow_notification_t)
                bounded_buffer_get(tor->flow_notification_queue[i]);
            i = (i + 1) % NODES_PER_RACK;
            tor->start_pointer = i;
            if (notf != NULL) {
                tor->temp_new_flow_src_id[count] = notf->new_flow_src_id;
                tor->temp_new_flow_dst_id[count] = notf->new_flow_dst_id;
                tor->temp_flow_start[count] = notf->flow_start;
                free(notf);
                ++count;
                if (count == NOTF_SIZE) break;
            } else {
                ++empty_queue_num;
                if (empty_queue_num == NODES_PER_RACK) break;
            }
        }
    }

    packet_t pkt = (packet_t)
        bounded_buffer_get(tor->upstream_pkt_buffer[spine_id]);
    if (pkt == NULL) {
        pkt = create_packet(-1, -1, -1, -1);
        clear_protocol_fields(pkt);
    } else {
        pkt->data_spine_id = spine_id;
    }

    pkt->notf_spine_id = spine_id;
    for (int i = 0; i < NOTF_SIZE; ++i) {
        pkt->new_flow_src_id[i] = tor->temp_new_flow_src_id[i];
        pkt->new_flow_dst_id[i] = tor->temp_new_flow_dst_id[i];
        pkt->flow_start[i] = tor->temp_flow_start[i];
    }

    return pkt;
}

packet_t send_to_host(tor_t tor, int16_t host_within_tor)
{
    packet_t pkt = (packet_t)
        bounded_buffer_get(tor->downstream_pkt_buffer[host_within_tor]);
    if (pkt == NULL) {
        pkt = create_packet(-1, -1, -1, -1);
        clear_protocol_fields(pkt);
    }

    pull_req_t req = (pull_req_t)
        bounded_buffer_get(tor->pull_req_queue[host_within_tor]);
    if (req != NULL) {
        pkt->new_flow_src_id[0] = req->source;
        pkt->new_flow_dst_id[0] = req->destination;
        pkt->notf_spine_id = req->spine_id;
        free(req);
    }

    return pkt;
}
