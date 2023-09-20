#include "driver.h"

// Default values for simulation
static int pkt_size = MTU; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len = 120; //in ns
static int bytes_per_timeslot = 1500;
#ifdef ENABLE_SNAPSHOTS
static int snapshot_epoch = 1; //timeslots between snapshots
static int belady_epoch = 1;
static int belady_accesses = 1;
#endif

int16_t header_overhead = 64;
float per_hop_propagation_delay_in_ns = 100;
int per_hop_propagation_delay_in_timeslots;
int dram_access_time = DRAM_DELAY;
int timeslots_per_dram_access = 1;
int accesses_per_timeslot = 1;
volatile int64_t curr_timeslot = 0; //extern var
int packet_counter = 0;
int num_datapoints = 100000;

int enable_sram = 1; // Value of 1 = Enable SRAM usage
int init_sram = 0; // Value of 1 = initialize SRAMs
int program_tors = 1; // Value of 1 = ToRs are programmable
int sram_type = 1; // 0: Direct-mapped SRAM, 1: LRU SRAM, 2: LFU SRAM, 3: ARC SRAM, 4: S3-FIFO SRAM, 5: SIEVE SRAM
int64_t sram_size = (int64_t) SRAM_SIZE;
int burst_size = 1; // Number of packets to send in a burst
int packet_mode = 0; // 0: Full network bandwidth mode, 1: incast mode, 2: burst mode
int incast_active = 0; // Number of flows actively sending incast packets. Remaining nodes are reserved for switching
int incast_switch = 1; // Number of flows that switch flows for each incast period
int incast_size = 1; // Number of timeslots to send incast packets
double load = 1.0; // Network load 
int64_t cache_misses = 0;
int64_t cache_hits = 0;
int64_t total_bytes_rcvd = 0;
int64_t total_pkts_rcvd = 0;
float avg_delay_at_spine = 0;
float avg_max_delay_at_spine = 0;
int max_delay_in_timeperiod = 0;
int64_t pkt_delays_recorded = 0;
float avg_flow_completion_time = 0;

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;
static volatile int8_t terminate4 = 0;
static volatile int8_t terminate5 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 1000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 1000; //stop after these many flows start

volatile int64_t max_timeslots = 20000; // extern var
volatile int64_t max_cache_accesses = 200000;
volatile int64_t max_bytes_rcvd = 100000000;

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
    int incast_buffer_len = 0;
    int incast_sent_on_period = 0;
    int new_period = 1;
    int incast_active_senders[incast_active];
    int incast_inactive_senders[NUM_OF_NODES - incast_active - 1];
    for (int i = 0; i < incast_active; i++) { 
        incast_active_senders[i] = i;
    }
    for (int i = 0; i < NUM_OF_NODES - incast_active - 1; i++) {
        incast_inactive_senders[i] = i + incast_active;
    }
    while (1) {
/*---------------------------------------------------------------------------*/
                                  //Update memory
/*---------------------------------------------------------------------------*/

        // Update DRAM access counters
        // Spines
        for (int i = 0; i < NUM_OF_SPINES; i++) {
            spine_t spine = spines[i];

            if (spine->dram->lock > 0) {
                spine->dram->lock++;

                if (spine->dram->lock > spine->dram->delay) {
                    if (sram_type == 1) {
                        pull_from_dram(spine->sram, spine->dram, spine->dram->accessing);
                    }
                    else if (sram_type == 2) {
                        pull_from_dram_lfu(spine->lfu_sram, spine->dram, spine->dram->accessing);
                    }
                    else if (sram_type == 3) {
                        pull_from_dram_arc(spine->arc_sram, spine->dram, spine->dram->accessing);
                    }
                    else if (sram_type == 4) {
                        pull_from_dram_s3f(spine->s3f_sram, spine->dram, spine->dram->accessing);
                    }
                    else if (sram_type == 5) {
                        pull_from_dram_sve(spine->sve_sram, spine->dram, spine->dram->accessing);
                    }
                   
                    // printf("%d: Spine %d pulled id %d to index %d\n", (int) curr_timeslot, i, spine->dram->accessing, spine->dram->placement_idx);

                    // Reset DRAM lock
                    spine->dram->lock = 0;
                    spine->dram->accessing = -1;
                    spine->dram->placement_idx = 0;
                }
            }
        }

        // ToRs
        for (int i = 0; i < NUM_OF_RACKS; i++) {
            tor_t tor = tors[i];

            if (tor->dram->lock > 0) {
                tor->dram->lock++;

                if (tor->dram->lock > tor->dram->delay) {
                    if (sram_type == 1) {
                        pull_from_dram(tor->sram, tor->dram, tor->dram->accessing);
                    }
                    else if (sram_type == 2) { 
                        pull_from_dram_lfu(tor->lfu_sram, tor->dram, tor->dram->accessing);
                    }
                    else if (sram_type == 3) {
                        pull_from_dram_arc(tor->arc_sram, tor->dram, tor->dram->accessing);
                    }
                    else if (sram_type == 4) {
                        pull_from_dram_s3f(tor->s3f_sram, tor->dram, tor->dram->accessing);
                    }
                    else if (sram_type == 5) {
                        pull_from_dram_sve(tor->sve_sram, tor->dram, tor->dram->accessing);
                    }

                    // printf("%d: ToR %d pulled id %d to index %d\n", (int) curr_timeslot, i, tor->dram->accessing, tor->dram->placement_idx);

                    // Reset DRAM lock
                    tor->dram->lock = 0;
                    tor->dram->accessing = -1;
                    tor->dram->placement_idx = 0;
                }
            }
        }

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
                                  //SPINE -- PROCESS
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; i++) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;

            // Process packets for each port
            for (int j = 0; j < SPINE_PORT_COUNT; j++) {
                int64_t misses_before = cache_misses;
                process_packets(spine, j, &cache_misses, &cache_hits, sram_type);
            }
        }
