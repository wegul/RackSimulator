#include "system_stats.h"

void update_pkt_counters(node_t dst_node, packet_t pkt)
{
    if (dst_node->curr_num_of_sending_nodes > 0) {
        ++(dst_node->stat.total_time_active);

        if (pkt->dst_node == -1) {
            ++(dst_node->stat.dummy_pkt_received);
        } else if (pkt->dst_node == dst_node->node_index) {
            ++(dst_node->stat.host_pkt_received);
        }
    }
}

void update_stats_on_pkt_recv(node_t dst_node,
                            packet_t pkt,
                            int8_t static_workload,
                            int8_t start_logging)
{
    //re-ordering buffer
    if (pkt->seq_num != dst_node->curr_seq_num[pkt->src_node]) {
        //add to re-order buffer
        int64_t* x = (int64_t*) malloc(sizeof(int64_t));
        *x = pkt->seq_num;
        arraylist_add(dst_node->re_order_buffer[pkt->src_node], x);
        int64_t size = arraylist_size
            (dst_node->re_order_buffer[pkt->src_node]);
        if (size > dst_node->max_re_order_buffer_size) {
            dst_node->max_re_order_buffer_size = size;
        }

    } else {
        //start freeing packets from the re-order buffer
        ++(dst_node->curr_seq_num[pkt->src_node]);
        int64_t prev_seq_num = -1;
        while (dst_node->curr_seq_num[pkt->src_node]!=prev_seq_num) {
            prev_seq_num = dst_node->curr_seq_num[pkt->src_node];
            int64_t size = arraylist_size
                (dst_node->re_order_buffer[pkt->src_node]);
            for (int j = 0; j < size; ++j) {
                int64_t* x = (int64_t*) arraylist_get
                    (dst_node->re_order_buffer[pkt->src_node], j);
                if (*x == dst_node->curr_seq_num[pkt->src_node]) {
                    ++(dst_node->curr_seq_num[pkt->src_node]);
                    arraylist_remove
                        (dst_node->re_order_buffer[pkt->src_node],j);
                    free(x);
                    break;
                }
            }
        }

    }

    flow_recv_t dst_flow = dst_node->dst_flows[pkt->src_node];

    //find the recv stat logger corres to curr app level flow
    int8_t found = 0;
    flow_stat_logger_receiver_t r = NULL;
    int64_t r_index = 0;
    arraylist_t r_list
        = dst_flow->flow_stat_logger_receiver_list;
    int64_t size = arraylist_size(r_list);

    for (int16_t j = 0; j < size; ++j) {
        flow_stat_logger_receiver_t temp
            = (flow_stat_logger_receiver_t)
                arraylist_get(r_list, j);
        if (temp->flow_id == pkt->flow_id
            && temp->app_id == pkt->app_id) {
            found = 1;
            r = temp;
            r_index = j;
            break;
        }
    }
    //assert(found == 1);

    if (static_workload == 1) {
        if (start_logging) {
             ++(r->pkt_recvd_since_logging_started);
        }
    }

    if (r->pkt_recvd == 0) { //first pkt recvd
        r->time_first_pkt_recvd = curr_timeslot;
    }

    ++(r->pkt_recvd);

    if (r->pkt_recvd == r->app_flow_size) { //last pkt recvd

        --(dst_node->curr_num_of_sending_nodes);

        r->time_last_pkt_recvd = curr_timeslot;

        flow_stats_t fstat = (flow_stats_t)
            malloc(sizeof(struct flow_stats));
        MALLOC_TEST(fstat, __LINE__);

        node_t src_node = nodes[pkt->src_node];
        --(src_node->num_of_active_host_flows);

        //log all the completion times
        flow_send_t src_flow
            = src_node->host_flows[dst_node->node_index];

        found = 0;
        flow_stat_logger_sender_t s;
        flow_stat_logger_sender_t s_list
            = src_flow->flow_stat_logger_sender_list;

        for (int16_t j = 0; j < MAX_FLOW_ID; ++j) {
            if (s_list[j].flow_id == pkt->flow_id) {
                found = 1;
                s = &(s_list[j]);
                break;
            }
        }
        assert(found == 1);

        arraylist_t app_list
            = s->flow_stat_logger_sender_app_list;
        int64_t app_list_size = arraylist_size(app_list);

        found = 0;
        flow_stat_logger_sender_app_t app;
        int64_t app_index;

        for (int64_t j = 0; j < app_list_size; ++j) {
            flow_stat_logger_sender_app_t temp
                = (flow_stat_logger_sender_app_t)
                    arraylist_get(app_list, j);
            if (temp->app_id == pkt->app_id) {
                found = 1;
                app = temp;
                app_index = j;
                break;
            }
        }
        assert(found == 1);

        num_of_flows_finished += 1;

        *fstat = (struct flow_stats) {
            .flow_id = r->app_id,
            .src = dst_flow->src,
            .dst = dst_flow->dst,
            .flow_size_bytes = r->app_flow_size_bytes,
            .flow_size = r->app_flow_size,
            .sender_completion_time_1
                = (app->time_last_pkt_sent
                    - app->time_app_flow_created + 1),
            .sender_completion_time_2
                = (app->time_last_pkt_sent
                    - app->time_first_pkt_sent + 1),
            .receiver_completion_time
                = (r->time_last_pkt_recvd
                    - r->time_first_pkt_recvd + 1),
            .sender_receiver_completion_time
                = (r->time_last_pkt_recvd
                    - app->time_first_pkt_sent + 1),
            .actual_completion_time
                = (r->time_last_pkt_recvd
                    - app->time_app_flow_created + 1)
        };

        arraylist_add(src_node->stat.flow_stat_list, fstat);

        //invalidating the entries
        flow_stat_logger_receiver_t x
            = (flow_stat_logger_receiver_t) arraylist_get(r_list, r_index);
        arraylist_remove(r_list, r_index);
        free(x);
        flow_stat_logger_sender_app_t y
            = (flow_stat_logger_sender_app_t) arraylist_get(app_list, app_index);
        arraylist_remove(app_list, app_index);
        free(y);

        //network level flow has ended
        if (arraylist_size(app_list) == 0) {
            s->flow_id = -1;

            flow_send_t fs = src_node->host_flows[dst_node->node_index];
            --(fs->num_flow_in_progress);
            if (fs->num_flow_in_progress == 0) {
                --(dst_node->stat.curr_incast_degree);
            }
        }
    }
}

