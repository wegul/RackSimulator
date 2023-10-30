#include "driver.h"

// Default values for simulation
static int pkt_size = 8;           // in bytes
static float link_bandwidth = 100; // in Gbps
static float timeslot_len;         // in ns
static int bytes_per_timeslot = 8;

float per_hop_propagation_delay_in_ns = 10;
int per_hop_propagation_delay_in_timeslots;
float per_sw_delay_in_ns = 450;
int per_sw_delay_in_timeslots;

volatile int64_t curr_timeslot = 0; // extern var
int packet_counter = 0;

int burst_size = 175; // = 1400Byte Number of blocks to send in a burst
double load = 1.0;    // Network load

int64_t total_bytes_rcvd = 0;
int64_t total_pkts_rcvd = 0;
float avg_flow_completion_time = 0;

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;
static volatile int8_t terminate4 = 0;
static volatile int8_t terminate5 = 0;

volatile int64_t num_of_flows_finished = 0; // extern var
int64_t num_of_flows_to_finish = 1000;      // stop after these many flows finish

volatile int64_t total_flows_started = 0; // extern var

volatile int64_t max_bytes_rcvd = 1000000;

// Output files
FILE *out_fp = NULL;
FILE *timeseries_fp = NULL;
FILE *spine_outfiles[NUM_OF_SPINES];
FILE *tor_outfiles[NUM_OF_RACKS];
FILE *host_outfiles[NUM_OF_NODES];

// Network
node_t *nodes;
tor_t *tors;
links_t links;
flowlist_t *flowlist;

