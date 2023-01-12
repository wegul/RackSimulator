#include "driver.h"

// Default values for simulation
static int pkt_size = 1500; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len = 120; //in ns
static int bytes_per_timeslot = 1500;
static int snapshot_epoch = 1; //timeslots between snapshots

int16_t header_overhead = 64;
float per_hop_propagation_delay_in_ns = 100;
int per_hop_propagation_delay_in_timeslots;
volatile int64_t curr_timeslot = 0; //extern var
int packet_counter = 0;
int num_datapoints = 100000;

int enable_sram = 1; // Value of 1 = Enable SRAM usage
int init_sram = 1; // Value of 1 = initialize SRAM
int fully_associative = 0; // Value of 1 = use fully-associative SRAM
int64_t sram_size = (int64_t) SRAM_SIZE;
int burst_size = 3; // Number of packets to send in a burst
double load = 1.0; // Network load 
int64_t cache_misses = 0;
int64_t cache_hits = 0;
int64_t total_bytes_rcvd = 0;

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;
static volatile int8_t terminate4 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 500000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 500000; //stop after these many flows start

volatile int64_t max_timeslots = 30000; // extern var

// Output files
FILE * out_fp = NULL;
FILE * timeseries_fp = NULL;
FILE * spine_outfiles[NUM_OF_SPINES];
FILE * tor_outfiles[NUM_OF_RACKS];

// Network
node_t * nodes;
tor_t * tors;
spine_t * spines;
links_t links;
flowlist_t * flowlist;