void print_network_tput(int pkt_size, int16_t header_overhead,float timeslot_len)
{
    pkt_size -= header_overhead;
    double data_recvd = 0;
    //double network_tput;
    int count = 0;

    for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
        data_recvd += (nodes[i]->stat.host_pkt_received * pkt_size * 8);
        if (nodes[i]->stat.total_time_active > 0) {
            ++count;
        }
    }

    //network_tput = data_recvd / (curr_timeslot * timeslot_len * 1.0);
}

void log_active_flows(int pkt_size,
                    int16_t header_overhead,
                    float timeslot_len,
                    int64_t time_to_start_logging,
                    int static_workload,
                    FILE* out)
{
    pkt_size -= header_overhead;
    for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
        for (int16_t j = 0; j < NUM_OF_NODES; ++j) {
            arraylist_t r_list
                = nodes[i]->dst_flows[j]->flow_stat_logger_receiver_list;
            assert(i == nodes[i]->dst_flows[j]->dst);
            int64_t size = arraylist_size(r_list);

            for (int16_t k = 0; k < size; ++k) {
                long time_active;
                flow_stat_logger_receiver_t r
                    = (flow_stat_logger_receiver_t) arraylist_get(r_list, k);
                if (static_workload == 1) {
                    time_active = curr_timeslot - time_to_start_logging;
                } else {
                    time_active = curr_timeslot - r->time_app_created;
                }
                long time_active_in_ns = (time_active * timeslot_len);
                double throughput = 0;
                if (time_active > 0) {
                    if( static_workload == 1){
                        throughput = ((double)r->pkt_recvd_since_logging_started
                            * pkt_size * 8.0) / time_active_in_ns;
                    } else{
                        throughput = ((double)r->pkt_recvd * pkt_size * 8.0)
                            / time_active_in_ns;
                    }
                }

                if (static_workload == 1) {
                    fprintf(out, "(%d->%d) %ld/%ldpkt %ld, 1/1us/1Gbps, "
                        "1/1us/1Gbps, 1/1us/1Gbps, 1/1us/1Gbps, "
                        "%ld/%0.3lfus/%0.3lfGbps\n",
                        nodes[i]->dst_flows[j]->src,
                        nodes[i]->dst_flows[j]->dst,
                        nodes[j]->host_flows[i]->pkt_transmitted,
                        r->pkt_recvd_since_logging_started, r->app_id, time_active,
                        (time_active_in_ns * 1e-3), throughput);

                } else {
                   fprintf(out, "(%d->%d) %ldpkt %ldpkt, "
                            "1/1us/1Gbps, 1/1us/1Gbps, "
                            "1/1us/1Gbps, 1/1us/1Gbps, %ld/%0.3lfus/%0.3lfGbps\n",
                        nodes[i]->dst_flows[j]->src,
                        nodes[i]->dst_flows[j]->dst,
                        r->app_flow_size,
                        r->pkt_recvd,
                        //r->app_id,
                        time_active,
                        (time_active_in_ns * 1e-3),
                        throughput);
                }
            }
        }
    }
}