void work_per_timeslot()
{
    printf("Simulation started\n");
    int flow_idx = 0;

    while (1)
    {
        /*---------------------------------------------------------------------------*/
        // Activate inactive flows
        /*---------------------------------------------------------------------------*/
        for (; flow_idx < flowlist->num_flows; flow_idx++)
        {
            flow_t *flow = flowlist->flows[flow_idx];
            if (flow == NULL || flow->timeslot > curr_timeslot)
            {
                continue;
            }
            else if (flow->timeslot == curr_timeslot)
            {
                int src = flow->src;
                if (flow->isMemFlow)
                {
                    buffer_put(nodes[src]->active_mem_flows, flow);
                }
                else
                {
                    buffer_put(nodes[src]->active_flows, flow);
                }
            }
        }

        /*---------------------------------------------------------------------------*/
        // ToR -- PROCESS
        /*---------------------------------------------------------------------------*/

        tor_t tor = tors[0];
        int16_t tor_index = tor->tor_index;

        for (int j = 0; j < NODES_PER_RACK; j++)
        {

            packet_t net_pkt = NULL, mem_pkt = NULL;
            net_pkt = (packet_t)buffer_get(tor->upstream_pkt_buffer[j]); // this is recved from hosts, now need to forward
            // Move packet to send buffer
            while (net_pkt)
            {
                int dst_host = net_pkt->dst_node % NODES_PER_RACK;
                // push into down stream buffer
                // printf("tor proc (%d), cnt: %d, bufsize: %d?\n",
                // net_pkt->isMemPkt, net_pkt->pkt_id, tor->downstream_send_buffer[dst_host]->num_elements + 1);
                assert(buffer_put(tor->downstream_send_buffer[dst_host], net_pkt) != -1);
                net_pkt = (packet_t)buffer_get(tor->upstream_pkt_buffer[j]);
            }
            mem_pkt = (packet_t)buffer_get(tor->upstream_mem_buffer[j]);
            while (mem_pkt)
            {
                // printf("tor proc (%d), cnt: %d\n", mem_pkt->isMemPkt, mem_pkt->pkt_id);
                int dst_host = mem_pkt->dst_node % NODES_PER_RACK;
                assert(buffer_put(tor->downstream_mem_buffer[dst_host], mem_pkt) != -1);
                mem_pkt = (packet_t)buffer_get(tor->upstream_mem_buffer[j]);
            }
        }

        /*---------------------------------------------------------------------------*/
        // HOST -- SEND
        /*---------------------------------------------------------------------------*/
        for (int i = 0; i < NUM_OF_NODES; ++i)
        {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;
            flow_t *flow = NULL;
            // whenever there is a mem flow, send it
            if ((node->active_mem_flows)->num_elements > 0)
            {
                // RULE: a host has to finish a mem flow before starting a net flow
                if (node->current_flow == NULL)
                {
                    node->current_flow = buffer_remove(node->active_mem_flows, 0);
                }
                // If host has a current mem flow, retain it, but if it is a netflow, shut it.
                else if (!node->current_flow->isMemFlow)
                {
                    // Preemption: Return the current net flow back to the active flows list and select a new mem flow
                    buffer_put(node->active_flows, node->current_flow);
                    node->current_flow = buffer_remove(node->active_mem_flows, 0);
                }
            }
            // Check if host has any active net flows at current moment
            else if ((node->active_flows)->num_elements > 0)
            {
                // Time to select a new burst flow
                if (curr_timeslot % burst_size == 0 || node->current_flow == NULL)
                {
                    // Return the current flow back to the active flows list
                    if (node->current_flow != NULL)
                    {
                        buffer_put(node->active_flows, node->current_flow);
                        node->current_flow = NULL;
                    }
                    // Randomly select a new active flow to start
                    int32_t num_af = node->active_flows->num_elements;
                    int32_t selected = (int32_t)rand() % num_af;
                    node->current_flow = buffer_remove(node->active_flows, selected);
                }
            }

            flow = node->current_flow;

            // now that flow is selected, start sending packet
            // send packet
            if (flow)
            {
                int16_t src_node = flow->src;
                int16_t dst_node = flow->dst;
                int64_t flow_id = flow->flow_id;
                // Mem flow send packet
                if (flow->isMemFlow)
                {
                    int64_t flow_bytes_remaining = flow->flow_size_bytes - flow->bytes_sent;
                    if (flow_bytes_remaining > 0)
                    {
                        int64_t size = BLK_SIZE;
                        packet_t mem_pkt = create_packet(src_node, dst_node, flow_id, size /*size*/, node->seq_num[flow_id], packet_counter++);
                        node->seq_num[flow_id] += size;
                        mem_pkt->isMemPkt = 1;
                        // Send packet
                        mem_pkt->time_when_transmitted_from_src = curr_timeslot;
                        int16_t dst_tor = node_index / NODES_PER_RACK;
                        mem_pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][dst_tor], mem_pkt);

                        flow->pkts_sent++;
                        flow->bytes_sent += size;
                        // Update flow state
                        if (flow->active == 0)
                        {
                            flowlist->active_flows++;
                            flow->start_timeslot = curr_timeslot;
                            total_flows_started++;
                        }
                        flow->active = 1;
                        flow->timeslots_active++;
                        // if (mem_pkt)
                        // printf("host sent (1), cnt: %d, seq:%d, deq: %d\n", mem_pkt->pkt_id, mem_pkt->seq_num, mem_pkt->time_to_dequeue_from_link);
                    }

                    if (flow_bytes_remaining < 1)
                    {
                        node->current_flow = NULL;
                    }
                }
                else // Send packets from this flow until cwnd is reached or the flow runs out of bytes to send
                {
                    int64_t flow_bytes_remaining = flow->flow_size_bytes - flow->bytes_sent;
                    int64_t flow_bytes_unacked = flow->bytes_sent - flow->bytes_received;
                    int64_t cwnd_bytes_remaining = node->cwnd[flow_id] * BLK_SIZE - (node->seq_num[flow_id] - node->last_acked[flow_id]);
                    int64_t size = BLK_SIZE;
                    packet_t pkt = NULL;

                    // Check outstanding, if timeout, retransmit
                    if (flow_bytes_unacked > 0 && curr_timeslot - node->last_ack_time[flow->flow_id] > TIMEOUT)
                    {
                        pkt = create_packet(src_node, dst_node, flow_id, size, node->last_acked[flow->flow_id], packet_counter);
                        packet_counter++;
                        printf("timeout!! flowid:%d , %d->%d, last ack time:%d, curr:%d, cwnd: %d\n",
                               flow->flow_id, flow->src, flow->dst, node->last_ack_time[flow->flow_id], curr_timeslot, node->cwnd[flow_id]);
                        // Refresh last_ack_time so that no contiguous retrans
                        node->last_ack_time[flow->flow_id] = curr_timeslot;
                        flow_bytes_unacked -= size;
                    }
                    // If no retrans and there are new packets need to be sent (some packets are not received/acked)
                    else if (flow_bytes_remaining > 0 && cwnd_bytes_remaining > 0)
                    {
                        // Determine how large the packet will be
                        size = BLK_SIZE;
                        // Create packet
                        pkt = create_packet(src_node, dst_node, flow_id, size, node->seq_num[flow_id], packet_counter);
                        packet_counter++;
                        node->seq_num[flow_id] += size;
                        // Update flow state
                        if (flow->active == 0)
                        {
                            flowlist->active_flows++;
                            flow->start_timeslot = curr_timeslot;
                            total_flows_started++;
                        }
                        flow->active = 1;
                        flow->bytes_sent += size;
                        flow->pkts_sent++;
                        flow->timeslots_active++;
                    }
                    // Send packet
                    if (pkt)
                    {
                        pkt->time_when_transmitted_from_src = curr_timeslot;
                        int16_t dst_tor = node_index / NODES_PER_RACK;
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
                        // printf("host sent (0), cnt: %d, seq: %d, deq: %d\n", pkt->pkt_id, pkt->seq_num, pkt->time_to_dequeue_from_link);
                    }
                    // Set current flow back to null if there are no more bytes left to send from this flow
                    if (flow_bytes_remaining < 1)
                    {
                        node->current_flow = NULL;
                    }
                }
            }
        }

        /*---------------------------------------------------------------------------*/
        // ToR -- SEND TO HOST
        /*---------------------------------------------------------------------------*/

        // send to each host
        for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port)
        {
            //=======debug traverse downstream buffer=========
            packet_t peek_pkt = NULL;
            buffer_t *fifo = tor->downstream_send_buffer[tor_port];
            // Traverse the link to see if we have proper packet to dequeue;
            for (int i = 0; i < fifo->num_elements; i++)
            {
                peek_pkt = (packet_t)buffer_peek(tor->downstream_send_buffer[tor_port], i);
                if (peek_pkt)
                {
                    // printf("net downstream buf has cnt: %d, num: %d, curr:%d\n", peek_pkt->pkt_id, fifo->num_elements, curr_timeslot);
                }
            }
            // fifo = tor->downstream_mem_buffer[tor_port];
            // peek_pkt = NULL;
            // for (int i = 0; i < fifo->num_elements; i++)
            // {
            //     peek_pkt = (packet_t)buffer_peek(tor->downstream_mem_buffer[tor_port], i);
            //     if (peek_pkt)
            //     {
            //         printf("mem downstream buf has cnt: %d, idx: %d, num: %d, curr:%d\n", peek_pkt->pkt_id, i, fifo->num_elements, curr_timeslot);
            //     }
            // }
            //==============================
            // prioritize mem traffic
            packet_t pkt = (packet_t)buffer_get(tor->downstream_mem_buffer[tor_port]);
            if (pkt)
            {
                int16_t dst_host = pkt->dst_node;
                pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->tor_to_host_link[tor_index][dst_host], pkt);
                printf("tor sent (%d), cnt: %d\n\n", pkt->isMemPkt, pkt->pkt_id);
            }
            else // No mem, send net if any.
            {
                pkt = (packet_t)buffer_get(tor->downstream_send_buffer[tor_port]);
                if (pkt)
                {
                    int16_t dst_host = pkt->dst_node;
                    pkt->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->tor_to_host_link[tor_index][dst_host], pkt);
                    printf("tor sent (%d), cnt: %d\n\n", pkt->isMemPkt, pkt->pkt_id);
                }
            }
        }

        /*---------------------------------------------------------------------------*/
        // ToR -- RECV FROM HOST
        /*---------------------------------------------------------------------------*/

        // Recv packet from host
        for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port)
        {
            // deq packet from the link
            int16_t src_host = (tor_index * TOR_PORT_COUNT_LOW) + tor_port;
            packet_t peek_pkt = NULL, pkt = NULL;
            buffer_t *fifo = links->host_to_tor_link[src_host][tor_index]->fifo;
            // Traverse the link to see if we have proper packet to dequeue;
            for (int i = 0; i < fifo->num_elements; i++)
            {
                peek_pkt = (packet_t)link_peek(links->host_to_tor_link[src_host][tor_index], i);
                if (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link <= curr_timeslot)
                {
                    pkt = link_get(links->host_to_tor_link[src_host][tor_index], i--);
                    if (peek_pkt->time_to_dequeue_from_link < curr_timeslot)
                    {
                        printf("conflict pkt(%d) cnt:%d, deq: %d, curr: %d\n",
                               pkt->isMemPkt, pkt->pkt_id, pkt->time_to_dequeue_from_link, curr_timeslot);
                    }
                    // If already found a pkt, it is either mem or net
                    // if (pkt)
                    // {
                    //     // If it is net, replace.
                    //     if (!pkt->isMemPkt)
                    //     {
                    //         printf("conflict drop pktid: %d\n", pkt->pkt_id);
                    //         pkt = link_get(links->host_to_tor_link[src_host][tor_index], i);
                    //     }
                    // }
                    // else
                    // {
                    //     pkt = link_get(links->host_to_tor_link[src_host][tor_index], i);
                    //     i -= 1;
                    // }
                }
            }

            if (pkt)
            {
                if (pkt->isMemPkt)
                {
                    printf("tor recv (%d), cnt: %d, deq: %d, curr: %d\n", pkt->isMemPkt, pkt->pkt_id, pkt->time_to_dequeue_from_link, curr_timeslot);
                    buffer_put(tor->upstream_mem_buffer[tor_port], pkt);
                }
                else
                {
                    int8_t dropped = pkt_recv(tor->upstream_pkt_buffer[tor_port], pkt);
                    // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                    if (tor->upstream_pkt_buffer[tor_port]->num_elements > ECN_CUTOFF_TOR_UP)
                    {
                        pkt->ecn_flag = 1;
                    }
                    // dropped data packets
                    if (dropped < 0)
                    {
                        printf("no space dropping %d", pkt->pkt_id);
                        fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, data, dropped\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)tor_port, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src);
                    }
                    else
                    {
                        printf("tor recv (%d), cnt: %d, deq: %d, curr: %d\n", pkt->isMemPkt, pkt->pkt_id, pkt->time_to_dequeue_from_link, curr_timeslot);

                        if (pkt->control_flag == 1)
                        {
                            fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, ack\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)tor_port, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src);
                        }
                        else
                        {
                            fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, data\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)tor_port, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src);
                        }
                    }
                }
            }
        }

        /*---------------------------------------------------------------------------*/
        // HOST -- RECV
        /*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_NODES; ++i)
        {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;

            // deq packet
            int16_t src_tor = node_index / NODES_PER_RACK;

            packet_t peek_pkt = NULL, pkt = NULL;
            buffer_t *fifo = links->tor_to_host_link[src_tor][node_index]->fifo;
            // Traverse the link to see if we have proper packet to dequeue;
            for (int i = 0; i < fifo->num_elements; i++)
            {
                peek_pkt = (packet_t)link_peek(links->tor_to_host_link[src_tor][node_index], i);
                if (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link == curr_timeslot)
                {
                    pkt = link_get(links->tor_to_host_link[src_tor][node_index], i);
                }
            }
            if (pkt)
            {
                assert(pkt->dst_node == node_index);
                // Data Packet
                if (pkt->control_flag == 0)
                {
                    if (pkt->isMemPkt)
                    {
                        fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, mem, %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->seq_num);
                    }
                    else
                    {
                        fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, net, %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->seq_num);
                    }
                    // if (pkt)
                    // {
                    //     printf("host recv (%d), cnt: %d, seq:%d, bytes recv: %d, deq: %d\n", pkt->isMemPkt, pkt->pkt_id, pkt->seq_num, flowlist->flows[pkt->flow_id]->bytes_received, pkt->time_to_dequeue_from_link);
                    // }
                    // Only create ACK if seq-last_ack >= 1400
                    if (!pkt->isMemPkt && pkt->seq_num - node->ack_num[pkt->flow_id] > 1400)
                    {
                        node->ack_num[pkt->flow_id] = pkt->seq_num + pkt->size;
                        packet_t ack = ack_packet(pkt, node->ack_num[pkt->flow_id]);
                        ack->isMemPkt = pkt->isMemPkt;
                        // Respond with ACK packet
                        ack->time_when_transmitted_from_src = curr_timeslot;
                        int16_t dst_tor = node_index / NODES_PER_RACK;
                        ack->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][dst_tor], ack);
                    }

                    // Update flow
                    flow_t *flow = flowlist->flows[pkt->flow_id];
                    assert(flow != NULL);
                    flow->pkts_received++;
                    flow->bytes_received += pkt->size;
                    if (flow->bytes_received != pkt->seq_num + pkt->size)
                    {
                        printf("dsit\n");
                    }
                    total_bytes_rcvd += pkt->size;
                    total_pkts_rcvd++;
                    // Determine if last packet of flow has been received
                    if (flow->bytes_received >= flow->flow_size_bytes)
                    {
                        flow->active = 0;
                        flowlist->active_flows--;
                        flow->finished = 1;
                        flow->finish_timeslot = curr_timeslot;
                        num_of_flows_finished++;
                        write_to_outfile(out_fp, flow, timeslot_len, link_bandwidth);
                        // printf("%d flows finished\n", num_of_flows_finished);
                        printf("%d: Flow %d finished in %d timeslots\n", (int)curr_timeslot, (int)flow->flow_id, (int)(flow->finish_timeslot - flow->timeslot));
                        fflush(stdout);
                    }
                }
                // Control Packet, then this node is a sender node
                else
                {
                    fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, netack, %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->seq_num);
                    // Check ECN flag
                    track_ecn(node, pkt->flow_id, pkt->ecn_flag);
                    flow_t *flow = flowlist->flows[pkt->flow_id];
                    // Check ACK value
                    if (pkt->ack_num > node->last_acked[pkt->flow_id])
                    {
                        node->last_acked[pkt->flow_id] = pkt->ack_num;
                        node->last_ack_time[pkt->flow_id] = curr_timeslot;
                    }
                    else if (pkt->ack_num == node->last_acked[pkt->flow_id]) // Duplicate ack
                    {
                        node->last_ack_time[pkt->flow_id] -= 40; // fast retransmit
                    }
                }
                free_packet(pkt);
            }
        }

        /*---------------------------------------------------------------------------*/
        // State updates before next iteration
        /*---------------------------------------------------------------------------*/

        if (flowlist->active_flows < 1)
        {
            int no_flows_left = 1;
            for (int i = 0; i < flowlist->num_flows; i++)
            {
                flow_t *flow = flowlist->flows[i];
                if (flow != NULL && flow->finished == 0)
                {
                    no_flows_left = 0;
                }
            }
            if (no_flows_left > 0)
            {
                printf("Finished all flows\n\n");
                terminate0 = 1;
            }
        }

        if (num_of_flows_finished >= num_of_flows_to_finish)
        {
            printf("\nFinished %d flows\n\n", (int)num_of_flows_finished);
            terminate2 = 1;
        }

        if (total_bytes_rcvd >= max_bytes_rcvd)
        {
            printf("\nReached %d bytes received\n\n", max_bytes_rcvd);
            terminate5 = 1;
        }

        if (terminate0 || terminate1 || terminate2 || terminate3 || terminate4 || terminate5)
        {
            int completed_flows = 0;
            int flow_completion_times[flowlist->num_flows];
            int slowdowns[flowlist->num_flows];
            float avg_slowdown = 0;
            printf("FCT: ");
            for (int i = 0; i < flowlist->num_flows; i++)
            {
                if (flowlist->flows[i]->finished > 0)
                {
                    printf("%d, ", flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot);
                    flow_completion_times[completed_flows] = flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot;
                    avg_flow_completion_time += flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot;
                    slowdowns[completed_flows] = flow_completion_times[completed_flows] / flowlist->flows[i]->expected_runtime;
                    avg_slowdown += slowdowns[completed_flows];
                    completed_flows++;
                }
            }
            printf("\n");

            int pct99fct = 0;
            int medfct = 0;
            qsort(flow_completion_times, completed_flows, sizeof(int), comp);

            if (completed_flows > 0)
            {
                avg_flow_completion_time /= completed_flows;
                avg_slowdown /= completed_flows;
                int pct99 = (completed_flows * 99 + 99) / 100;
                pct99fct = flow_completion_times[pct99 - 1];
                int pct50 = (completed_flows * 50) / 100;
                medfct = flow_completion_times[pct50 - 1];
            }

            avg_slowdown -= 1;

            printf("Avg flow completion time: %0.3f\n", avg_flow_completion_time);
            printf("Avg slowdown: %0.3f\n", avg_slowdown);
            printf("Median flow completion time: %d\n", medfct);
            printf("99th %%ile FCT: %d\n", pct99fct);
            printf("Flows completed: %d\n", completed_flows);

            double curr_time = curr_timeslot * timeslot_len / 1e9;
            printf("Finished in %d timeslots\n", (int)curr_timeslot);
            printf("Finished in %f seconds\n", curr_time);
            printf("Finished in %d bytes\n", total_bytes_rcvd);
            fflush(stdout);
            break;
        }
        fflush(stdout);
        curr_timeslot++;
    }
    printf("\nSimulation Ended\n");
}

