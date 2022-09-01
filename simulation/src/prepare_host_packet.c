#include "prepare_host_packet.h"

static int64_t get_app_to_send_next
    (node_t node, flow_send_t host_flow, int64_t* app_flow_size)
{
    int8_t found = 0;
    flow_stat_logger_sender_t s;
    flow_stat_logger_sender_t s_list
        = host_flow->flow_stat_logger_sender_list;

    for (int16_t j = 0; j < MAX_FLOW_ID; ++j) {
        if (s_list[j].flow_id == host_flow->curr_flow_id) {
            s = &(s_list[j]);
            found = 1;
            break;
        }
    }
    assert(found);

    int64_t size = arraylist_size(s->flow_stat_logger_sender_app_list);
    assert(size > 0);

    int64_t index = 0, curr_app_index = -1;
    flow_stat_logger_sender_app_t curr_app = NULL;

#ifdef ROUND_ROBIN
    int64_t start_index = 0;
    do {
        curr_app = (flow_stat_logger_sender_app_t)
            arraylist_get(s->flow_stat_logger_sender_app_list, start_index);
        arraylist_remove(s->flow_stat_logger_sender_app_list, start_index);
        arraylist_add(s->flow_stat_logger_sender_app_list, curr_app);
        ++index;
    } while (curr_app->app_pkt_transmitted == curr_app->app_flow_size
            && index < size);
    curr_app_index = size-1;
#endif

#ifdef SHORTEST_FLOW_FIRST
    flow_stat_logger_sender_app_t temp_app = NULL;
    int64_t min_bytes_remaining = INT64_MAX;
    while (index < size) {
        temp_app = (flow_stat_logger_sender_app_t)
            arraylist_get(s->flow_stat_logger_sender_app_list, index);
        int64_t bytes_remaining
            = temp_app->app_flow_size - temp_app->app_pkt_transmitted;
        if ((bytes_remaining > 0) && (bytes_remaining < min_bytes_remaining)) {
            min_bytes_remaining = bytes_remaining;
            curr_app = temp_app;
            curr_app_index = index;
        }
        ++index;
    }
    assert(index == size);
#endif

    assert(index <= size && curr_app != NULL && curr_app_index < size);

    if (curr_app->time_first_pkt_sent == -1) {
        curr_app->time_first_pkt_sent = curr_timeslot;
    }

    ++(curr_app->app_pkt_transmitted);

    if (curr_app->app_pkt_transmitted == curr_app->app_flow_size) {
        curr_app->time_last_pkt_sent = curr_timeslot;
    }

    *app_flow_size = curr_app->app_flow_size;
    return curr_app->app_id;
}

packet_t prepare_packet(node_t host_node, int16_t flow_dst_index)
{
    flow_send_t host_flow = host_node->host_flows[flow_dst_index];

    if (host_flow->active == 0) return NULL;

    //create the pkt
    packet_t pkt = create_packet
        (host_node->node_index, flow_dst_index, host_flow->curr_flow_id,
         (host_node->seq_num[flow_dst_index])++);
    int64_t app_flow_size;
    pkt->app_id = get_app_to_send_next(host_node, host_flow, &app_flow_size);

    pkt->time_when_transmitted_from_src = curr_timeslot;

#ifdef DEBUG_SCHEDULING
    if ((pkt->src_mac == 531 && pkt->dst_ip == 624)) {
    //    || (pkt->src_mac == 2 && pkt->dst_mac == 5)) {
    //if (host_index == 0) {
        printf("[%ld/%ld] SCHEDULING HOST PKT %d to %d app id = %ld\n",
                curr_timeslot, curr_epoch,
                pkt->src_node, pkt->dst_node,
                pkt->app_id);
    }
#endif

    //update flow params
    ++(host_flow->pkt_transmitted);

    //host flow has ended
    if (host_flow->pkt_transmitted == host_flow->flow_size) {
        host_flow->active = 0;
        ++(host_flow->curr_flow_id);
        --(host_node->num_of_active_network_host_flows);
        add_flow_finish_to_flow_notf_buffer(host_node, flow_dst_index);
    }

    return pkt;
}

packet_t get_host_packet(node_t host_node)
{
    packet_t pkt = NULL;

    recvd_pull_request_t recvd_pull_req = (recvd_pull_request_t)
        bounded_buffer_get(host_node->recvd_pull_req_queue);

    if (recvd_pull_req != NULL) {
        assert(recvd_pull_req->destination_id != -1);

        pkt = prepare_packet(host_node, recvd_pull_req->destination_id);
        if (pkt != NULL) {
            pkt->data_spine_id = recvd_pull_req->spine_id;
        }

        free(recvd_pull_req);
    }

    if (pkt == NULL) {
        pkt = create_packet(-1, -1, -1, -1);
        pkt->data_spine_id = -1;
    }

    pkt->notf_spine_id = -1;
    for (int i = 0; i < NOTF_SIZE; ++i) {
        pkt->new_flow_src_id[i] = -1;
        pkt->new_flow_dst_id[i] = -1;
        pkt->flow_start[i] = -1;
    }

    for (int i = 0; i < 1; ++i) {
        flow_notification_t x = (flow_notification_t)
            bounded_buffer_get(host_node->flow_notification_buffer);
        if (x != NULL) {
            pkt->new_flow_src_id[i] = host_node->node_index;
            pkt->new_flow_dst_id[i] = x->flow_id;
            pkt->flow_start[i] = x->start;
            pkt->notf_spine_id = 0;
            free(x);
        }
    }

    return pkt;
}