void work_per_timeslot()
{
    printf("Simulation started\n");
    int flow_idx = 0;
    while (1) {
/*---------------------------------------------------------------------------*/
                                  //Activate inactive flows
/*---------------------------------------------------------------------------*/
        for (; flow_idx < flowlist->num_flows; flow_idx++) {
            flow_t * flow = flowlist->flows[flow_idx];
            if (flow == NULL || flow->timeslot > curr_timeslot) {
                break;
            }
            else if (flow->timeslot == curr_timeslot) {
#ifdef DEBUG_DRIVER
                printf("%d: flow %d has been enabled\n", (int) curr_timeslot, (int) flow->flow_id);
#endif
                int src = flow->src;
                buffer_put(nodes[src]->active_flows, flow);
            }
        }

/*---------------------------------------------------------------------------*/
                                  //SPINE -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;

            //record spine port queue lengths
            for (int j = 0; j < SPINE_PORT_COUNT; ++j) {
                int32_t size = spine_buffer_bytes(spine, j);
                timeseries_add(spine->queue_stat[j], size);
            }

            //send the packet
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(spine->pkt_buffer[k], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot){
                    packet_t pkt = NULL;
                    if (enable_sram == 0) {
                        pkt = send_to_tor_dram_only(spine, k, &cache_misses);
                    }
                    else if (fully_associative > 0) {
                        pkt = send_to_tor(spine, k, &cache_misses, &cache_hits);
                    }
                    else {
                        pkt = send_to_tor_dm(spine, k, &cache_misses, &cache_hits);
                    }
                    if (pkt != NULL) {
                        pkt->time_to_dequeue_from_link = curr_timeslot +
                            per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->spine_to_tor_link[spine_index][k], pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: spine %d sent pkt to ToR %d\n", (int) curr_timeslot, i, k);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(spine->pkt_buffer[k], 0);
                    }
                    else {
                        break;
                    }
                }

                //send snapshots every snapshot_epoch
                if (curr_timeslot % snapshot_epoch == 0) {
                    snapshot_t * snapshot = snapshot_to_tor(spine, k);
                    if (snapshot != NULL) {
                        snapshot->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        ipg_send(links->spine_to_tor_link[spine_index][k], snapshot);
#ifdef DEBUG_SNAPSHOTS
                        printf("%d: spine %d sent snapshot to ToR %d\n", (int) curr_timeslot, i, k);
#endif
                    }
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //HOST -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;
            // Check if host has any active flows at current moment
            if ((node->active_flows)->num_elements > 0 || node->current_flow != NULL) {
                flow_t * flow = NULL;
                // Time to select a new burst flow
                if (curr_timeslot % burst_size == 0 || node->current_flow == NULL) {
                    // Return the current flow back to the active flows list
                    if (node->current_flow != NULL) {
#ifdef DEBUG_DRIVER
                        printf("%d: Returning flow %d after finishing burst\n", (int) curr_timeslot, (int) node->current_flow->flow_id);
#endif
                        buffer_put(node->active_flows, node->current_flow);
                        node->current_flow = NULL;
                    }
                    // Randomly select a new active flow to start
                    int32_t num_af = node->active_flows->num_elements;
                    int32_t selected = (int32_t) rand() % num_af;
                    flow = buffer_remove(node->active_flows, selected);
                    // If flow is not active yet, return it to flow list
                    if (flow == NULL || (flow->active == 0 && flow->timeslot > curr_timeslot)) {
                        buffer_put(node->active_flows, flow);
                        flow = NULL;
#ifdef DEBUG_DRIVER
                        printf("%d: Node %d attempted burst on unviable flow\n", (int) curr_timeslot, (int) node_index);
#endif
                    }

                    else {
                        node->current_flow = flow;
#ifdef DEBUG_DRIVER
                        printf("%d: Node %d has started new burst on flow %d\n", (int) curr_timeslot, (int) node_index, (int) flow->flow_id);
#endif
                    }
                }
                // Burst size has not been reached; keep sending from current flow
                else {
#ifdef DEBUG_DRIVER
                    printf("%d: Node %d contiuing to send burst from flow %d\n", (int) curr_timeslot, (int) node_index, (int) node->current_flow->flow_id);
#endif
                    flow = node->current_flow;
                }
                // Send packet if random generates number below load ratio
                if ((double) rand() / (double) RAND_MAX < load) {        
                    if (flow != NULL && (flow->active == 1 || flow->timeslot <= curr_timeslot)) {
                        int16_t src_node = flow->src;
                        int16_t dst_node = flow->dst;
                        int64_t flow_id = flow->flow_id;

                        // Send packets from this flow until cwnd is reached or the flow runs out of bytes to send
                        int64_t flow_bytes_remaining = flow->flow_size_bytes - flow->bytes_sent;
                        int64_t cwnd_bytes_remaining = node->cwnd[dst_node] * MTU - (node->seq_num[flow_id] - node->last_acked[flow_id]);
                        if (flow_bytes_remaining > 0 && cwnd_bytes_remaining > 0) {
                            // Determine how large the packet will be
                            int64_t size = 1500;
                            if (cwnd_bytes_remaining < 1500) {
                                size = cwnd_bytes_remaining;
                            }
                            if (flow_bytes_remaining < size) {
                                size = flow_bytes_remaining;
                            }
                            assert(size > 0);

                            // Create packet
                            packet_t pkt = create_packet(src_node, dst_node, flow_id, size, node->seq_num[flow_id], packet_counter);
                            packet_counter++;
                            node->seq_num[dst_node] += size;

                            // Send packet
                            pkt->time_when_transmitted_from_src = curr_timeslot;
                            int16_t dst_tor = node_index / NODES_PER_RACK;
                            pkt->time_to_dequeue_from_link = curr_timeslot +
                                per_hop_propagation_delay_in_timeslots;
                            link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
    #ifdef DEBUG_DRIVER
                            printf("%d: host %d created and sent pkt to ToR %d\n", (int) curr_timeslot, i, (int) dst_tor);
                            print_packet(pkt);
    #endif

                            // Update flow state
                            if (flow->active == 0) {
                                flowlist->active_flows++;
                                flow->start_timeslot = curr_timeslot;
                                total_flows_started++;
    #ifdef DEBUG_DRIVER 
                                printf("%d: flow %d started\n", (int) curr_timeslot, (int) flow_id);
    #endif
                            }

                            flow->active = 1;
                            flow->pkts_sent++;
                            flow->bytes_sent += size;
                            flow->timeslots_active++;
                            // Determine if last packet of flow has been sent
                            if (flow->pkts_sent >= flow->flow_size) {
                                flow->active = 0;
                                flowlist->active_flows--;
    #ifdef DEBUG_DRIVER
                                printf("%d: flow %d sending final packet\n", (int) curr_timeslot, (int) flow_id);
    #endif
                            }
                        }

                        // Set current flow back to null if there are no more bytes left to send from this flow
                        if (flow_bytes_remaining < 1) {
                            node->current_flow = NULL;
                        }
                        
                        // Return flow back to active flows if it is not complete
                        //if (flow_bytes_remaining > 0) {
                        //    buffer_put(node->active_flows, flow);
                        //    node->current_flow = NULL;
                        //}
                    }
                }
            }
        }

/*---------------------------------------------------------------------------*/
                          //ToR -- SEND TO HOST AND SPINE
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_TORS; ++i) {
            tor_t tor = tors[i];
            int16_t tor_index = tor->tor_index;

            //record upstream port queue lengths
            for (int j = 0; j < NUM_OF_SPINES; ++j) {
                int32_t size = tor_up_buffer_bytes(tor, j);
                timeseries_add(tor->upstream_queue_stat[j], size);
            }

            //record downstream port queue lengths
            for (int j = 0; j < NODES_PER_RACK; ++j) {
                int32_t size = tor_down_buffer_bytes(tor, j);
                timeseries_add(tor->downstream_queue_stat[j], size);
            }

            //send to each spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(tor->upstream_pkt_buffer[tor_port], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot) {
                    packet_t pkt = NULL;
                    if (enable_sram == 0) {
                        pkt = send_to_spine_dram_only(tor, tor_port, &cache_misses);
                    }
                    else if (fully_associative > 0) {
                        pkt = send_to_spine(tor, tor_port, &cache_misses, &cache_hits);
                    }
                    else {
                        pkt = send_to_spine_dm(tor, tor_port, &cache_misses, &cache_hits);
                    }
                    if (pkt != NULL) {
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots; 
                        link_enqueue(links->tor_to_spine_link[tor_index][tor_port], pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: ToR %d sent pkt to spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(tor->upstream_pkt_buffer[tor_port], 0);
                    }
                    else {
                        break;
                    }
                }
                //send snapshots every snapshot_epoch
                if (curr_timeslot % snapshot_epoch == 0) {
                    snapshot_t * snapshot = snapshot_to_spine(tor, tor_port);
                    if (snapshot != NULL) {
                        snapshot->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        ipg_send(links->spine_to_tor_link[tor_index][tor_port], snapshot);
#ifdef DEBUG_SNAPSHOTS
                    printf("%d: ToR %d sent snapshot to spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
                    }
                }
            }

            //send to each host
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port) {
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(tor->downstream_pkt_buffer[tor_port], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot) {
                    packet_t pkt = NULL;
                    if (enable_sram == 0) {
                        pkt = send_to_host_dram_only(tor, tor_port, &cache_misses);
                    }
                    else {
                        pkt = send_to_host(tor, tor_port, fully_associative, &cache_misses, &cache_hits);
                    }
                    if (pkt != NULL) {
                        int16_t dst_host = pkt->dst_node;
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->tor_to_host_link[tor_index][dst_host],pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: ToR %d sent pkt to host %d\n", (int) curr_timeslot, i, dst_host);
                        print_packet(pkt);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(tor->downstream_pkt_buffer[tor_port], 0);
                    }
                    else {
                        break;
                    }
                }
            }
        }