void process_args(int argc, char **argv)
{
    int opt;
    char filename[500] = "";
    char out_filename[504] = "";
    char out_suffix[4] = ".out";
    char timeseries_filename[515] = "";
    char timeseries_suffix[15] = ".timeseries.csv";

    while ((opt = getopt(argc, argv, "f:b:c:h:d:n:m:t:q:a:e:s:i:u:l:p:k:v:x:y:z:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            num_of_flows_to_finish = atoi(optarg);
            printf("Stop experiment after %ld flows have finished\n", num_of_flows_to_finish);
            break;
        case 'u':
            burst_size = atoi(optarg);
            printf("Using packet burst size of %d\n", burst_size);
            break;
        case 'l':
            load = atof(optarg);
            printf("Using a load of %0.1f\n", load);
            break;
        case 'f':
            if (strlen(optarg) < 500)
            {
                strcpy(filename, optarg);
                strncpy(out_filename, filename, strlen(filename));
                strncat(out_filename, out_suffix, 4);
                // strncpy(timeseries_filename, filename, strlen(filename));
                // strncat(timeseries_filename, timeseries_suffix, 15);
#ifdef RECORD_PACKETS
                printf("Writing switch packet data to switch.csv files\n");
                printf("open switch out tiles %s\n", filename);
                open_switch_outfiles(filename);
                open_host_outfiles(filename);
#endif
            }
            break;
        default:
            printf("Wrong command line argument\n");
            exit(1);
        }
    }

    timeslot_len = (pkt_size * 8) / link_bandwidth;
    printf("Running with a slot length of %fns\n", timeslot_len);

    per_hop_propagation_delay_in_timeslots = round((float)per_hop_propagation_delay_in_ns / (float)timeslot_len);
    per_sw_delay_in_timeslots = round((float)per_sw_delay_in_ns / (float)timeslot_len);
    printf("SW delay: %f ns (%d timeslots)\nPer hop propagation delay: %f ns (%d timeslots)\n",
           per_sw_delay_in_ns, per_sw_delay_in_timeslots, per_hop_propagation_delay_in_ns, per_hop_propagation_delay_in_timeslots);

    bytes_per_timeslot = (int)(timeslot_len * link_bandwidth / 8);
    printf("Bytes sent per timeslot %d\n", bytes_per_timeslot);

    DIR *dir = opendir("out/");
    if (dir)
    {
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        mkdir("out/", 0777);
    }
    else
    {
        printf("Could not open out directory.");
        exit(1);
    }

    read_tracefile(filename);
    out_fp = open_outfile(out_filename);
}
int comp(const void *elem1, const void *elem2)
{
    int f = *((int *)elem1);
    int s = *((int *)elem2);
    if (f > s)
        return 1;
    if (f < s)
        return -1;
    return 0;
}
void read_tracefile(char *filename)
{
    FILE *fp;
    flowlist = create_flowlist();
    if (strcmp(filename, ""))
    {
        printf("Opening tracefile %s\n", filename);
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            perror("open");
            exit(1);
        }
        int flow_id = -1;
        int isMemFlow = -1;
        int src = -1;
        int dst = -1;
        int flow_size_bytes = -1;
        int timeslot = -1;
        // mem and net packets are in together. distinguish them when initializing flow
        while (fscanf(fp, "%d,%d,%d,%d,%d,%d,%d", &flow_id, &isMemFlow, &src, &dst, &flow_size_bytes, &timeslot) >= 6 && flow_id < MAX_FLOW_ID)
        {
            initialize_flow(flow_id, isMemFlow, src, dst, flow_size_bytes, timeslot);
        }

        printf("Flows initialized\n");
        fclose(fp);
    }
    return;
}