void print_queue_len(spine_t* spines,
            tor_t* tors,
            int16_t* max_queue_len,
            int16_t* _999_percentile,
            int16_t* _99_percentile,
            int16_t* _95_percentile,
            int16_t* _90_percentile,
            int8_t spine_queue,
            int8_t tor_upstream_queue,
            int8_t tor_downstream_queue)
{

    int LEN = 0;

    if (spine_queue) {
        LEN = SPINE_PORT_BUFFER_LEN+1;
    } else if (tor_upstream_queue) {
        LEN = TOR_UPSTREAM_BUFFER_LEN+1;
    } else if (tor_downstream_queue) {
        LEN = TOR_DOWNSTREAM_BUFFER_LEN+1;
    }

    int64_t agg_histogram[NUM_OF_NODES] = {0};

    for (int64_t j = 0; j < LEN; ++j) {
        if (spine_queue) {
            for (int16_t i = 0; i < NUM_OF_SPINES; ++i) {
                agg_histogram[j] +=
                    spines[i]->queue_stat.queue_len_histogram[j];
            }
        } else if (tor_upstream_queue){
            for (int16_t i = 0; i < NUM_OF_TORS; ++i) {
                agg_histogram[j] +=
                    tors[i]->queue_stat.upstream_queue_len_histogram[j];
            }
        } else if (tor_downstream_queue){
            for (int16_t i = 0; i < NUM_OF_TORS; ++i) {
                agg_histogram[j] +=
                    tors[i]->queue_stat.downstream_queue_len_histogram[j];
            }
        }
    }

    for (int64_t i = 0; i < LEN; ++i) {
        if (agg_histogram[i] != 0) {
            *max_queue_len = i;
        }
        if (i != 0) agg_histogram[i] += agg_histogram[i-1];
    }

    int64_t total = agg_histogram[LEN-1];
    double w = 0.999 * total;
    double x = 0.99 * total;
    double y = 0.95 * total;
    double z = 0.90 * total;

    for (int64_t i = LEN - 1; i >= 0; --i) {
        if (w < agg_histogram[i]) {
            *_999_percentile = i;
        }
        if (x < agg_histogram[i]) {
            *_99_percentile = i;
        }
        if (y < agg_histogram[i]) {
            *_95_percentile = i;
        }
        if (z < agg_histogram[i]) {
            *_90_percentile = i;
        }
    }

}