/*---------------------------------------------------------------------------*/
                              //ToR -- RECV FROM HOST AND SPINE
/*---------------------------------------------------------------------------*/
        
        for (int i = 0; i < NUM_OF_TORS; ++i) {
            tor_t tor = tors[i];
            int16_t tor_index = tor->tor_index;

            //Recv packet from host
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port) {
                //deq packet from the link
                int16_t src_host = (tor_index * TOR_PORT_COUNT_LOW) + tor_port;
                int bytes_rcvd = 0;
                packet_t peek_pkt = (packet_t) link_peek(links->host_to_tor_link[src_host][tor_index]);
                while (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link == curr_timeslot && bytes_rcvd + peek_pkt->size <= bytes_per_timeslot) {
                    int spine_id = hash(tor->routing_table, peek_pkt->flow_id);
                    packet_t pkt = (packet_t)
                        link_dequeue(links->host_to_tor_link[src_host][tor_index]);
                    if (pkt != NULL) {
                        buffer_put(tor->upstream_pkt_buffer[spine_id], pkt);
                        // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                        if (tor->upstream_pkt_buffer[spine_id]->num_elements > ECN_CUTOFF_TOR_UP) {
                            pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                            printf("%d: ToR %d marked upstream pkt with ECN\n", (int) curr_timeslot, i);
#endif
                        }

#ifdef DEBUG_DRIVER
                        printf("%d: Tor %d recv pkt from host %d\n", (int) curr_timeslot, i, src_host);
#endif
#ifdef RECORD_PACKETS
                        fprintf(tor_outfiles[i], "%d, %d, %d, %d, %d, up\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) tor_port, (int) (curr_timeslot * timeslot_len));
#endif
                        bytes_rcvd += pkt->size;
                    }
                    peek_pkt = (packet_t) link_peek(links->host_to_tor_link[src_host][tor_index]);
                }
            }

            //Recv packet from spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                //deq packet from the link
                int16_t src_spine = tor_port;
                int bytes_rcvd = 0;
                packet_t peek_pkt = (packet_t) link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                while (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link == curr_timeslot && bytes_rcvd + peek_pkt->size <= bytes_per_timeslot) {
                    packet_t pkt = (packet_t)
                        link_dequeue(links->spine_to_tor_link[src_spine][tor_index]);
                    if (pkt != NULL) {
                        int node_index = pkt->dst_node % NODES_PER_RACK;
                        buffer_put(tor->downstream_pkt_buffer[node_index], pkt);
                        // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                        if (tor->downstream_pkt_buffer[node_index]->num_elements > ECN_CUTOFF_TOR_DOWN) {
                            pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                            printf("%d: ToR %d marked downstream pkt with ECN\n", (int) curr_timeslot, i);
#endif
                        }
#ifdef DEBUG_DRIVER
                        printf("%d: Tor %d recv pkt from spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
#ifdef RECORD_PACKETS
                        fprintf(tor_outfiles[i], "%d, %d, %d, %d, %d, down\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) tor_port, (int) (curr_timeslot * timeslot_len));
#endif
                        bytes_rcvd += pkt->size;
                    }
                    peek_pkt = (packet_t) link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                }
                // Receive snapshot
                snapshot_t * snapshot = (snapshot_t * ) ipg_peek(links->spine_to_tor_link[src_spine][tor_index]);
                if (snapshot != NULL && snapshot->time_to_dequeue_from_link == curr_timeslot) {
                    snapshot = ipg_recv(links->spine_to_tor_link[src_spine][tor_index]);
                    free(snapshot);
#ifdef DEBUG_SNAPSHOTS
                    printf("%d: ToR %d recv snapshot from spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //SPINE -- RECV
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;


            for (int spine_port = 0; spine_port < SPINE_PORT_COUNT; ++spine_port) {
                //deq packet
                int16_t src_tor = spine_port;
                int bytes_rcvd = 0;
                packet_t peek_pkt = (packet_t) link_peek(links->tor_to_spine_link[src_tor][spine_index]);
                while (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link == curr_timeslot && bytes_rcvd + peek_pkt->size <= bytes_per_timeslot) {  
                    packet_t pkt = (packet_t) link_dequeue
                        (links->tor_to_spine_link[src_tor][spine_index]);
#ifdef DEBUG_DRIVER
                    printf("%d: Spine %d recv pkt from ToR %d\n", (int) curr_timeslot, spine_index, src_tor);
#endif
#ifdef RECORD_PACKETS
                    fprintf(spine_outfiles[i], "%d, %d, %d, %d, %d\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) spine_port, (int) (curr_timeslot * timeslot_len));
#endif
                    //enq packet in the virtual queue
                    int16_t dst_host = pkt->dst_node;
                    int16_t dst_tor = dst_host / NODES_PER_RACK;
                    if (dst_host != -1) {
                        pkt->time_when_added_to_spine_queue = curr_timeslot;
                        assert(buffer_put(spine->pkt_buffer[dst_tor], pkt) != -1);
                        // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                        if (spine->pkt_buffer[dst_tor]->num_elements > ECN_CUTOFF_SPINE) {
                            pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                            printf("%d: Spine %d marked pkt with ECN\n", (int) curr_timeslot, spine_index);
#endif
                        }

                        if (dst_host == -1) {
                            free_packet(pkt);
                        }
                    }
                    bytes_rcvd += pkt->size;
                    peek_pkt = (packet_t) link_peek(links->tor_to_spine_link[src_tor][spine_index]);
                }
                // Receive snapshot
                snapshot_t * snapshot = (snapshot_t * ) ipg_peek(links->tor_to_spine_link[src_tor][spine_index]);
                if (snapshot != NULL && snapshot->time_to_dequeue_from_link == curr_timeslot) {
                    snapshot = ipg_recv(links->spine_to_tor_link[src_tor][spine_index]);
                    free(snapshot);
#ifdef DEBUG_SNAPSHOTS
                    printf("%d: Spine %d recv snapshot from ToR %d\n", (int) curr_timeslot, spine_index, src_tor);
#endif
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //HOST -- RECV
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;

            //deq packet
            int16_t src_tor = node_index / NODES_PER_RACK;
            int bytes_rcvd = 0;
            packet_t peek_pkt = (packet_t) link_peek(links->tor_to_host_link[src_tor][node_index]);
            while (peek_pkt != NULL && peek_pkt->time_to_dequeue_from_link == curr_timeslot && bytes_rcvd + peek_pkt->size <= bytes_per_timeslot) {
                assert(peek_pkt->dst_node == node_index);
                packet_t pkt = (packet_t) link_dequeue(links->tor_to_host_link[src_tor][node_index]);
                // Data Packet
                if (pkt->control_flag == 0) {
#ifdef DEBUG_DRIVER
                    printf("%d: host %d received data packet from ToR %d\n", (int) curr_timeslot, node_index, src_tor);
#endif
                    // Create ACK packet
                    node->ack_num[pkt->flow_id] = pkt->seq_num + pkt->size;
                    packet_t ack = ack_packet(pkt, node->ack_num[pkt->flow_id]);
                    // Respond with ACK packet
                    ack->time_when_transmitted_from_src = curr_timeslot;
                    int16_t dst_tor = node_index / NODES_PER_RACK;
                    ack->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->host_to_tor_link[node_index][dst_tor], ack);
#ifdef DEBUG_DCTCP
                    printf("%d: host %d created and sent ACK %d to dst %d\n", (int) curr_timeslot, i, (int) ack->ack_num, (int) pkt->src_node);
#endif

                    // Update flow
                    flow_t * flow = flowlist->flows[pkt->flow_id];
                    assert(flow != NULL);
                    flow->pkts_received++;
                    flow->bytes_received += pkt->size;
                    total_bytes_rcvd += pkt->size;

                    if (flow->pkts_received == flow->flow_size) {
                        assert(flow->bytes_sent == flow->bytes_received);
                        flow->active = 0;
                        flow->finished = 1;
                        flow->finish_timeslot = curr_timeslot;
                        num_of_flows_finished++;
                        write_to_outfile(out_fp, flow, timeslot_len, link_bandwidth);
#ifdef DEBUG_DRIVER
                        printf("%d: Flow %d finished\n", (int) curr_timeslot, (int) flow->flow_id);
#endif
                    }
                }
                // Control Packet
                else {
#ifdef DEBUG_DCTCP
                    printf("%d: host %d received control packet from dst %d\n", (int) curr_timeslot, node_index, pkt->src_node);
#endif
                    // Check ECN flag
                    track_ecn(node, pkt->flow_id, pkt->ecn_flag);

                    // Check ACK value
                    if (pkt->ack_num > node->last_acked[pkt->flow_id]){
                        node->last_acked[pkt->flow_id] = pkt->ack_num;
                    }
                }
                bytes_rcvd += pkt->size;
                peek_pkt = (packet_t) link_peek(links->tor_to_host_link[src_tor][node_index]);
                free_packet(pkt);
            }
        }

/*---------------------------------------------------------------------------*/
                 //Data logging
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
                 //State updates before next iteration
/*---------------------------------------------------------------------------*/

        if (flowlist->active_flows < 1) {
            int no_flows_left = 1;
            for (int i = 0; i < flowlist->num_flows; i++) {
                flow_t * flow = flowlist->flows[i];
                if (flow != NULL && flow->finished == 0) {
                    no_flows_left = 0;
                }
            }
            if (no_flows_left > 0) {
                printf("Finished all flows\n\n");
                terminate0 = 1;
            }
        }

        if (total_flows_started >= num_of_flows_to_start) {
            printf("\nStarted %d flows\n\n", (int) total_flows_started);
            terminate1 = 1;
        }

        if (num_of_flows_finished >= num_of_flows_to_finish) {
            printf("\nFinished %d flows\n\n", (int) num_of_flows_finished);
            terminate2 = 1;
        }

        if (curr_timeslot >= max_timeslots) {
            printf("\nReached max timeslot %d\n\n", (int) curr_timeslot);
            terminate3 = 1;
        }

        if (terminate0 || terminate1 || terminate2 || terminate3) {
            double curr_time = curr_timeslot * timeslot_len / 1e9;
            printf("Finished in %d timeslots\n", (int) curr_timeslot);
            printf("Finished in %f seconds\n", curr_time);
            break;
        }

        if (curr_timeslot % 100 == 0) {
            printf(".");
        }
        if (curr_timeslot % 10000 == 9999) {
            printf("\n");
        }

        curr_timeslot++;
    }
    printf("\nSimulation Ended\n");
}

static inline void usage()
{
    printf("usage: ./<exec> -f <filename>\n");
    printf("filename must be <500 char long\n");
    exit(1);
}


void process_args(int argc, char ** argv) {
    int opt;
    char filename[500] = "";
    char out_filename[504] = "";
    char out_suffix[4] = ".out";
    char timeseries_filename[515] = "";
    char timeseries_suffix[15] = ".timeseries.csv";
    int sram_set = 0;

    while ((opt = getopt(argc, argv, "f:b:c:h:d:n:m:t:q:a:i:e:s:u:l:")) != -1) {
        switch(opt) {
            case 'c': 
                pkt_size = atoi(optarg);
                printf("Running with a pkt size of: %dB\n", pkt_size);
                break;
            case 'b': 
                link_bandwidth = atof(optarg);
                printf("Running with a link bandwidth of: %fGbps\n", link_bandwidth);
                break;
            case 'h': 
                header_overhead = atoi(optarg);
                printf("Header overhead of: %dB\n", header_overhead);
                break;
            case 'd': 
                per_hop_propagation_delay_in_ns = atof(optarg);
                break;
            case 'n': 
                num_of_flows_to_finish = atoi(optarg);
                printf("Stop experiment after %ld flows have finished\n", num_of_flows_to_finish);
                break;
            case 'm': 
                num_of_flows_to_start = atoi(optarg);
                printf("Stop experiment after %ld flows have started\n", num_of_flows_to_start);
                break;
            case 't':
                max_timeslots = atoi(optarg);
                printf("Stop experiment after %ld timeslots have finished\n", max_timeslots);
                break;
            case 'q':
                num_datapoints = atoi(optarg);
                printf("Writing %d queue length datapoints to timeseries.csv file\n", num_datapoints);
                break;
            case 'a':
                fully_associative = atoi(optarg);
                if (fully_associative == 0) {
                    printf("Using direct-mapped SRAM of size %d\n", sram_size);
                }
                else {
                    printf("Using fully-associative SRAM of size %d\n", sram_size);
                }
                break;
            case 'i':
                init_sram = atoi(optarg);
                if (init_sram == 1) {
                    printf("Initializing SRAM\n");
                    for (int i = 0; i < NUM_OF_SPINES; i++) {
                        initialize_sram(spines[i]->sram);
                        initialize_dm_sram(spines[i]->dm_sram);
                    }
                    for (int i = 0; i < NUM_OF_RACKS; i++) {
                        initialize_sram(tors[i]->sram);
                        initialize_dm_sram(tors[i]->dm_sram);
                    }
                }
                break;
            case 'e':
                enable_sram = atoi(optarg);
                if (enable_sram == 0) {
                    printf("SRAM is disabled\n");
                }
                else {
                    printf("SRAM is enabled\n");
                }
                break;
            case 's':
                sram_set = 1;
                sram_size = atoi(optarg);
                printf("Utilizing SRAM size of %d\n", sram_size);
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
                if (strlen(optarg) < 500) {
                    strcpy(filename, optarg);
                    strncpy(out_filename, filename, strlen(filename));
                    strncat(out_filename, out_suffix, 4);
                    strncpy(timeseries_filename, filename, strlen(filename));
                    strncat(timeseries_filename, timeseries_suffix, 15);
#ifdef RECORD_PACKETS
                    printf("Writing switch packet data to switch.csv files\n");
                    open_switch_outfiles(filename);
#endif
                }
                else
                    usage();
                break;
            default:
                printf("Wrong command line argument\n");
                exit(1);
        }
    }

    timeslot_len = (pkt_size * 8) / link_bandwidth;
    printf("Running with a slot length of %fns\n", timeslot_len);

    per_hop_propagation_delay_in_timeslots = round((float)per_hop_propagation_delay_in_ns / (float)timeslot_len);
    printf("Per hop propagation delay: %f ns (%d timeslots)\n",
        per_hop_propagation_delay_in_ns, per_hop_propagation_delay_in_timeslots);
    
    bytes_per_timeslot = (int) (timeslot_len * link_bandwidth / 8);
    printf("Bytes sent per timeslot %d\n", bytes_per_timeslot);
    printf("\n");

    DIR * dir = opendir("out/");
    if (dir) {
        closedir(dir);
    }
    else if (ENOENT == errno) {
        mkdir("out/", 0777);
    }
    else {
        printf("Could not open out directory.");
        exit(1);
    }

    read_tracefile(filename);
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        spines[i]->sram->capacity = sram_size;
        spines[i]->dm_sram->capacity = sram_size;
    }
    for (int i = 0; i < NUM_OF_RACKS; i++) {
        tors[i]->sram->capacity = sram_size;
        tors[i]->dm_sram->capacity = sram_size;
    }
    out_fp = open_outfile(out_filename);
#ifdef WRITE_QUEUELENS
    timeseries_fp = open_timeseries_outfile(timeseries_filename);
#endif
}