void initialize_flow(int flow_id, int isMemFlow, int src, int dst, int flow_size_bytes, int timeslot)
{
    if (flow_id < MAX_FLOW_ID)
    {
        flow_t *new_flow = create_flow(flow_id, flow_size_bytes, src, dst, timeslot);
        new_flow->isMemFlow = isMemFlow;
        new_flow->expected_runtime = flow_size_bytes / BLK_SIZE + 6;
        add_flow(flowlist, new_flow);
        // #ifdef DEBUG_DRIVER
        printf("initialized flow %d (%d), src %d dst %d, flow_size %dB, ts %d\n", flow_id, isMemFlow, src, dst, flow_size_bytes, timeslot);
        // #endif
    }
}

void free_flows()
{
    free_flowlist(flowlist);
    printf("Freed flows\n");
}

void initialize_network()
{
    // create nodes
    nodes = (node_t *)malloc(NUM_OF_NODES * sizeof(node_t));
    MALLOC_TEST(nodes, __LINE__);
    for (int i = 0; i < NUM_OF_NODES; ++i)
    {
        nodes[i] = create_node(i);
    }
    printf("Nodes initialized\n");

    // create ToRs
    tors = (tor_t *)malloc(NUM_OF_TORS * sizeof(tor_t));
    MALLOC_TEST(tors, __LINE__);
    for (int i = 0; i < NUM_OF_TORS; ++i)
    {
        tors[i] = create_tor(i);
    }
    printf("ToRs initialized\n");

    // create links
    links = create_links();
    printf("Links initialized\n");
}

