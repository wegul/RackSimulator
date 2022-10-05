#include "driver.h"

// Default values for simulation
static int pkt_size = 64; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len; //in ns

int16_t header_overhead = 0;
float per_hop_propagation_delay_in_ns = 0;
int per_hop_propagation_delay_in_timeslots;
volatile int64_t curr_timeslot = 0; //extern var

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;
static volatile int8_t terminate4 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 500000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 500000; //stop after these many flows start

volatile int64_t max_timeslots = 10000000; // extern var

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
                //printf("spine %d port %d buflen %d\n", i, j, size);
                assert(size <= SPINE_PORT_BUFFER_LEN);
                ++(spine->queue_stat.queue_len_histogram[size]);
            }

            //send the packet
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                packet_t pkt = send_to_tor(spine, k);
                if (pkt != NULL) {
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots;
                    link_enqueue(links->spine_to_tor_link[spine_index][k], pkt);
                    printf("spine %d sent pkt to ToR %d\n", i, k);
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
                int j = 0;
                int pkt_sent = 0;
                while (pkt_sent == 0 && j < (node->active_flows)->num_elements) {
                    flow_t * check_flow = buffer_peek(node->active_flows, j);
                    if (check_flow->active == 1 || check_flow->timeslot <= curr_timeslot){
                        flow_t * flow = buffer_get(node->active_flows);
                        int16_t src_node = flow->src;
                        int16_t dst_node = flow->dst;
                        int64_t flow_id = flow->flow_id;
                        int64_t seq_num = node->seq_num[dst_node];
                        packet_t pkt = create_packet(src_node, dst_node, flow_id, seq_num);
                        node->seq_num[dst_node]++;
                        if (flow->active == 0) {
                            flowlist->active_flows++;
                            total_flows_started++;
                            printf("flow %d started\n", (int) flow_id);
                        }
                        flow->active = 1;
                        flow->pkts_sent++;
                        pkt_sent = 1;

                        if (flow->pkts_sent < flow->flow_size) {
                            buffer_put(node->active_flows, flow);
                        }
                        else {
                            flowlist->active_flows--;
                            flow->active = 0;
                            printf("flow %d sent final packet\n", (int) flow_id);
                        }

                        pkt->time_when_transmitted_from_src = curr_timeslot;
                        int16_t dst_tor = node_index / NODES_PER_RACK;
                        pkt->time_to_dequeue_from_link = curr_timeslot +
                            per_hop_propagation_delay_in_timeslots;
                        link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
                        printf("host %d created pkt at time %d\n", i, (int) curr_timeslot);
                        print_packet(pkt);
                    }
                    j++;
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
                ++(tor->queue_stat.upstream_queue_len_histogram[size]);
            }

            //record downstream port queue lengths
            for (int j = 0; j < NODES_PER_RACK; ++j) {
                int32_t size = (tor->downstream_pkt_buffer[j])->num_elements;
                ++(tor->queue_stat.downstream_queue_len_histogram[size]);
            }

            //send to each spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                packet_t pkt = send_to_spine(tor, tor_port);
                if (pkt != NULL) {
                    pkt->time_to_dequeue_from_link = curr_timeslot +
                        per_hop_propagation_delay_in_timeslots; 
                    link_enqueue(links->tor_to_spine_link[tor_index][tor_port], pkt);
                    printf("ToR %d sent pkt to spine %d\n", i, tor_port);
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
                    printf("ToR %d sent pkt to host %d\n", i, dst_host);
                    print_packet(pkt);
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
                if (pkt != NULL) {
                    int spine_id = hash(tor->routing_table, pkt->flow_id);
                    pkt = (packet_t)
                        link_dequeue(links->host_to_tor_link[src_host][tor_index]);
                    if (pkt != NULL) {
                        buffer_put(tor->upstream_pkt_buffer[spine_id], pkt);
                        printf("Tor %d recv pkt from host %d\n", i, tor_port);
                    }
                }
            }

            //Recv packet from spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                //deq packet from the link
                int16_t src_spine = tor_port;
                packet_t pkt = (packet_t)
                    link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                if (pkt != NULL) {
                    pkt = (packet_t)
                        link_dequeue(links->spine_to_tor_link[src_spine][tor_index]);
                    if (pkt != NULL) {
                        buffer_put(tor->downstream_pkt_buffer[src_spine], pkt);
                        printf("Tor %d recv pkt from spine %d\n", i, tor_port);
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
                if (pkt != NULL) {  
                    pkt = (packet_t) link_dequeue
                        (links->tor_to_spine_link[src_tor][spine_index]);

                    printf("Spine %d recv pkt from ToR %d\n", spine_index, src_tor);

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
            
            if (pkt != NULL) {
                assert(pkt->dst_node == node_index);
                pkt = (packet_t)
                    link_dequeue(links->tor_to_host_link[src_tor][node_index]);
                printf("host %d received packet from ToR %d\n", node_index, src_tor);
                flow_t * flow = flowlist->flows[pkt->flow_id];
                assert(flow != NULL);
                flow->pkts_received++;

                if (flow->pkts_received == flow->flow_size) {
                    flow->active = 0;
                    flow->finished = 1;
                    num_of_flows_finished++;
                    printf("Flow %d finished\n", (int) flow->flow_id);
                }
                free_packet(pkt);
            }
        }

/*---------------------------------------------------------------------------*/
                 //Data logging and state updates before next iteration
/*---------------------------------------------------------------------------*/
        if (flowlist->active_flows < 1) {
            int no_flows_left = 1;
            for (int i = 0; i < flowlist->num_flows; i++) {
                if ((flowlist->flows[i])->finished == 0) {
                    no_flows_left = 0;
                }
            }
            if (no_flows_left > 0) {
                terminate0 = 1;
            }
        }

        if (total_flows_started >= num_of_flows_to_start) {
            terminate1 = 1;
        }

        if (num_of_flows_finished >= num_of_flows_to_finish) {
            terminate2 = 1;
        }

        if (curr_timeslot >= max_timeslots) {
            terminate3 = 1;
        }

        if (terminate0) {
            printf("Finished all flows\n\n");
            break;
        }

        if (terminate1) {
            printf("Started %d flows\n\n", (int) total_flows_started);
            break;
        }

        if (terminate2) {
            printf("Finished %d flows\n\n", (int) num_of_flows_finished);
            break;
        }
         
        if (terminate3) {
            printf("Reached max timeslot %d\n\n", (int) curr_timeslot);
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
    //char out_filename[500] = "";

    while ((opt = getopt(argc, argv, "f:b:c:h:d:n:m:t:")) != -1) {
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
            case 'f': 
                if (strlen(optarg) < 500)
                    strcpy(filename, optarg);
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
        
        while (fscanf(fp, "%d,%d,%d,%d,%d,%d", &flow_id, &src, &dst, &flow_size_bytes, &flow_size_pkts, &timeslot) == 6 ) {
            initialize_flow(flow_id, src, dst, flow_size_pkts, timeslot);
        }
        
        printf("Flows initialized\n");
        fclose(fp);
    }
    return;
}

void initialize_flow(int flow_id, int src, int dst, int flow_size_pkts, int timeslot) {
    flow_t * new_flow = create_flow(flow_id, flow_size_pkts, src, dst, timeslot);
    add_flow(flowlist, new_flow);
    buffer_put(nodes[src]->active_flows, new_flow);
    printf("initialized flow %d src %d dst %d flow_size %d ts %d\n", flow_id, src, dst, flow_size_pkts, timeslot);
}

void initialize_flows() {
    flowlist = create_flowlist();
    flow_t * f1 = create_flow(0, 1, 20, 65, 0);
    flow_t * f2 = create_flow(1, 1, 99, 1, 0);
    add_flow(flowlist, f1);
    add_flow(flowlist, f2);
    buffer_put(nodes[20]->active_flows, f1);
    buffer_put(nodes[99]->active_flows, f2);
    printf("Flows initialized\n");
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
    //initialize_flows();
    
    work_per_timeslot();

    free_flows();
    free_network();

    printf("Finished execution\n");

    return 0;
}