void read_tracefile(char * filename) {
    FILE * fp;
    flowlist = create_flowlist();
    if (strcmp(filename, "")) {
        printf("Opening tracefile %s\n", filename);
        fp = fopen(filename, "r");
        if (fp == NULL) {
            perror("open");
            exit(1);
        }
        int flow_id = -1;
        int src = -1;
        int dst = -1;
        int flow_size_bytes = -1;
        int flow_size_pkts = -1;
        int timeslot = -1;
        
        while (fscanf(fp, "%d,%d,%d,%d,%d,%d", &flow_id, &src, &dst, &flow_size_bytes, &flow_size_pkts, &timeslot) == 6 && flow_id < MAX_FLOW_ID) {
            initialize_flow(flow_id, src, dst, flow_size_pkts, flow_size_bytes, timeslot);
            // Final timeslot is start time of last flow
            max_timeslots = timeslot;
        }
        
        printf("Flows initialized\n");
        fclose(fp);
    }
    return;
}

void initialize_flow(int flow_id, int src, int dst, int flow_size_pkts, int flow_size_bytes, int timeslot) {
    if (flow_id < MAX_FLOW_ID) {
        flow_t * new_flow = create_flow(flow_id, flow_size_pkts, flow_size_bytes, src, dst, timeslot);
        add_flow(flowlist, new_flow);
#ifdef DEBUG_DRIVER
        printf("initialized flow %d src %d dst %d flow_size %d bytes %d ts %d\n", flow_id, src, dst, flow_size_pkts, flow_size_bytes, timeslot);
#endif
    }
}