double oracle_fct(int src,
                int dst,
                long flowsize,
                float link_bandwidth,
                int pkt_size,
                int per_hop_propagation_delay_in_timeslots,
                float timeslot_len)
{
    int num_hops;
    if (src / NODES_PER_RACK == dst / NODES_PER_RACK) {
        num_hops = 2;
    } else {
        num_hops = 4;
    }

    double propagation_delay = (num_hops * per_hop_propagation_delay_in_timeslots
                                * timeslot_len);
    double transmission_delay = 0;
    if (flowsize < pkt_size) {
        transmission_delay = ((num_hops * flowsize * 8.0) / link_bandwidth);
    } else {
        transmission_delay = ((num_hops * pkt_size * 8.0) / link_bandwidth);
        transmission_delay += (((flowsize - pkt_size) * 8.0) / link_bandwidth);
    }

    return (propagation_delay + transmission_delay);
}

void print_summary_stats(int pkt_size,
                    int16_t header_overhead,
                    float link_bandwidth,
                    float timeslot_len,
                    int per_hop_propagation_delay_in_timeslots,
                    int64_t time_to_start_logging,
                    int static_workload,
                    int8_t run_till_max_epoch,
                    FILE* f)
{
    fprintf(f, "Flow ID,Src,Dst,Flow Size(bytes),Flow Completion Time(secs),Slowdown,Throughput(Gbps)\n");
    int payload_bits = (pkt_size - header_overhead) * 8;
    for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
        arraylist_t fstat_list = nodes[i]->stat.flow_stat_list;
        int64_t len = arraylist_size(fstat_list);
        for (int64_t j = 0; j < len; ++j) {
            flow_stats_t fstat = (flow_stats_t) arraylist_get(fstat_list, j);
            /*
            double completion_time_1
                = (fstat->sender_completion_time_1 * timeslot_len);
            double goodput_1 = (fstat->flow_size * payload_bits)/completion_time_1;
            double completion_time_2
                = (fstat->sender_completion_time_2 * timeslot_len);
            double goodput_2 = (fstat->flow_size * payload_bits)/completion_time_2;
            double completion_time_3
                = (fstat->receiver_completion_time * timeslot_len);
            double goodput_3 = (fstat->flow_size * payload_bits)/completion_time_3;
            double completion_time_4
                = (fstat->sender_receiver_completion_time * timeslot_len);
            double goodput_4 = (fstat->flow_size * payload_bits)/completion_time_4;
            */
            double completion_time_5
                = (fstat->actual_completion_time * timeslot_len);
            double goodput_5 = (fstat->flow_size * payload_bits)/completion_time_5;

            double ideal_fct = oracle_fct(fstat->src,
                                        fstat->dst,
                                        fstat->flow_size_bytes,
                                        link_bandwidth,
                                        (pkt_size - header_overhead),
                                        per_hop_propagation_delay_in_timeslots,
                                        timeslot_len);
            double slowdown = completion_time_5 / ideal_fct;
            if (slowdown < 1.0) {
                printf("%ld %d %d %ld %lf %lf\n",
                    fstat->flow_id, fstat->src, fstat->dst, fstat->flow_size_bytes,
                    completion_time_5, ideal_fct);
            }
            assert(slowdown >= 1.0);

            fprintf(f, "%ld,%d,%d,%ld,%0.15lf,%0.15lf,%0.15lf\n",
                    fstat->flow_id, fstat->src, fstat->dst, fstat->flow_size_bytes,
                    completion_time_5 * 1e-9, slowdown, goodput_5);
            //fprintf(f, "(%d->%d) %ldpkt %ldpkt, %ld/%0.3lfus/%0.3lfGbps, "
            //        "%ld/%0.3lfus/%0.3lfGbps, %ld/%0.3lfus/%0.3lfGbps, "
            //        "%ld/%0.3lfus/%0.3lfGbps, %ld/%0.3lfus/%0.3lfGbps\n",
            //        fstat->src, fstat->dst, fstat->flow_size, fstat->flow_size,
            //        fstat->sender_completion_time_1, completion_time_1 * 1e-3,
            //        goodput_1, fstat->sender_completion_time_2,
            //        completion_time_2 * 1e-3, goodput_2,
            //        fstat->receiver_completion_time, completion_time_3 * 1e-3,
            //        goodput_3, fstat->sender_receiver_completion_time,
            //        completion_time_4 * 1e-3, goodput_4,
            //        fstat->actual_completion_time, completion_time_5 * 1e-3,
            //        goodput_5);
            free(fstat);
        }
    }

    //log_active_flows(pkt_size,
    //                header_overhead,
    //                timeslot_len,
    //                time_to_start_logging,
    //                static_workload,
    //                f);

    printf("[timeslot = %ld] ", curr_timeslot);
    print_network_tput(pkt_size, header_overhead, timeslot_len);

    int16_t *max_queue_len = (int16_t*) malloc(sizeof(int16_t));
    int16_t *_999_percentile = (int16_t*) malloc(sizeof(int16_t));
    int16_t *_99_percentile = (int16_t*) malloc(sizeof(int16_t));
    int16_t *_95_percentile = (int16_t*) malloc(sizeof(int16_t));
    int16_t *_90_percentile = (int16_t*) malloc(sizeof(int16_t));

    *max_queue_len = 0;
    *_999_percentile = 0;
    *_99_percentile = 0;
    *_95_percentile = 0;
    *_90_percentile = 0;

    print_queue_len(spines, tors, max_queue_len, _999_percentile,
            _99_percentile, _95_percentile, _90_percentile, 1, 0, 0);

    printf("\n********** SPINE PORT QUEUE LEN **********\n");
    printf("90th percentile = %d\n", *_90_percentile);
    printf("95th percentile = %d\n", *_95_percentile);
    printf("99th percentile = %d\n", *_99_percentile);
    printf("99.9th percentile = %d\n", *_999_percentile);
    printf("Maximum = %d\n", *max_queue_len);

    *max_queue_len = 0;
    *_999_percentile = 0;
    *_99_percentile = 0;
    *_95_percentile = 0;
    *_90_percentile = 0;

    print_queue_len(spines, tors, max_queue_len, _999_percentile,
            _99_percentile, _95_percentile, _90_percentile, 0, 1, 0);

    printf("\n********** TOR UPSTREAM PORT QUEUE LEN **********\n");
    printf("90th percentile = %d\n", *_90_percentile);
    printf("95th percentile = %d\n", *_95_percentile);
    printf("99th percentile = %d\n", *_99_percentile);
    printf("99.9th percentile = %d\n", *_999_percentile);
    printf("Maximum = %d\n", *max_queue_len);

    *max_queue_len = 0;
    *_999_percentile = 0;
    *_99_percentile = 0;
    *_95_percentile = 0;
    *_90_percentile = 0;

    print_queue_len(spines, tors, max_queue_len, _999_percentile,
            _99_percentile, _95_percentile, _90_percentile, 0, 0, 1);

    printf("\n********** TOR DOWNSTREAM PORT QUEUE LEN **********\n");
    printf("90th percentile = %d\n", *_90_percentile);
    printf("95th percentile = %d\n", *_95_percentile);
    printf("99th percentile = %d\n", *_99_percentile);
    printf("99.9th percentile = %d\n", *_999_percentile);
    printf("Maximum = %d\n", *max_queue_len);

    free(max_queue_len);
    free(_999_percentile);
    free(_99_percentile);
    free(_95_percentile);
    free(_90_percentile);

    int64_t max_re_ordering = 0;
    for (int i = 0; i < NUM_OF_NODES; ++i) {
        if (nodes[i]->max_re_order_buffer_size > max_re_ordering) {
            max_re_ordering = nodes[i]->max_re_order_buffer_size;
        }
    }
    printf("\nMax re-ordering = %ld cells\n", max_re_ordering);

    printf("\n********** MAX INCAST AT EACH NODE **********\n");
    printf("[");
    for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
        printf("%d, ", nodes[i]->stat.max_incast_degree);
    }
    printf("]\n");

}

