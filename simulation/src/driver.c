#include "driver.h"

// Default values for simulation
static int pkt_size = BLK_SIZE;    // in bytes
static float link_bandwidth = 100; // in Gbps
static float timeslot_len;         // in ns
static int bytes_per_timeslot = 8;

float per_hop_propagation_delay_in_ns = 10;
int per_hop_propagation_delay_in_timeslots;
float per_sw_delay_in_ns = 500;
int per_sw_delay_in_timeslots;

volatile int64_t curr_timeslot = 0; // extern var
int packet_counter = 0;

int burst_size = 175; // = 1500Byte Number of blocks to send in a burst
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
FILE *sw_queue_fp = NULL;
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
        for (flow_idx = 0; flow_idx < flowlist->num_flows; flow_idx++)
        {
            flow_t *flow = flowlist->flows[flow_idx];
            if (flow == NULL || flow->timeslot > curr_timeslot)
            {
                continue;
            }
            else if (flow->timeslot == curr_timeslot && !flow->finished && !flow->active)
            {
                // printf("activate flow: %d, curr: %d\n", flow->flow_id, curr_timeslot);
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
        int16_t tor_index = 0;
        // Peek traverse NotificationQueue and give GRANT
        for (int i = 0; i < MAX_FLOW_ID; i++)
        {
            notif_t ntf = tor->notif_queue[i];

            if (!ntf->isGranted && ntf->req_type >= 0) // Notification Requests for buffer
            {
                // Check if available: only grant one sender per DOWNSTREAM port
                if (!tor->downstream_mem_buffer_lock[ntf->receiver])
                {
                    // printf("Grant to %d, curr: %d\n", ntf->sender, curr_timeslot);
                    tor->ntf_cnt--;
                    ntf->isGranted = 1;
                    tor->downstream_mem_buffer_lock[ntf->receiver] = 1;
                    // Send grant to future msg_sender
                    packet_t grant = create_packet(ntf->receiver, ntf->sender, i, BLK_SIZE, -1, packet_counter++);
                    grant->isMemPkt = 1;
                    grant->memType = 200;
                    grant->req_len = ntf->length;
                    if (tor->downstream_mem_buffer[ntf->sender]->num_elements > 4 * NODES_PER_RACK)
                    {
                        printf("GRANT num: %d, type: %d\n", tor->downstream_mem_buffer[ntf->sender]->num_elements, grant->memType);
                    }
                    assert("TOR GRANT OVERFLOW" && pkt_recv(tor->downstream_mem_buffer[ntf->sender], grant) != -1);
#ifdef RECORD_PACKETS
                    fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, grant\n", (int)grant->flow_id, (int)grant->src_node, (int)grant->dst_node, (int)grant->dst_node, (int)(curr_timeslot), (int)grant->time_when_transmitted_from_src);
#endif
                }
            }
        }
        // Forwarding: extract from ingress port (UPTREAM) to egress port (DOWNSTREAM)
        for (int j = 0; j < NODES_PER_RACK; j++)
        {
            packet_t net_pkt = NULL, mem_pkt = NULL;
            mem_pkt = (packet_t)buffer_get(tor->upstream_mem_buffer[j]);
            while (mem_pkt)
            {
                int dst_host = mem_pkt->dst_node;
                if (tor->downstream_mem_buffer[dst_host]->num_elements > TOR_DOWNSTREAM_MEMBUF_LEN - 64)
                {
                    printf("flow remain: %d ", flowlist->flows[mem_pkt->flow_id]->flow_size_bytes - mem_pkt->seq_num);
                    print_packet(mem_pkt);
                    // printf("PROC num: %d, type: %d, time: %d\n", tor->downstream_mem_buffer[dst_host]->num_elements, mem_pkt->memType, curr_timeslot);
                    fflush(stdout);
                }

                assert("TOR PROC OVERFLOW" && pkt_recv(tor->downstream_mem_buffer[dst_host], mem_pkt) != -1);
                // printf("tor recv (%d), cnt: %d %d-%d, flowid: %d, curr: %d\n", mem_pkt->isMemPkt, mem_pkt->pkt_id, mem_pkt->src_node, mem_pkt->dst_node, mem_pkt->flow_id, curr_timeslot);
#ifdef RECORD_PACKETS
                fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, mem\n", (int)mem_pkt->flow_id, (int)mem_pkt->src_node, (int)mem_pkt->dst_node, (int)mem_pkt->dst_node, (int)(curr_timeslot), (int)mem_pkt->time_when_transmitted_from_src);
#endif
                mem_pkt = (packet_t)buffer_get(tor->upstream_mem_buffer[j]);
            }
            net_pkt = (packet_t)buffer_get(tor->upstream_pkt_buffer[j]); // this is recved from hosts, now need to forward
            // Move packet to send buffer
            while (net_pkt)
            {
                int dst_host = net_pkt->dst_node;
                // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                if (tor->downstream_send_buffer[dst_host]->num_elements > ECN_CUTOFF_TOR_DOWN)
                {
                    net_pkt->ecn_flag = 1;
                }
                // Push into down stream buffer; drop data packets if egress queue has no space
                int dropped = pkt_recv(tor->downstream_send_buffer[dst_host], net_pkt);
                if (dropped < 0)
                {
// printf("NET egress port to host: %d drops %d at %d\n", net_pkt->dst_node, net_pkt->pkt_id, curr_timeslot);
#ifdef RECORD_PACKETS
                    fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, net, dropped\n", (int)net_pkt->flow_id, (int)net_pkt->src_node, (int)net_pkt->dst_node, (int)net_pkt->dst_node, (int)(curr_timeslot), (int)net_pkt->time_when_transmitted_from_src);
#endif
                }
                else
                {
                    // printf("tor recv (%d), cnt: %d %d-%d, deq: %d, curr: %d\n", net_pkt->isMemPkt, net_pkt->pkt_id, net_pkt->src_node, net_pkt->dst_node, net_pkt->time_to_dequeue_from_link, curr_timeslot);
#ifdef RECORD_PACKETS
                    if (net_pkt->control_flag == 1)
                    {
                        fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, ack\n", (int)net_pkt->flow_id, (int)net_pkt->src_node, (int)net_pkt->dst_node, (int)net_pkt->dst_node, (int)(curr_timeslot), (int)net_pkt->time_when_transmitted_from_src);
                    }
                    else
                    {
                        fprintf(tor_outfiles[0], "%d, %d, %d, %d, %d, %d, data\n", (int)net_pkt->flow_id, (int)net_pkt->src_node, (int)net_pkt->dst_node, (int)net_pkt->dst_node, (int)(curr_timeslot), (int)net_pkt->time_when_transmitted_from_src);
                    }
#endif
                }
                net_pkt = (packet_t)buffer_get(tor->upstream_pkt_buffer[j]);
            }
        }

        // Update queue info
        // for (int j = 0; j < NODES_PER_RACK; j++)
        // {
        //     fprintf(sw_queue_fp, "%d, ", tor->downstream_send_buffer[j]->num_elements);
        // }
        // for (int j = 0; j < NODES_PER_RACK; j++)
        // {
        //     fprintf(sw_queue_fp, "%d, ", tor->downstream_mem_buffer[j]->num_elements);
        // }
        // fprintf(sw_queue_fp, "\n");

        /*---------------------------------------------------------------------------*/
        // HOST -- SEND
        /*---------------------------------------------------------------------------*/
        for (int i = 0; i < NUM_OF_NODES; ++i)
        {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;
            flow_t *flow = NULL;
            // Check if host has any active mem flows at current moment; and if yes, check if it is granted
            if ((node->active_mem_flows)->num_elements > 0)
            {
                if (node->current_flow)
                {
                    // Check if nee to put it back
                    if (!node->current_flow->isMemFlow)
                    {
                        buffer_put(node->active_flows, node->current_flow);
                        node->current_flow = NULL;
                    }
                    else if (node->current_flow->memType == 998)
                    {
                        buffer_put(node->active_mem_flows, node->current_flow);
                        node->current_flow = NULL;
                    }
                }
                if (node->current_flow == NULL)
                {
                    // Peek Traverse to see if there is a granted(1/2) or need-to-notify(0/999) mem flow, choose this flow
                    for (int j = 0; j < (node->active_mem_flows)->num_elements; j++)
                    {
                        flow_t *peek_flow = (flow_t *)buffer_peek(node->active_mem_flows, j);
                        if (peek_flow->memType != 998)
                        {
                            node->current_flow = buffer_remove(node->active_mem_flows, j);
                            break;
                        }
                    }
                }
            }
            // Check if host has any active net flows at current moment
            if (node->current_flow == NULL && (node->active_flows)->num_elements > 0)
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
                    int32_t selected = 0;
                    // int32_t selected = (int32_t)rand() % num_af;
                    node->current_flow = buffer_remove(node->active_flows, selected);
                }
            }

            flow = node->current_flow;

            // Now that flow is selected, start sending packet
            if (flow)
            {
                int16_t src_node = flow->src;
                int16_t dst_node = flow->dst;
                int64_t flow_id = flow->flow_id;
                // Mem flow send packet
                if (flow->isMemFlow)
                {
                    assert("FLOW->MEMTYPE ERROR" && flow->memType >= 0);
                    // Notification
                    // WREQ, if not yet notify
                    if (flow->memType == 999)
                    {
                        flow->memType--; // Notified
                        // printf("Flow %d send WREQ Notif to %d, curr: %d\n", flow->flow_id, flow->dst, curr_timeslot);
                        // Push a Notification in link.
                        packet_t ntf_pkt = create_packet(src_node, dst_node, flow_id, BLK_SIZE, -1, packet_counter++);
                        ntf_pkt->isMemPkt = 1;
                        ntf_pkt->memType = 0x0c; // Notification for WREQ
                        ntf_pkt->req_len = flow->flow_size_bytes;
                        ntf_pkt->time_when_transmitted_from_src = curr_timeslot;
                        ntf_pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][0], ntf_pkt);
                    }
                    else if (flow->memType < 3) // It is RREQ, granted RRESP or granted WREQ, so put packet in link and update flow status
                    {
                        int64_t flow_bytes_remaining = flow->flow_size_bytes - node->seq_num[flow->flow_id];
                        if (flow->flow_size_bytes - flow->bytes_received < 1)
                        {
                            node->current_flow = NULL;
                            continue;
                        }
                        if (flow_bytes_remaining > 0)
                        {
                            int64_t size = BLK_SIZE;
                            packet_t mem_pkt = create_packet(src_node, dst_node, flow_id, size, node->seq_num[flow_id], packet_counter++);
                            node->seq_num[flow_id] += size;
                            mem_pkt->isMemPkt = 1;
                            if (flow->memType == 0) // RREQ, check if it is header (first).
                            {
                                if (flow->bytes_sent > 0) // Not Header
                                {
                                    mem_pkt->memType = 0x1a;
                                }
                                else
                                {
                                    mem_pkt->memType = 0x0a; // RREQ header
                                    mem_pkt->req_len = flow->rreq_bytes;
                                }
                            }
                            else if (flow->memType == 1) // RRESP
                            {
                                if (flow_bytes_remaining <= BLK_SIZE) // The last block
                                {
                                    mem_pkt->memType = 0x2b;
                                }
                                else
                                    mem_pkt->memType = 0x1b;
                            }
                            else if (flow->memType == 2) // WREQ
                            {
                                if (flow_bytes_remaining <= BLK_SIZE) // The last block
                                {
                                    mem_pkt->memType = 0x2c;
                                }
                                else
                                {
                                    mem_pkt->memType = 0x1c;
                                }
                            }
                            assert("mem_pkt memtype error" && mem_pkt->memType >= 0);
                            // Send packet
                            mem_pkt->time_when_transmitted_from_src = curr_timeslot;
                            mem_pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                            link_enqueue(links->host_to_tor_link[node_index][0], mem_pkt);

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
                            // if (mem_pkt->memType == 0x2b || mem_pkt->memType == 0x2c)
                            // {
                            //     if (!flow->released)
                            //     {
                            //         flow->released=1;
                            //         tor->downstream_mem_buffer_lock[mem_pkt->dst_node] = 0;
                            //     }

                            // }

                            // printf("host sent (1,%02x), flowid: %d, cnt: %d, seq:%d, deq: %d\n", mem_pkt->memType, mem_pkt->flow_id, mem_pkt->pkt_id, mem_pkt->seq_num, mem_pkt->time_to_dequeue_from_link);
                        }
                    }
                }
                else // Send packets from this flow until cwnd is reached or the flow runs out of bytes to send
                {
                    if (flow->flow_size_bytes - flow->bytes_received < 1)
                    {
                        node->current_flow = NULL;
                        continue;
                    }
                    int64_t size = BLK_SIZE;
                    int64_t flow_bytes_remaining = flow->flow_size_bytes - node->seq_num[flow->flow_id];
                    int64_t flow_bytes_unacked = node->seq_num[flow->flow_id] - node->last_acked[flow->flow_id];
                    int64_t cwnd_bytes_remaining = node->cwnd[flow_id] * BLK_SIZE - flow_bytes_unacked;
                    packet_t pkt = NULL;
                    // printf("remainning: %d, unacked: %d, cwnd remain: %d\n", flow_bytes_remaining, flow_bytes_unacked, cwnd_bytes_remaining);
                    // Check outstanding, if timeout, retransmit
                    if (flow_bytes_unacked > 0 && curr_timeslot - node->last_ack_time[flow->flow_id] > TIMEOUT)
                    {
                        node->seq_num[flow->flow_id] = node->last_acked[flow->flow_id];
                        pkt = create_packet(src_node, dst_node, flow_id, size, node->seq_num[flow->flow_id], packet_counter++);
                        node->seq_num[flow_id] += size;
                        // Refresh timer
                        node->last_ack_time[flow->flow_id] = curr_timeslot;
                        // printf("timeout!! flowid:%d , %d->%d, last ack time:%d, last_ack: %d, curr:%d, cwnd: %d\n",
                        //        flow->flow_id, flow->src, flow->dst, node->last_ack_time[flow->flow_id], node->last_acked[flow->flow_id], curr_timeslot, node->cwnd[flow_id]);
                    }
                    // If no retrans and there are new packets need to be sent (some packets are not received/acked)
                    else if (flow_bytes_remaining > 0 && cwnd_bytes_remaining > 0)
                    {
                        // Create packet
                        pkt = create_packet(src_node, dst_node, flow_id, size, node->seq_num[flow_id], packet_counter++);
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
                        // Set current flow back to null if there are no more bytes left to send from this flow
                    }

                    // Send packet
                    if (pkt)
                    {
                        pkt->time_when_transmitted_from_src = curr_timeslot;
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][0], pkt);
                        // printf("host sent (0), cnt: %d, seq: %d, deq: %d\n", pkt->pkt_id, pkt->seq_num, pkt->time_to_dequeue_from_link);
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
            // prioritize mem traffic
            packet_t pkt = (packet_t)buffer_get(tor->downstream_mem_buffer[tor_port]);
            if (pkt)
            {
                // printf("tor sent (%d), cnt: %d %d-%d, flowid: %d, queueing: %d, buf: %d/%d, curr: %d\n", pkt->isMemPkt, pkt->pkt_id, pkt->src_node, pkt->dst_node, pkt->flow_id, curr_timeslot - pkt->time_to_dequeue_from_link, tor->downstream_mem_buffer[tor_port]->num_elements, tor->downstream_mem_buffer[tor_port]->size, curr_timeslot);
                pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->tor_to_host_link[tor_index][tor_port], pkt);
            }
            else // No mem, send net if any.
            {
                pkt = (packet_t)buffer_get(tor->downstream_send_buffer[tor_port]);
                if (pkt)
                {
                    // printf("tor sent (%d), cnt: %d %d-%d, seq: %d, queueing: %d, buf: %d/%d, curr: %d\n", pkt->isMemPkt, pkt->pkt_id, pkt->src_node, pkt->dst_node, pkt->seq_num, curr_timeslot - pkt->time_to_dequeue_from_link, tor->downstream_send_buffer[tor_port]->num_elements, tor->downstream_send_buffer[tor_port]->size, curr_timeslot);
                    pkt->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->tor_to_host_link[tor_index][tor_port], pkt);
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
            int16_t src_host = tor_port;
            packet_t peek_pkt = NULL, pkt = NULL;
            // Sort the link to see if we have proper packet to dequeue;
            int link_num = links->host_to_tor_link[src_host][tor_index]->fifo->num_elements;
            int64_t min_time = INT64_MAX - 1;
            int min_idx = -1, cnt_link = 0;
            for (int i = 0; i < link_num; i++)
            {
                peek_pkt = (packet_t)link_peek(links->host_to_tor_link[src_host][tor_index], i);
                if (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link < min_time && peek_pkt->time_to_dequeue_from_link <= curr_timeslot)
                {
                    if (!peek_pkt->control_flag) // is data
                    {
                        cnt_link++;
                    }
                    min_time = peek_pkt->time_to_dequeue_from_link;
                    min_idx = i;
                }
            }
            assert("host to TOR link num!!!" && cnt_link < 3);

            if (min_idx >= 0)
            {
                pkt = link_get(links->host_to_tor_link[src_host][tor_index], min_idx);
                if (pkt->isMemPkt)
                {
                    assert("PKT->MEMTYPE ERROR" && pkt->memType > 0);
                    // If it is a notification or the header of RREQ, put a Notif in NotificationQueue, mark UNGRANTED
                    if (pkt->memType < 0x0f) // Could be 0a, 0c
                    {
                        tor->ntf_cnt++;
                        tor->notif_queue[pkt->flow_id]->req_type = pkt->memType;
                        tor->notif_queue[pkt->flow_id]->length = pkt->req_len;
                        tor->notif_queue[pkt->flow_id]->isGranted = 0;
                        if (pkt->memType == 0x0a) // For RREQ Notification, RRESP's sender is DST_NODE
                        {
                            tor->notif_queue[pkt->flow_id]->sender = pkt->dst_node;
                            tor->notif_queue[pkt->flow_id]->receiver = pkt->src_node;
                            // Directly fwd RREQ
                            // printf("Flow %d ask for RREQ to %d, curr: %d\n", pkt->flow_id, pkt->src_node, curr_timeslot);
                            assert("TOR RREQ RECV OVERFLOW" && pkt_recv(tor->upstream_mem_buffer[tor_port], pkt) != -1);
                        }
                        else
                        {
                            // printf("Flow %d ask for WREQ to %d, curr: %d\n", pkt->flow_id, pkt->dst_node, curr_timeslot);
                            tor->notif_queue[pkt->flow_id]->sender = pkt->src_node;
                            tor->notif_queue[pkt->flow_id]->receiver = pkt->dst_node;
                        }
                    }
                    else // Could be 1a or 1b or 1c or 2b or 2c
                    {
                        assert("TOR RECV OVERFLOW" && pkt_recv(tor->upstream_mem_buffer[tor_port], pkt) != -1);
                        flow_t *flow = flowlist->flows[pkt->flow_id];
                        // if (!flow->released && flow->flow_size_bytes - pkt->seq_num < BLK_SIZE)
                        // {
                        //     flow->released = 1;
                        //     tor->downstream_mem_buffer_lock[pkt->dst_node] = 0;
                        // }
                        if (pkt->memType == 0x2b || pkt->memType == 0x2c)
                        {
                            tor->downstream_mem_buffer_lock[pkt->dst_node] = 0;
                        }
                    }
                }
                else
                {
                    int8_t drop = pkt_recv(tor->upstream_pkt_buffer[tor_port], pkt);
                    if (drop < 0)
                    {
                        printf("Upstream drop NET, num: %d\n", tor->upstream_pkt_buffer[tor_port]->num_elements);
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
            packet_t peek_pkt = NULL, pkt = NULL;
            // Sort the link to see if we have proper packet to dequeue;
            int link_num = links->tor_to_host_link[0][node_index]->fifo->num_elements;
            int64_t min_time = INT64_MAX - 1;
            int min_idx = -1, cnt_link = 0;
            for (int i = 0; i < link_num; i++)
            {
                peek_pkt = (packet_t)link_peek(links->tor_to_host_link[0][node_index], i);
                if (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link < min_time && peek_pkt->time_to_dequeue_from_link <= curr_timeslot)
                {
                    if (!peek_pkt->control_flag) // is data
                    {
                        cnt_link++;
                    }
                    min_time = peek_pkt->time_to_dequeue_from_link;
                    min_idx = i;
                }
            }
            assert("host to TOR link num!!!" && cnt_link < 3);

            if (min_idx >= 0)
            {
                pkt = link_get(links->tor_to_host_link[0][node_index], min_idx);
                assert(pkt->dst_node == node_index);
                // Data Packet
                if (pkt->control_flag == 0)
                {
                    // printf("host recv (%d, %02x), flow-memType: %d, cnt: %d, seq:%d, bytes recv: %d, curr: %d\n", pkt->isMemPkt, pkt->memType, flowlist->flows[pkt->flow_id]->memType, pkt->pkt_id, pkt->seq_num, flowlist->flows[pkt->flow_id]->bytes_received, curr_timeslot);
                    // Update flow
                    flow_t *flow = flowlist->flows[pkt->flow_id];
                    assert(flow != NULL);
                    // Check SEQ state, accept only if no BLOCK drop: If received bytes is less than seq, suggesting pkt loss, then reject the whole packet, i.e., return to last_ack
                    if (flow->bytes_received + pkt->size < pkt->seq_num)
                    {
                        // printf("mismatch flow: %d %d-%d recved: %d seq: %d cnt: %d, curr: %d\n", flow->flow_id, pkt->src_node, pkt->dst_node, flow->bytes_received, pkt->seq_num, pkt->pkt_id, curr_timeslot);
                        flow->bytes_received = nodes[flow->src]->last_acked[flow->flow_id];
                    }
                    else
                    {
                        if (pkt->isMemPkt)
                        {
                            // The Sender received Grant
                            if (pkt->memType == 200)
                            {
                                // Counteract the flow updates...
                                flow->pkts_received--;
                                flow->bytes_received -= pkt->size;
                                total_bytes_rcvd -= pkt->size;
                                total_pkts_rcvd--;

                                // Change timeslot = Re-activate it.
                                // flow->timeslot = curr_timeslot + 1;
                                if (flow->memType > 900) // if it is a WREQ and not yet granted
                                {
                                    // printf("WREQ granted! curr: %d\n", curr_timeslot);
                                    flow->memType = 2; // WREQ Granted
                                }
                                else if (flow->memType == 0) // Received a Grant of a RREQ, means I need to add a new RRESP flow
                                {
                                    flow_t *rresp_flow = create_flow(flowlist->num_flows, pkt->req_len, node->node_index /* DST send to Requester*/, flow->src, curr_timeslot + 1);
                                    rresp_flow->isMemFlow = 1;
                                    rresp_flow->memType = 1; // RRESP memType is 1
                                    rresp_flow->expected_runtime = pkt->req_len / BLK_SIZE + 2 * per_hop_propagation_delay_in_timeslots;
                                    add_flow(flowlist, rresp_flow);
                                    printf("Created RRESP flow %d, requester: %d responder %d, flow_size %dB ts %d, num of flows: %d\n", flowlist->num_flows - 1, flow->src, flow->dst, pkt->req_len, curr_timeslot + 1, flowlist->num_flows);
                                }
                            }
                            else // Normal Mem traffic
                            {
#ifdef RECORD_PACKETS
                                fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, mem(%02x), %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->memType, (int)pkt->seq_num);
#endif
                            }
                        }
                        else
                        {
                            // Reply ACK, and write to file as a pkt
                            if (pkt->seq_num - node->ack_num[pkt->flow_id] > 1500)
                            {
#ifdef RECORD_PACKETS
                                fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, net, %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->seq_num);
#endif
                                node->ack_num[pkt->flow_id] = pkt->seq_num + pkt->size;
                                packet_t ack = ack_packet(pkt, node->ack_num[pkt->flow_id]);
                                ack->isMemPkt = 0;
                                // Respond with ACK packet
                                ack->time_when_transmitted_from_src = curr_timeslot;
                                ack->time_to_dequeue_from_link = curr_timeslot + per_sw_delay_in_timeslots + per_hop_propagation_delay_in_timeslots;
                                link_enqueue(links->host_to_tor_link[node_index][0], ack);
                                // printf("ack cnt: %d, acknum: %d, deq: %d, curr: %d\n", ack->pkt_id, ack->ack_num, ack->time_to_dequeue_from_link, curr_timeslot);
                            }
                        }
                        flow->pkts_received++;
                        flow->bytes_received += pkt->size;
                        total_bytes_rcvd += pkt->size;
                        total_pkts_rcvd++;
                        // Determine if last packet of flow has been received
                        if (!flow->finished && flow->bytes_received >= flow->flow_size_bytes)
                        {
                            flow->active = 0;
                            flowlist->active_flows--;
                            flow->finished = 1;
                            flow->finish_timeslot = curr_timeslot;
                            num_of_flows_finished++;
                            write_to_outfile(out_fp, flow, timeslot_len, link_bandwidth);
                            // printf("%d flows finished\n", num_of_flows_finished);
                            printf("%d: Flow %d finished in %d timeslots, %d blocks received\n", (int)curr_timeslot, (int)flow->flow_id, (int)(flow->finish_timeslot - flow->timeslot), flow->pkts_received);
                            fflush(stdout);
                        }
                    }
                }
                // Control Packet, then this node is a sender node
                else
                {
#ifdef RECORD_PACKETS
                    fprintf(host_outfiles[i], "%d, %d, %d, %d, %d, netack, %d\n", (int)pkt->flow_id, (int)pkt->src_node, (int)pkt->dst_node, (int)(curr_timeslot), (int)pkt->time_when_transmitted_from_src, (int)pkt->seq_num);
#endif
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
                        node->last_ack_time[pkt->flow_id] -= TIMEOUT / 3; // fast retransmit
                    }
                }
            }
            free_packet(pkt);
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
        if (total_flows_started >= flowlist->num_flows)
        {
            printf("\n======== All %d flows started ========\n\n", (int)total_flows_started);
            terminate1 = 1;
        }
        if (terminate0 || terminate1 || terminate2 || terminate3 || terminate4 || terminate5)
        {
            int completed_flows = 0;
            float flow_completion_times[flowlist->num_flows];
            float slowdowns[flowlist->num_flows];
            float avg_slowdown = 0;
            printf("FCT: ");
            for (int i = 0; i < flowlist->num_flows; i++)
            {
                if (flowlist->flows[i]->finished > 0)
                {
                    printf("%d, ", flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot);
                    flow_completion_times[completed_flows] = flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot;
                    avg_flow_completion_time += flowlist->flows[i]->finish_timeslot - flowlist->flows[i]->timeslot;
                    slowdowns[completed_flows] = flow_completion_times[completed_flows] / (double)flowlist->flows[i]->expected_runtime;
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
/*
Example trace:
FlowID, isMemFlow, memType, src, dst, flow size bytes, rreq_bytes, initialize timeslot
0,      1,          0,      0,    1,    1024,             1024            0
1,      0,         -1,      0,    2,    1024,             -1              0
*/
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
        int memType = -1;
        int rreq_bytes = -1;
        int src = -1;
        int dst = -1;
        int flow_size_bytes = -1;
        double time = -1;
        // mem and net packets are in together. distinguish them when initializing flow
        while (fscanf(fp, "%d,%d,%d,%d,%d,%d,%d,%lf", &flow_id, &isMemFlow, &memType, &src, &dst, &flow_size_bytes, &rreq_bytes, &time) >= 8 && flow_id <= MAX_FLOW_ID / 2)
        {
            int timeslot = (int)(time * 1e9 / timeslot_len);
            initialize_flow(flow_id, isMemFlow, memType, src, dst, flow_size_bytes, rreq_bytes, timeslot);
        }

        printf("Flows initialized\n");
        fclose(fp);
    }
    return;
}

void initialize_flow(int flow_id, int isMemFlow, int memType, int src, int dst, int flow_size_bytes, int rreq_bytes, int timeslot)
{
    if (flow_id < MAX_FLOW_ID)
    {
        flow_t *new_flow = create_flow(flow_id, flow_size_bytes, src, dst, timeslot);
        new_flow->isMemFlow = isMemFlow;
        new_flow->memType = memType;
        new_flow->rreq_bytes = rreq_bytes;
        if (isMemFlow)
        {
            new_flow->expected_runtime = flow_size_bytes / BLK_SIZE + 2 * per_hop_propagation_delay_in_timeslots;
        }
        else
        {
            new_flow->expected_runtime = flow_size_bytes / BLK_SIZE + 2 * per_hop_propagation_delay_in_timeslots + 2 * per_sw_delay_in_timeslots;
        }

        add_flow(flowlist, new_flow);
        // #ifdef DEBUG_DRIVER
        printf("initialized flow %d (%d, %d), src %d dst %d, flow_size %dB (%d) ts %d\n", flow_id, isMemFlow, memType, src, dst, flow_size_bytes, flow_size_bytes / 8, timeslot);
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

    char pathname[520] = "out/";
    strncat(pathname, "QueueInfo_", 500);
    strncat(pathname, base_filename, 20);
    sw_queue_fp = fopen(pathname, "w");
    // Net: downstream_send_buffer (egress)
    for (int i = 0; i < NODES_PER_RACK; i++)
    {
        fprintf(sw_queue_fp, "Net-%d,", i);
    }

    for (int i = 0; i < NODES_PER_RACK; i++)
    {
        fprintf(sw_queue_fp, "Mem-%d,", i);
    }
    fprintf(sw_queue_fp, "\n");
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
        fprintf(host_outfiles[i], "flow_id, src, dst, arrival_time, creation_time, Type, SeqNum\n");
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
        case 'b':
            link_bandwidth = atof(optarg);
            printf("Running with a link bandwidth of: %fGbps\n", link_bandwidth);
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