void free_flows() {
    free_flowlist(flowlist);
    printf("Freed flows\n");
}

void initialize_network() {
    //create nodes
    nodes = (node_t*) malloc(NUM_OF_NODES * sizeof(node_t));
    MALLOC_TEST(nodes, __LINE__);
    for (int i = 0; i < NUM_OF_NODES; ++i) {
        nodes[i] = create_node(i);
    }
    printf("Nodes initialized\n");

    //create ToRs
    tors = (tor_t*) malloc(NUM_OF_TORS * sizeof(tor_t));
    MALLOC_TEST(tors, __LINE__);
    for (int i = 0; i < NUM_OF_TORS; ++i) {
        tors[i] = create_tor(i, sram_size, init_sram);
    }
    printf("ToRs initialized\n");

    //create spines
    spines = (spine_t*) malloc(NUM_OF_SPINES * sizeof(spine_t));
    MALLOC_TEST(spines, __LINE__);
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        spines[i] = create_spine(i, sram_size, init_sram);
    }
    printf("Spines initialized\n");

    //create links
    links = create_links();
    printf("Links initialized\n");
}

void free_network() {
    //free nodes
    for (int i = 0; i < NUM_OF_NODES; ++i) {
        free_node(nodes[i]);
    }
    free(nodes);

    //free ToRs
    for (int i = 0; i < NUM_OF_TORS; ++i) {
        free_tor(tors[i]);
    }
    free(tors);

    //free spines
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        free_spine(spines[i]);
    }
    free(spines);

    //free links
    free_links(links);

    printf("Freed network\n");
}

