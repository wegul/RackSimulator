#include "driver.h"

// Default values for simulation
static int pkt_size = 1500; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len; //in ns

int16_t header_overhead = 0;
float per_hop_propagation_delay_in_ns = 100;
int per_hop_propagation_delay_in_timeslots;
volatile int64_t curr_timeslot = 0; //extern var
int num_datapoints = 100000;

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;
static volatile int8_t terminate4 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 500000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 500000; //stop after these many flows start

volatile int64_t max_timeslots = 1000000; // extern var

// Output file
FILE * out_fp = NULL;
FILE * timeseries_fp = NULL;

// Network
node_t * nodes;
tor_t * tors;
spine_t * spines;
links_t links;
flowlist_t * flowlist;

void work_per_timeslot()
{
    while (1) {
/*---------------------------------------------------------------------------*/
                                  //SPINE -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;

            //record spine port queue lengths
            for (int j = 0; j < SPINE_PORT_COUNT; ++j) {
                int32_t size = (spine->pkt_buffer[j])->num_elements;
                //++(spine->queue_stat.queue_len_histogram[size]);
                timeseries_add(spine->queue_stat[j], size);
            }

            //send the packet
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                packet_t pkt = send_to_tor(spine, k);
                if (pkt != NULL) {
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->spine_to_tor_link[spine_index][k], pkt);
#ifdef DEBUG_DRIVER
                    printf("spine %d sent pkt to ToR %d on ts %d\n", i, k, (int) curr_timeslot);
#endif
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //HOST -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;

            if ((node->active_flows)->num_elements > 0) {
                flow_t * flow = buffer_peek(node->active_flows, 0);
                if (flow->active == 1 || flow->timeslot <= curr_timeslot){
                    flow = buffer_get(node->active_flows);
                    int16_t src_node = flow->src;
                    int16_t dst_node = flow->dst;
                    int64_t flow_id = flow->flow_id;
                    int64_t seq_num = node->seq_num[dst_node];
                    int64_t size = 0;

                    int64_t flow_bytes_remaining = flow->flow_size - flow->bytes_sent;
                    if (flow_bytes_remaining < 1500) {
                        size = flow_bytes_remaining;
                    }
                    else {
                        size = 1500;
                    }

                    packet_t pkt = create_packet(src_node, dst_node, flow_id, size, seq_num);
                    node->seq_num[dst_node]++;
                    if (flow->active == 0) {
                        flowlist->active_flows++;
                        flow->start_timeslot = curr_timeslot;
                        total_flows_started++;
#ifdef DEBUG_DRIVER 
                        printf("flow %d started\n", (int) flow_id);
#endif
                    }
                    flow->active = 1;
                    flow->pkts_sent++;
                    flow->bytes_sent += size;

                    if (flow->pkts_sent >= flow->flow_size) {
                        flowlist->active_flows--;
                        flow->active = 0;
#ifdef DEBUG_DRIVER
                        printf("flow %d sending final packet\n", (int) flow_id);
#endif
                    }
                    else {
                        buffer_put(node->active_flows, flow);
                    }

                    pkt->time_when_transmitted_from_src = curr_timeslot;
                    int16_t dst_tor = node_index / NODES_PER_RACK;
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
#ifdef DEBUG_DRIVER
                    printf("host %d created pkt at time %d\n", i, (int) curr_timeslot);
                    print_packet(pkt);
#endif
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
                int32_t size = (tor->upstream_pkt_buffer[j])->num_elements;
                timeseries_add(tor->upstream_queue_stat[j], size);
            }

            //record downstream port queue lengths
            for (int j = 0; j < NODES_PER_RACK; ++j) {
                int32_t size = (tor->downstream_pkt_buffer[j])->num_elements;
                timeseries_add(tor->downstream_queue_stat[j], size);
            }

            //send to each spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                packet_t pkt = send_to_spine(tor, tor_port);
                if (pkt != NULL) {
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots; 
                    link_enqueue(links->tor_to_spine_link[tor_index][tor_port], pkt);
#ifdef DEBUG_DRIVER
                    printf("ToR %d sent pkt to spine %d\n", i, tor_port);
#endif
                }
            }

            //send to each host
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port) {
                packet_t pkt = send_to_host(tor, tor_port);
                if (pkt != NULL) {
                    int16_t dst_host = pkt->dst_node;
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->tor_to_host_link[tor_index][dst_host],pkt);
#ifdef DEBUG_DRIVER
                    printf("ToR %d sent pkt to host %d\n", i, dst_host);
                    print_packet(pkt);
#endif
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
                packet_t pkt = (packet_t)
                    link_peek(links->host_to_tor_link[src_host][tor_index]);
                if (pkt != NULL && pkt->time_to_dequeue_from_link == curr_timeslot) {
                    int spine_id = hash(tor->routing_table, pkt->flow_id);
                    pkt = (packet_t)
                        link_dequeue(links->host_to_tor_link[src_host][tor_index]);
                    if (pkt != NULL) {
                        buffer_put(tor->upstream_pkt_buffer[spine_id], pkt);
#ifdef DEBUG_DRIVER
                        printf("Tor %d recv pkt from host %d\n", i, tor_port);
#endif
                    }
                }
            }

            //Recv packet from spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                //deq packet from the link
                int16_t src_spine = tor_port;
                packet_t pkt = (packet_t)
                    link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                if (pkt != NULL && pkt->time_to_dequeue_from_link == curr_timeslot) {
                    pkt = (packet_t)
                        link_dequeue(links->spine_to_tor_link[src_spine][tor_index]);
                    if (pkt != NULL) {
                        int node_index = pkt->dst_node % NODES_PER_RACK;
                        buffer_put(tor->downstream_pkt_buffer[node_index], pkt);
#ifdef DEBUG_DRIVER
                        printf("Tor %d recv pkt from spine %d on ts %d\n", i, tor_port, (int) curr_timeslot);
#endif
                    }
                }
            }
        }