/*---------------------------------------------------------------------------*/
                                  //ToR -- PROCESS
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_RACKS; i++) {
            tor_t tor = tors[i];
            int16_t tor_index = tor->tor_index;

            // Process packets on upstream pipelines
            for (int j = 0; j < NODES_PER_RACK; j++) {
                int64_t misses_before = cache_misses;
                //process_packets_up(tor, j, &cache_misses, &cache_hits, sram_type);
                move_to_up_send_buffer(tor, j);
            }

            // Process packets on downstream pipelines
            for (int j = 0; j < NUM_OF_SPINES; j++) {
                int64_t misses_before = cache_misses;
                process_packets_down(tor, j, &cache_misses, &cache_hits, sram_type);
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

            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                //send snapshots every snapshot_epoch
#ifdef ENABLE_SNAPSHOTS
                if (curr_timeslot % snapshot_epoch == 0) {
                    snapshot_t * snapshot = snapshot_to_tor(spine, k);
                    //snapshot_t * snapshot = snapshot_array[k];
                    if (snapshot != NULL) {
                        if (snapshot->flow_id[0] != -1) {
                            snapshot->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                            ipg_send(links->spine_to_tor_link[spine_index][k], snapshot);
#ifdef DEBUG_SNAPSHOTS
                            printf("%d: spine %d sent snapshot to ToR %d\n", (int) curr_timeslot, i, k);
#endif
                        }
                        else {
                            free(snapshot);
                        }
                    }
                }
#endif

                //send the packet
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(spine->send_buffer[k], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot){
                    packet_t pkt = NULL;
                    pkt = (packet_t) buffer_get(spine->send_buffer[k]);
                    if (pkt != NULL) {
                        pkt->time_left_spine = curr_timeslot;
                        pkt->time_spent_at_spine = pkt->time_left_spine - pkt->time_arrived_at_spine;
                        //printf("delay: %0.3f\nrecorded: %d\n", avg_max_delay_at_spine, pkt_delays_recorded);
                        avg_delay_at_spine = (avg_delay_at_spine * pkt_delays_recorded + pkt->time_spent_at_spine) / (pkt_delays_recorded + 1);
                        pkt_delays_recorded++;
                        if (pkt->time_spent_at_spine > max_delay_in_timeperiod) {
                            max_delay_in_timeperiod = pkt->time_spent_at_spine;
                        }
                        pkt->time_to_dequeue_from_link = curr_timeslot +
                            per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->spine_to_tor_link[spine_index][k], pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: spine %d sent pkt to ToR %d\n", (int) curr_timeslot, i, k);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(spine->send_buffer[k], 0);
                    }
                    else {
                        break;
                    }
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //HOST -- SEND
/*---------------------------------------------------------------------------*/

        if (packet_mode == 1 && incast_sent_on_period <= incast_size) {
            incast_sent_on_period++;
        }

        if (packet_mode == 1 && new_period == 1) {
            shuffle(incast_active_senders, incast_active);
            shuffle(incast_inactive_senders, NUM_OF_NODES - incast_active - 1);
            
            for (int i = 0; i < incast_switch; i++) {
                int temp = incast_active_senders[i];
                incast_active_senders[i] = incast_inactive_senders[i];
                incast_inactive_senders[i] = temp;
#ifdef DEBUG_DRIVER
                printf("%d: Activating flow %d, Deactivating node %d\n", (int) curr_timeslot, incast_active_senders[i], incast_inactive_senders[i]);
#endif
            }
        }

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;
            // Full Network Bandwidth mode
            if (packet_mode == 0) {
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
                        // Ensure initial flows exist in memory already so as not to flood cache misses start of simlation
                        if (node->current_flow != NULL && curr_timeslot == 0) {
                            for (int k = 0; k < NUM_OF_SPINES; k++) {
                                spine_t spine = spines[k];
                                if (sram_type == 1) {
                                    pull_from_dram(spine->sram, spine->dram, flow->flow_id);
                                }
                                else if (sram_type == 2) {
                                    pull_from_dram_lfu(spine->lfu_sram, spine->dram, flow->flow_id);
                                }
                                else if (sram_type == 3) {
                                    pull_from_dram_arc(spine->arc_sram, spine->dram, flow->flow_id);
                                }
                                else if (sram_type == 4) {
                                    pull_from_dram_s3f(spine->s3f_sram, spine->dram, flow->flow_id);
                                }
                                else if (sram_type == 5) {
                                    pull_from_dram_sve(spine->sve_sram, spine->dram, flow->flow_id);
                                }
                                else {
                                    dm_pull_from_dram(spine->dm_sram, spine->dram, flow->flow_id);
                                }
                            }
                            for (int k = 0; k < NUM_OF_RACKS; k++) {
                                tor_t tor = tors[k];
                                if (sram_type == 1) {
                                    pull_from_dram(tor->sram, tor->dram, flow->flow_id);
                                }
                                else if (sram_type == 2) {
                                    pull_from_dram_lfu(tor->lfu_sram, tor->dram, flow->flow_id);
                                }
                                else if (sram_type == 3) {
                                    pull_from_dram_arc(tor->arc_sram, tor->dram, flow->flow_id);
                                }
                                else if (sram_type == 4) {
                                    pull_from_dram_s3f(tor->s3f_sram, tor->dram, flow->flow_id);
                                }
                                else if (sram_type == 5) {
                                    pull_from_dram_sve(tor->sve_sram, tor->dram, flow->flow_id);
                                }
                                else {
                                    dm_pull_from_dram(tor->dm_sram, tor->dram, flow->flow_id);
                                }
                            }
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
                            node->cwnd[dst_node] = incast_size * 100;
                            int64_t flow_bytes_remaining = flow->flow_size_bytes - flow->bytes_sent;
                            int64_t cwnd_bytes_remaining = node->cwnd[dst_node] * MTU - (node->seq_num[flow_id] - node->last_acked[flow_id]);
                            if (flow_bytes_remaining > 0 && cwnd_bytes_remaining > 0) {
                                // Determine how large the packet will be
                                int64_t size = MTU;
                                if (cwnd_bytes_remaining < MTU) {
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
            // Incast Packets mode
            else if (packet_mode == 1 && i != NUM_OF_NODES - 1) {
                flow_t * flow = NULL;
                // Assign new flows
                if (new_period == 1) {
                    // Determine if flow was active last timeslot
                    int was_active = 0;
                    if (node->current_flow != NULL) {
                        was_active = 1;
                    }
                    // Determine if flow will be activated this timeslot
                    int is_active = 1;
                    for (int j = 0; j < NUM_OF_NODES - incast_active - 1; j++) {
                        if (incast_inactive_senders[j] == i) {
                            is_active = 0;
                            break;
                        }
                    }

                    // If node will begin sending on this timeslot, activate flow
                    if (was_active == 0 && is_active == 1) {
#ifdef DEBUG_DRIVER
                        printf("%d: Node %d has begun sending\n", (int) curr_timeslot, i);
#endif
                        flow = buffer_remove(node->active_flows, 0);
                        // Return flow if not yet active
                        if (flow == NULL || (flow->active == 0 && flow->timeslot > curr_timeslot)) {
                            buffer_put(node->active_flows, flow);
                            flow = NULL;
#ifdef DEBUG_DRIVER
                            printf("%d: Node %d attempted burst on unviable flow\n", (int) curr_timeslot, (int) node_index);
#endif            
                        }
                        node->current_flow = flow;
                    }

                    // If node will cease sending on this timeslot, activate flow
                    if (was_active == 1 && is_active == 0) {
#ifdef DEBUG_DRIVER
                        printf("%d: Node %d has stopped sending\n", (int) curr_timeslot, i);
#endif
                        buffer_put(node->active_flows, node->current_flow);
                        node->current_flow = NULL;
                        flow = NULL;
                    }

                    // Ensure initial flows exist in memory already so as not to flood cache misses start of simulation
                    // if (node->current_flow != NULL && curr_timeslot == 0) {
                    //     for (int k = 0; k < NUM_OF_SPINES; k++) {
                    //         spine_t spine = spines[k];
                    //         if (sram_type == 1) {
                    //             pull_from_dram(spine->sram, spine->dram, flow->flow_id);
                    //         }
                    //         else if (sram_type == 2) {
                    //             pull_from_dram_lfu(spine->lfu_sram, spine->dram, flow->flow_id);
                    //         }
                    //         else {
                    //             dm_pull_from_dram(spine->dm_sram, spine->dram, flow->flow_id);
                    //         }
                    //     }
                    //     for (int k = 0; k < NUM_OF_RACKS; k++) {
                    //         tor_t tor = tors[k];
                    //         if (sram_type == 1) {
                    //             pull_from_dram(tor->sram, tor->dram, flow->flow_id);
                    //         }
                    //         else if (sram_type == 2) {
                    //             pull_from_dram_lfu(tor->lfu_sram, tor->dram, flow->flow_id);
                    //         }
                    //         else {
                    //             dm_pull_from_dram(tor->dm_sram, tor->dram, flow->flow_id);
                    //         }
                    //     }
                    // }
                }
                flow = node->current_flow;
                // Sending Period
                if (incast_sent_on_period <= incast_size) {
                    // Send packets from currently selected flows
                    if (flow != NULL && (flow->active == 1 || flow->timeslot <= curr_timeslot)) {
                        int src_node = flow->src;
                        int dst_node = flow->dst;
                        int64_t flow_id = flow->flow_id;

                        // Send packets from flow until cwnd is reached or the flow runs out of bytes to send
                        int64_t flow_bytes_remaining = flow->flow_size_bytes - flow->bytes_sent;
                        //int64_t cwnd_bytes_remaining = node->cwnd[dst_node] * MTU - (node->seq_num[flow_id] - node->last_acked[flow_id]);
                        //if (flow_bytes_remaining > 0 && cwnd_bytes_remaining > 0) {
                        if (flow_bytes_remaining > 0) {
                            // Determine packet sizee
                            int64_t size = MTU;
                            //if (cwnd_bytes_remaining < MTU) {
                            //    size = cwnd_bytes_remaining;
                            //}
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
                            int dst_tor = node_index / NODES_PER_RACK;
                            pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                            link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
#ifdef DEBUG_DRIVER
                            printf("%d: host %d created and sent pkt to ToR %d\n", (int) curr_timeslot, i, (int) dst_tor);
#endif

                            // Update flow state
                            if (flow->active == 0) {
                                flowlist->active_flows++;
                                flow->start_timeslot = curr_timeslot;
                                total_flows_started++;
                            }
                            flow->active = 1;
                            flow->pkts_sent++;
                            flow->bytes_sent += size;
                            flow->timeslots_active++;
                            // Determine if last packet of flow has been sent
                            if (flow->pkts_sent >= flow->flow_size) {
                                flow->active = 0;
                                flowlist->active_flows--;
                            }
                        }
                    }
                }
                // Recovery Period: send nothing
                //else {
                //}    
            }
        }
        new_period = 0;

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
#ifdef ENABLE_SNAPSHOTS
                //send snapshots every snapshot_epoch
                if (curr_timeslot % snapshot_epoch == 0) {
                    snapshot_t * snapshot = snapshot_to_spine(tor, tor_port);
                    if (snapshot != NULL) {
                        if (snapshot->flow_id[0] != -1) {
                            snapshot->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                            ipg_send(links->tor_to_spine_link[tor_index][tor_port], snapshot);
#ifdef DEBUG_SNAPSHOTS
                            printf("%d: ToR %d sent snapshot to spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
                        }
                        else {
                            free(snapshot);
                        }
                    }
                }
#endif
                // Send packets to spine
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(tor->upstream_send_buffer[tor_port], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot) {
                    packet_t pkt = NULL;
                    pkt = (packet_t) buffer_get(tor->upstream_send_buffer[tor_port]);
                    if (pkt != NULL) {
                        pkt->snapshotted = 0;
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots; 
                        link_enqueue(links->tor_to_spine_link[tor_index][tor_port], pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: ToR %d sent pkt to spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(tor->upstream_send_buffer[tor_port], 0);
                    }
                    else {
                        break;
                    }
                }

            }

            //send to each host
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port) {
                if (i == 8 && tor_port == 8) {
                    if (incast_buffer_len >= 0 && tor->downstream_pkt_buffer[tor_port]->num_elements == 0) {
                        incast_sent_on_period = 0;
                        new_period = 1;
                    }
                    incast_buffer_len = tor->downstream_pkt_buffer[tor_port]->num_elements;
                }
                int bytes_sent = 0;
                packet_t peek_pkt = buffer_peek(tor->downstream_send_buffer[tor_port], 0);
                while (peek_pkt != NULL && bytes_sent + peek_pkt->size <= bytes_per_timeslot) {
                    packet_t pkt = NULL;
                    pkt = (packet_t) buffer_get(tor->downstream_send_buffer[tor_port]);
                    
                    if (pkt != NULL) {
                        int16_t dst_host = pkt->dst_node;
                        pkt->time_to_dequeue_from_link = curr_timeslot + per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->tor_to_host_link[tor_index][dst_host],pkt);
#ifdef DEBUG_DRIVER
                        printf("%d: ToR %d sent pkt to host %d\n", (int) curr_timeslot, i, dst_host);
                        print_packet(pkt);
#endif
                        bytes_sent += pkt->size;
                        peek_pkt = buffer_peek(tor->downstream_send_buffer[tor_port], 0);
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
                    packet_t pkt = (packet_t)
                        link_dequeue(links->host_to_tor_link[src_host][tor_index]);
                    if (pkt != NULL) {
                        buffer_put(tor->upstream_pkt_buffer[tor_port], pkt);
                        // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                        if (tor->upstream_pkt_buffer[tor_port]->num_elements > ECN_CUTOFF_TOR_UP) {
                            pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                            printf("%d: ToR %d marked upstream pkt with ECN\n", (int) curr_timeslot, i);
#endif
                        }

#ifdef DEBUG_DRIVER
                        printf("%d: Tor %d recv pkt from host %d\n", (int) curr_timeslot, i, src_host);
#endif
#ifdef RECORD_PACKETS
                        if (pkt->control_flag == 0) {
                            fprintf(tor_outfiles[i], "%d, %d, %d, %d, %d, %d, up\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) tor_port, (int) (curr_timeslot), (int) pkt->time_when_transmitted_from_src);
                        }
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
                        buffer_put(tor->downstream_pkt_buffer[tor_port], pkt);
                        // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                        if (tor->downstream_pkt_buffer[tor_port]->num_elements > ECN_CUTOFF_TOR_DOWN) {
                            pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                            printf("%d: ToR %d marked downstream pkt with ECN\n", (int) curr_timeslot, i);
#endif
                        }
#ifdef DEBUG_DRIVER
                        printf("%d: Tor %d recv pkt from spine %d\n", (int) curr_timeslot, i, tor_port);
#endif
#ifdef RECORD_PACKETS
                        if (pkt->control_flag == 0) {
                            fprintf(tor_outfiles[i], "%d, %d, %d, %d, %d, %d, down\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) tor_port, (int) (curr_timeslot), (int) pkt->time_when_transmitted_from_src);
                        }
#endif
                        bytes_rcvd += pkt->size;
                    }
                    peek_pkt = (packet_t) link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                }
#ifdef ENABLE_SNAPSHOTS
                // Receive snapshot
                snapshot_t * snapshot = (snapshot_t * ) ipg_peek(links->spine_to_tor_link[src_spine][tor_index]);
                if (snapshot != NULL) {
                    snapshot = ipg_recv(links->spine_to_tor_link[src_spine][tor_index]);
                    // Process snapshot
                    for (int cnt = 0; cnt < SNAPSHOT_SIZE; cnt++) {
                        int64_t snapshot_flow = (int64_t) snapshot->flow_id[cnt];
                        if (enable_sram != 0 && snapshot_flow >= 0) {
                            int64_t flow_id = snapshot_flow;
                            // Determine destination for flow
                            buff_node_t * node = malloc(sizeof(buff_node_t));
                            node->val = flow_id;
                            buffer_put(tor->downstream_snapshot_list[tor_port], node);
                        }
                    }
                    free(snapshot);
                }
#endif
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
                    if (pkt->control_flag == 0) {
                        fprintf(spine_outfiles[i], "%d, %d, %d, %d, %d, %d\n", (int) pkt->flow_id, (int) pkt->src_node, (int) pkt->dst_node, (int) spine_port, (int) curr_timeslot, (int) pkt->time_when_transmitted_from_src);
                    }
#endif
                    //enq packet in the virtual queue
                    pkt->time_when_added_to_spine_queue = curr_timeslot;
                    pkt->time_arrived_at_spine = curr_timeslot;
                    assert(buffer_put(spine->pkt_buffer[spine_port], pkt) != -1);
                    // DCTCP: Mark packet when adding packet exceeds ECN cutoff
                    if (spine->pkt_buffer[spine_port]->num_elements > ECN_CUTOFF_SPINE) {
                        pkt->ecn_flag = 1;
#ifdef DEBUG_DCTCP
                        printf("%d: Spine %d marked pkt with ECN\n", (int) curr_timeslot, spine_index);
#endif
                    }
                    bytes_rcvd += pkt->size;
                    peek_pkt = (packet_t) link_peek(links->tor_to_spine_link[src_tor][spine_index]);
                }
#ifdef ENABLE_SNAPSHOTS
                // Receive snapshot
                snapshot_t * snapshot = (snapshot_t * ) ipg_peek(links->tor_to_spine_link[src_tor][spine_index]);
                if (snapshot != NULL) {
                    snapshot = ipg_recv(links->tor_to_spine_link[src_tor][spine_index]);
                    // process snapshot 
                    for (int cnt = 0; cnt < SNAPSHOT_SIZE; cnt++) {
                        int64_t snapshot_flow = (int64_t) snapshot->flow_id[cnt];
                        if (enable_sram != 0 && snapshot_flow >= 0) {
                            int64_t flow_id = snapshot_flow;
                            buff_node_t * node = malloc(sizeof(buff_node_t));
                            node->val = flow_id;
                            buffer_put(spine->snapshot_list[spine_port], node);
                        }
                    }

                    free(snapshot);
#ifdef DEBUG_SNAPSHOTS
                    printf("%d: Spine %d recv snapshot from ToR %d\n", (int) curr_timeslot, spine_index, src_tor);
#endif
                }
#endif
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
                    total_pkts_rcvd++;

                    if (flow->pkts_received == flow->flow_size) {
                        flow->active = 0;
                        flow->finished = 1;
                        flow->finish_timeslot = curr_timeslot;
                        num_of_flows_finished++;
                        write_to_outfile(out_fp, flow, timeslot_len, link_bandwidth);
                        printf("%d flows finished\n", num_of_flows_finished);
                        printf("%d: Flow %d finished in %d timeslots\n", (int) curr_timeslot, (int) flow->flow_id, (int) (flow->finish_timeslot - flow->timeslot));
                        fflush(stdout);
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
                 //Prefetch flows with Belady's Algorithm
/*---------------------------------------------------------------------------*/
        
#ifdef ENABLE_SNAPSHOTS 
        if (curr_timeslot % belady_epoch == 0) {
            for (int i = 0; i < NUM_OF_RACKS; i++) {
                tor_t tor = tors[i];
                int q_len = 0;
                int64_t * lin_queue = linearize_tor_downstream_queues(tor, &q_len);

                if (q_len > 0) {
                    // Organize SRAM by going through lin_queue backwards, tracking soonest not in SRAM
                    // This will order SRAM based on how soon the snapshots believe they will arrive at switch
                    int64_t id_to_fetch = -1;
                    int placement_index = 0;
                    for (int j = q_len - 1; j >= 0; j--) {
                        int64_t id = lin_queue[j];
                        int index = access_sram_return_index(tor->sram, id);
                        if (index < 0) {
                            id_to_fetch = id;
                            placement_index = 0;
                        }
                        else if (index >= placement_index) {
                            placement_index++;
                        }
                    }

                    if (tor->dram->lock == 0 && id_to_fetch >= 0 && placement_index < tor->sram->capacity) {
                        tor->dram->lock = 1;
                        tor->dram->accessing = id_to_fetch;
                        tor->dram->placement_idx = placement_index;
                        //printf("%d: ToR %d prefetching %d to index %d\n", (int) curr_timeslot, i, id_to_fetch, placement_index);
                    }

                    free(lin_queue);
                }
            }
            
            for (int i = 0; i < NUM_OF_SPINES; i++) {
                spine_t spine = spines[i];
                int q_len = 0;
                int64_t * lin_queue = linearize_spine_queues(spine, &q_len);

                if (q_len > 0) {
                    // Organize SRAM by going through lin_queue backwards, tracking soonest not in SRAM
                    // This will order SRAM based on how soon the snapshots believe they will arrive at switch
                    int64_t id_to_fetch = -1;
                    int placement_index = 0;
                    for (int j = q_len - 1; j >= 0; j--) {
                        int64_t id = lin_queue[j];
                        int index = access_sram_return_index(spine->sram, id);
                        if (index < 0) {
                            id_to_fetch = id;
                            placement_index = 0;
                        }
                        else if (index >= placement_index) {
                            placement_index++;
                        }
                    }

                    if (spine->dram->lock == 0 && id_to_fetch >= 0 && placement_index < spine->sram->capacity) {
                        spine->dram->lock = 1;
                        spine->dram->accessing = id_to_fetch;
                        spine->dram->placement_idx = placement_index;
                        //printf("%d: Spine %d prefetching %d to index %d\n", (int) curr_timeslot, i, id_to_fetch, placement_index);
                    }

                    free(lin_queue);
                }
            }
        }
#endif

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

        // if (curr_timeslot >= max_timeslots) {
        //     printf("\nReached max timeslot %d\n\n", (int) curr_timeslot);
        //     terminate3 = 1;
        // }

        // if (cache_hits + cache_misses >= max_cache_accesses) {
        //     //printf("\nReached %d cache accesses\n\n", (int) max_cache_accesses);
        //     terminate4 = 1;
        // }

        // if (curr_timeslot % 100000 == 99999) {
        //     printf("%d: %d bytes received\n", curr_timeslot, total_bytes_rcvd);
        // }

        // if (total_bytes_rcvd >= max_bytes_rcvd) {
        //     printf("\nReached %d bytes received\n\n", max_bytes_rcvd);
        //     terminate5 = 1;
        // }

        if (terminate0 || terminate1 || terminate2 || terminate3 || terminate4 || terminate5) {
            int completed_flows = 0;
            int flow_completion_times[flowlist->num_flows];
            int slowdowns[flowlist->num_flows];
            float avg_slowdown = 0;
            printf("FCT: ");
            for (int i = 0; i < flowlist->num_flows; i++) {
                if (flowlist->flows[i]->finished > 0) {
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

            if (completed_flows > 0) {
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
            printf("Finished in %d timeslots\n", (int) curr_timeslot);
            printf("Finished in %f seconds\n", curr_time);
            printf("Finished in %d bytes\n", total_bytes_rcvd);
            fflush(stdout);
            break;
        }

        // if (curr_timeslot % 50 == 0) {
        //     printf(".");
        // }
        // if (curr_timeslot % 1000 == 999) {
        //     printf(".\n");
        // }

        fflush(stdout);
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

void shuffle(int * array, size_t n) {
    if (n > 1) {
        // Choose 
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

void process_args(int argc, char ** argv) {
    int opt;
    char filename[500] = "";
    char out_filename[504] = "";
    char out_suffix[4] = ".out";
    char timeseries_filename[515] = "";
    char timeseries_suffix[15] = ".timeseries.csv";

    while ((opt = getopt(argc, argv, "f:b:c:h:d:n:m:t:q:a:e:s:i:u:l:p:k:v:x:y:z:")) != -1) {
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
                sram_type = atoi(optarg);
                if (sram_type == 0) {
                    printf("Using direct-mapped SRAM\n");
                }
                else if (sram_type == 1) {
                    printf("Using LRU SRAM\n");
                }
                else if (sram_type == 2) {
                    printf("Using LFU SRAM\n");
                }
                else if (sram_type == 3) {
                    printf("Using ARC SRAM\n");
                }
                else if (sram_type == 4) {
                    printf("Using S3-FIFO SRAM\n");
                }
                else if (sram_type == 5) {
                    printf("Using SIEVE SRAM\n");
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
                sram_size = atoi(optarg);
                printf("Utilizing SRAM size of %d\n", (int) sram_size);
                for (int i = 0; i < NUM_OF_SPINES; i++) {
                    spines[i]->sram->capacity = sram_size;
                    spines[i]->lfu_sram->capacity = sram_size;
                    spines[i]->arc_sram->capacity = sram_size;
                    spines[i]->s3f_sram->capacity = sram_size;
                    spines[i]->s3f_sram->s_fifo->size = sram_size / 10;
                    spines[i]->s3f_sram->m_fifo->size = sram_size - sram_size / 10;
                    spines[i]->s3f_sram->g_fifo->size = sram_size - sram_size / 10;
                    spines[i]->sve_sram->capacity = sram_size;
                    spines[i]->dm_sram->capacity = sram_size;
                }
                for (int i = 0; i < NUM_OF_RACKS; i++) {
                    tors[i]->sram->capacity = sram_size;
                    tors[i]->lfu_sram->capacity = sram_size;
                    tors[i]->arc_sram->capacity = sram_size;
                    tors[i]->s3f_sram->capacity = sram_size;
                    tors[i]->s3f_sram->s_fifo->size = sram_size / 10;
                    tors[i]->s3f_sram->m_fifo->size = sram_size - sram_size / 10;
                    tors[i]->s3f_sram->g_fifo->size = sram_size - sram_size / 10;
                    tors[i]->sve_sram->capacity = sram_size;
                    tors[i]->dm_sram->capacity = sram_size;
                }
                break;
            case 'i':
                init_sram = atoi(optarg);
                if (init_sram == 1) {
                    printf("Initializing SRAM\n");
                    for (int i = 0; i < NUM_OF_SPINES; i++) {
                        initialize_sram(spines[i]->sram);
                        initialize_lfu_sram(spines[i]->lfu_sram);
                        initialize_arc_sram(spines[i]->arc_sram);
                        initialize_s3f_sram(spines[i]->s3f_sram);
                        initialize_sve_sram(spines[i]->sve_sram);
                        initialize_dm_sram(spines[i]->dm_sram);
                    }
                    for (int i = 0; i < NUM_OF_RACKS; i++) {
                        initialize_sram(tors[i]->sram);
                        initialize_lfu_sram(tors[i]->lfu_sram);
                        initialize_arc_sram(tors[i]->arc_sram);
                        initialize_s3f_sram(tors[i]->s3f_sram);
                        initialize_sve_sram(tors[i]->sve_sram);
                        initialize_dm_sram(tors[i]->dm_sram);
                    }
                }
                break;
            case 'u':
                burst_size = atoi(optarg);
                printf("Using packet burst size of %d\n", burst_size);
                break;
            case 'l':
                load = atof(optarg);
                printf("Using a load of %0.1f\n", load);
                break;
            case 'p':
                program_tors = atof(optarg);
                printf("Enabling programmable ToRs\n");
                break;
            case 'k':
                dram_access_time = atof(optarg);
                printf("DRAM access speed is %d ns\n", dram_access_time);
                break;
            case 'v':
                packet_mode = 1;
                incast_active = atoi(optarg);
                printf("Incast %d active flows per incast period\n", incast_active);
                break;
            case 'x':
                packet_mode = 1;
                incast_switch = atoi(optarg);
                printf("Incast switching %d flows per incast period\n", incast_switch);
                break;
            case 'y':
                packet_mode = 1;
                incast_size = atoi(optarg);
                printf("Incast duration of %d packets\n", incast_size);
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

    timeslots_per_dram_access = dram_access_time / timeslot_len + (dram_access_time % (int) timeslot_len != 0);
    printf("Timeslots per DRAM access %d\n", timeslots_per_dram_access);

    accesses_per_timeslot = timeslot_len / dram_access_time + ((int) timeslot_len % dram_access_time != 0);
    printf("DRAM accesses per timeslot %d\n", accesses_per_timeslot);

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
        spines[i]->dram->delay = timeslots_per_dram_access;
        spines[i]->dram->accesses = timeslot_len;
    }
    for (int i = 0; i < NUM_OF_RACKS; i++) {
        tors[i]->dram->delay = timeslots_per_dram_access;
        spines[i]->dram->accesses = timeslot_len;
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
            flow_size_pkts = flow_size_bytes / MTU;
            initialize_flow(flow_id, src, dst, flow_size_pkts, flow_size_bytes, timeslot);
            // Final timeslot is start time of last flow
            //max_timeslots = timeslot;
        }
        
        printf("Flows initialized\n");
        fclose(fp);
    }
    return;
}

void initialize_flow(int flow_id, int src, int dst, int flow_size_pkts, int flow_size_bytes, int timeslot) {
    if (flow_id < MAX_FLOW_ID) {
        flow_t * new_flow = create_flow(flow_id, flow_size_pkts, flow_size_bytes, src, dst, timeslot);
        new_flow->expected_runtime = flow_size_bytes / MTU + 6;
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
        tors[i] = create_tor(i, sram_size, 0);
    }
    printf("ToRs initialized\n");

    //create spines
    spines = (spine_t*) malloc(NUM_OF_SPINES * sizeof(spine_t));
    MALLOC_TEST(spines, __LINE__);
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        spines[i] = create_spine(i, sram_size, 0);
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
        fprintf(spine_outfiles[i], "flow_id, src, dst, port, arrival_time, creation_time\n");
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
        fprintf(tor_outfiles[i], "flow_id, src, dst, port, arrival_time, creation_time, direction\n");
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
    print_system_stats(spines, tors, total_bytes_rcvd, total_pkts_rcvd, (int64_t) (curr_timeslot * timeslot_len), cache_misses, cache_hits, avg_delay_at_spine, avg_max_delay_at_spine, timeslot_len);
#ifdef WRITE_QUEUELENS
    write_to_timeseries_outfile(timeseries_fp, spines, tors, curr_timeslot, num_datapoints);
#endif

    free_flows();
    free_network();
    close_outfiles();

    printf("Finished execution\n");

    return 0;
}