void open_switch_outfiles(char * base_filename) {
    char csv_suffix[4] = ".csv";
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        char filename[520] = "out/";
        char spine_suffix[6] = ".spine";
        char spine_id[5];
        sprintf(spine_id, "%d", i);
        strncat(filename, base_filename, 500);
        strncat(filename, spine_suffix, 6);
        strncat(filename, spine_id, 5);
        strncat(filename, csv_suffix, 4);
        spine_outfiles[i] = fopen(filename, "w");
        fprintf(spine_outfiles[i], "flow_id, src, dst, port, arrival_time\n");
    }

    for (int i = 0; i < NUM_OF_RACKS; i++) {
        char filename[520] = "out/";
        char tor_suffix[4] = ".tor";
        char tor_id[5];
        sprintf(tor_id, "%d", i);
        strncat(filename, base_filename, 500);
        strncat(filename, tor_suffix, 4);
        strncat(filename, tor_id, 5);
        strncat(filename, csv_suffix, 4);
        tor_outfiles[i] = fopen(filename, "w");
        fprintf(tor_outfiles[i], "flow_id, src, dst, port, arrival_time, direction\n");
    }
}

void close_outfiles() {
    fclose(out_fp);
#ifdef WRITE_QUEUELENS
    fclose(timeseries_fp);
#endif
#ifdef RECORD_PACKETS
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        fclose(spine_outfiles[i]);
    }
    for (int i = 0; i < NUM_OF_RACKS; i++) {
        fclose(tor_outfiles[i]);
    }
#endif
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    initialize_network();
    process_args(argc, argv);
    
    work_per_timeslot();
    print_system_stats(spines, tors, total_bytes_rcvd, (int64_t) (curr_timeslot * timeslot_len), cache_misses, cache_hits);
#ifdef WRITE_QUEUELENS
    write_to_timeseries_outfile(timeseries_fp, spines, tors, curr_timeslot, num_datapoints);
#endif

    free_flows();
    free_network();
    close_outfiles();

    printf("Finished execution\n");

    return 0;
}
