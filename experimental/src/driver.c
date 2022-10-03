#include "params.h"
#include "arraylist.h"
#include "buffer.h"
#include "flow.h"
#include "flowlist.h"
#include "link.h"
#include "packet.h"
#include "node.h"
#include "tor.h"
#include "spine.h"
#include "links.h"

// Default values for simulation
static int pkt_size = 64; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len; //in ns

int16_t header_overhead = 0;
float per_hop_propagation_delay_in_ns = 0;
int per_hop_propagation_delay_in_timeslots;
volatile int64_t curr_timeslot = 0; //extern var

static int8_t start_logging = 0;
static int8_t static_workload = 1;

static volatile int8_t terminate = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 500000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 500000; //stop after these many flows start

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
                flow_t * flow = buffer_get(node->active_flows);
                flow->active = 1;
                flow->pkts_sent++;
                int16_t src_node = flow->src;
                int16_t dst_node = flow->dst;
                int64_t flow_id = flow->flow_id;
                int64_t seq_num = node->seq_num[dst_node];
                packet_t pkt = create_packet(src_node, dst_node, flow_id, seq_num);
                //node->seq_num[dst_node]++;
                
                if (flow->pkts_sent < flow->flow_size) {
                    buffer_put(node->active_flows, flow);
                }
                else {
                    flow->active = 0;
                }
                
                pkt->time_when_transmitted_from_src = curr_timeslot;

                int16_t dst_tor = node_index / NODES_PER_RACK;
                pkt->time_to_dequeue_from_link = curr_timeslot +
                    per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
                total_flows_started++;
                printf("host %d created pkt\n", i);
                print_packet(pkt);
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
                pkt = (packet_t)
                    link_dequeue(links->tor_to_host_link[src_tor][node_index]);
                printf("host %d received packet from ToR %d\n", node_index, src_tor);
                free_packet(pkt);
            }
        }
        curr_timeslot++;
        if (curr_timeslot > 5) {
            break;
        }
    }
}

// /*---------------------------------------------------------------------------*/
//                 //Data logging and state updates before next iteration
// /*---------------------------------------------------------------------------*/

//         //checking if there are any active flows
//         int16_t finished = 1;

//         for (int i = 0; i < NUM_OF_NODES; ++i) {
//             if (nodes[i]->num_of_active_host_flows > 0) {
//                 finished = 0;
//                 break;
//             }
//         }

//         if (static_workload == 1) {
//             if (time_to_start_logging == INT64_MAX) {
//                 time_to_start_logging = curr_timeslot;
//             }

//             if (curr_timeslot == time_to_start_logging) {
//                 start_logging = 1;
//                 printf("Started logging at epoch = %ld\n", curr_epoch);
//                 fflush(stdout);
//             }
//         }

//         if (finished && flow_trace_scanned_completely) terminate = 1;

//         //stop after certain number of flows have finished
//         if (num_of_flows_finished >= num_of_flows_to_finish) terminate1 = 1;
        
//         //printf("numx %d\n", num_of_flows_finished);
//         //printf("num %d\n", num_of_flows_to_finish);

//         //stop after certain number of flows have started
//         if (total_flows_started >= num_of_flows_to_start) terminate2 = 1;

//         ++curr_timeslot;
//         if (curr_timeslot % epoch_len == 0) {
//             ++curr_epoch;
//             if (curr_epoch % 10 == 0) {
//                 int count = 0;
//                 int count1 = 0;
//                 for (int j = 0; j < NUM_OF_NODES; ++j) {
//                     count += nodes[j]->num_of_active_host_flows;
//                 }
//                 for (int j = 0; j < NUM_OF_NODES; ++j) {
//                     count1 += nodes[j]->num_of_active_network_host_flows;
//                 }
//                 print_network_tput(pkt_size, header_overhead, timeslot_len);
//             }
//         }

//         read_from_tracefile();

//         if (run_till_max_epoch
//         && curr_timeslot >= (max_epochs_to_run * epoch_len)) {
//             terminate3 = 1;
//         }

//         //Terminate experiment
//         if (terminate || terminate1 || terminate2 || terminate3) {
//             return;
//         }
//     }
// }

void read_tracefile() {
    return;
}

void initialize_flows() {
    flowlist = create_flowlist();
    flow_t * f1 = create_flow(0, 1, 20, 65);
    flow_t * f2 = create_flow(1, 1, 99, 1);
    add_flow(flowlist, f1);
    add_flow(flowlist, f2);
    buffer_put(nodes[20]->active_flows, f1);
    buffer_put(nodes[99]->active_flows, f2);
    printf("Flows initialized\n");
}

void free_flows() {
    free_flowlist(flowlist);
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
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    read_tracefile();

    initialize_network();
    initialize_flows();
    
    work_per_timeslot();

    free_flows();
    free_network();

    printf("end\n");

    return 0;
}