/*---------------------------------------------------------------------------*/
                                  //SPINE -- RECV
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;


            for (int spine_port=0; spine_port<SPINE_PORT_COUNT; ++spine_port) {
                //deq packet
                int16_t src_tor = spine_port;
                packet_t pkt = (packet_t) link_peek
                    (links->tor_to_spine_link[src_tor][spine_index]);
                if (pkt != NULL && pkt->time_to_dequeue_from_link == curr_timeslot) {  
                    pkt = (packet_t) link_dequeue
                        (links->tor_to_spine_link[src_tor][spine_index]);
#ifdef DEBUG_DRIVER
                    printf("Spine %d recv pkt from ToR %d\n", spine_index, src_tor);
#endif
                    //enq packet in the virtual queue
                    int16_t dst_host = pkt->dst_node;
                    int16_t dst_tor = dst_host / NODES_PER_RACK;
                    if (dst_host != -1) {
                        pkt->time_when_added_to_spine_queue = curr_timeslot;
                        assert(buffer_put(spine->pkt_buffer[dst_tor], pkt)                                                                                                                                                                                                                                        
                                != -1);
                        if (dst_host == -1) {
                            free_packet(pkt);
                        }
                    }
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
            packet_t pkt = (packet_t)
                link_peek(links->tor_to_host_link[src_tor][node_index]);
            
            if (pkt != NULL && pkt->time_to_dequeue_from_link == curr_timeslot) {
                assert(pkt->dst_node == node_index);
                pkt = (packet_t)
                    link_dequeue(links->tor_to_host_link[src_tor][node_index]);
#ifdef DEBUG_DRIVER
                printf("host %d received packet from ToR %d\n", node_index, src_tor);
#endif
                flow_t * flow = flowlist->flows[pkt->flow_id];
                assert(flow != NULL);
                flow->pkts_received++;
                flow->bytes_received += pkt->size;

                if (flow->pkts_received == flow->flow_size) {
                    assert(flow->bytes_sent == flow->bytes_received);
                    flow->active = 0;
                    flow->finished = 1;
                    flow->finish_timeslot = curr_timeslot;
                    num_of_flows_finished++;
                    write_to_outfile(out_fp, flow, timeslot_len, link_bandwidth);
#ifdef DEBUG_DRIVER
                    printf("Flow %d finished\n", (int) flow->flow_id);
#endif
                }
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
            printf("Started %d flows\n\n", (int) total_flows_started);
            terminate1 = 1;
        }

        if (num_of_flows_finished >= num_of_flows_to_finish) {
            printf("Finished %d flows\n\n", (int) num_of_flows_finished);
            terminate2 = 1;
        }

        if (curr_timeslot >= max_timeslots) {
            printf("Reached max timeslot %d\n\n", (int) curr_timeslot);
            terminate3 = 1;
        }

        if (terminate0 || terminate1 || terminate2 || terminate3) {
            double curr_time = curr_timeslot * timeslot_len / 1e9;
            printf("Finished in %d timeslots\n", (int) curr_timeslot);
            printf("Finished in %f seconds\n", curr_time);
            break;
        }

        curr_timeslot++;
    }


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

    while ((opt = getopt(argc, argv, "f:b:c:h:d:n:m:t:q:")) != -1) {
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
            case 'f': 
                if (strlen(optarg) < 500) {
                    strcpy(filename, optarg);
                    strncpy(out_filename, filename, strlen(filename));
                    strncat(out_filename, out_suffix, 4);
                    strncpy(timeseries_filename, filename, strlen(filename));
                    strncat(timeseries_filename, timeseries_suffix, 15);
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
    printf("\n");

    read_tracefile(filename);
    out_fp = open_outfile(out_filename);
#ifdef WRITE_QUEUELENS
    timeseries_fp = open_timeseries_outfile(timeseries_filename);
#endif
}

void read_tracefile(char * filename) {
    FILE * fp;
    flowlist = create_flowlist();
    if (strcmp(filename, "")) {
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
        buffer_put(nodes[src]->active_flows, new_flow);
#ifdef DEBUG_DRIVER
        printf("initialized flow %d src %d dst %d flow_size %d ts %d\n", flow_id, src, dst, flow_size_pkts, timeslot);
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
        tors[i] = create_tor(i);
    }
    printf("ToRs initialized\n");

    //create spines
    spines = (spine_t*) malloc(NUM_OF_SPINES * sizeof(spine_t));
    MALLOC_TEST(spines, __LINE__);
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        spines[i] = create_spine(i);
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

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    initialize_network();
    process_args(argc, argv);
    
    work_per_timeslot();
    print_system_stats(spines, tors);
#ifdef WRITE_QUEUELENS
    write_to_timeseries_outfile(timeseries_fp, spines, tors, curr_timeslot, num_datapoints);
#endif

    free_flows();
    free_network();
    fclose(out_fp);
#ifdef WRITE_QUEUELENS
    fclose(timeseries_fp);
#endif

    printf("Finished execution\n");

    return 0;
}