void free_network()
{
    // free nodes
    for (int i = 0; i < NUM_OF_NODES; ++i)
    {
        free_node(nodes[i]);
    }
    free(nodes);

    // free ToRs
    for (int i = 0; i < NUM_OF_TORS; ++i)
    {
        free_tor(tors[i]);
    }
    free(tors);

    // free links
    free_links(links);

    printf("Freed network\n");
}

void open_switch_outfiles(char *base_filename)
{
    char csv_suffix[4] = ".csv";
    for (int i = 0; i < NUM_OF_RACKS; i++)
    {
        char filename[520] = "out/";
        char tor_suffix[4] = ".tor";
        char tor_id[5];
        sprintf(tor_id, "%d", i);
        strncat(filename, base_filename, 500);
        strncat(filename, tor_suffix, 4);
        strncat(filename, tor_id, 5);
        strncat(filename, csv_suffix, 4);
        tor_outfiles[i] = fopen(filename, "w");
        fprintf(tor_outfiles[i], "flow_id, src, dst, port, arrival_time, creation_time\n");
    }
}
void open_host_outfiles(char *base_filename)
{
    char csv_suffix[4] = ".csv";
    for (int i = 0; i < NUM_OF_NODES; i++)
    {
        char filename[520] = "out/";
        char host_suffix[5] = ".host";
        char host_id[5];
        sprintf(host_id, "%d", i);
        strncat(filename, base_filename, 500);
        strncat(filename, host_suffix, 5);
        strncat(filename, host_id, 5);
        strncat(filename, csv_suffix, 4);
        host_outfiles[i] = fopen(filename, "w");
        fprintf(host_outfiles[i], "flow_id, src, dst, arrival_time, creation_time, Type(ack/data), SeqNum\n");
    }
}

void close_outfiles()
{
    fclose(out_fp);

#ifdef RECORD_PACKETS

    for (int i = 0; i < NUM_OF_RACKS; i++)
    {
        fclose(tor_outfiles[i]);
    }
    for (int i = 0; i < NUM_OF_NODES; i++)
    {
        fclose(host_outfiles[i]);
    }

#endif
}

int main(int argc, char **argv)
{
    srand((unsigned int)time(NULL));

    initialize_network();
    process_args(argc, argv);

    work_per_timeslot();

    free_flows();
    free_network();
    close_outfiles();

    printf("Finished execution\n");

    return 0;
}