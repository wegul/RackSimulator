#include "params.h"
#include "arraylist.h"
#include "bounded_buffer.h"
#include "min_priority_queue.h"
#include "link.h"
#include "flow.h"
#include "packet.h"
#include "node.h"
#include "tor.h"
#include "spine.h"
#include "links.h"
#include "flow_patterns.h"
#include "host_flow_queue_add_remove.h"
#include "prepare_host_packet.h"
#include "system_stats.h"

// Default values for simulation
static int pkt_size = 64; //in bytes
static float link_bandwidth = 100; //in Gbps
static float timeslot_len; //in ns

int16_t*** source_list; //extern var
int16_t epoch_len = NODES_PER_RACK; //extern var
int16_t header_overhead = 0;
float per_hop_propagation_delay_in_ns = 0;
int per_hop_propagation_delay_in_timeslots;

node_t* nodes;
tor_t* tors;
spine_t* spines;
links_t links;
volatile int64_t curr_timeslot = 0; //extern var
volatile int64_t curr_epoch = 0; //extern var

static int64_t time_to_start_logging = INT64_MAX;
static int8_t start_logging = 0;
static int8_t static_workload = 1;

static int32_t max_epochs_to_run=2500;

static volatile int8_t terminate = 0;
static volatile int8_t terminate1 = 0;
static volatile int8_t terminate2 = 0;
static volatile int8_t terminate3 = 0;

volatile int64_t num_of_flows_finished = 0; //extern var
int64_t num_of_flows_to_finish = 500000; //stop after these many flows finish

volatile int64_t total_flows_started = 0; //extern var
int64_t num_of_flows_to_start = 500000; //stop after these many flows start

//needed for tracefile workloads
char* ptr; //points to the mmaped trace file
int8_t flow_trace_scanned_completely = 0; //extern var
int64_t num_of_pkt_injected = 0; //extern var

static FILE* out; //all the stats are logged in this file

static int8_t run_till_max_epoch = 0;

add_host_flows_t add_host_flows;

static inline void initialize_source_list()
{
    source_list = (int16_t***) malloc(NUM_OF_SPINES*sizeof(int16_t**));
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        source_list[i] = (int16_t**) malloc(NODES_PER_RACK*sizeof(int16_t*));
        for (int j = 0; j < NODES_PER_RACK; ++j) {
            source_list[i][j] = (int16_t*) malloc(SPINE_PORT_COUNT*sizeof(int16_t));
        }
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NODES_PER_RACK; ++j) {
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                source_list[i][j][k] = (k * NODES_PER_RACK) + j;
            }
        }
    }
}

static inline void randomize_source_list()
{
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
		for (int j = NODES_PER_RACK-1; j >= 0; --j) {
			int x = rand() % (j + 1);

			int16_t* temp = source_list[i][j];
			source_list[i][j] = source_list[i][x];
			source_list[i][x] = temp;
		}
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NODES_PER_RACK; ++j) {
            for (int k = SPINE_PORT_COUNT-1; k >= 0; --k) {
                int x = rand() % (k + 1);

                int16_t temp = source_list[i][j][k];
                source_list[i][j][k] = source_list[i][j][x];
                source_list[i][j][x] = temp;
            }
        }
    }
}

static inline void free_source_list()
{
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NODES_PER_RACK; ++j) {
            free(source_list[i][j]);
        }
        free(source_list[i]);
    }
    free(source_list);
}

void work_per_timeslot()
{
    while (1) {

        if (curr_timeslot % epoch_len == 0) {

            randomize_source_list();

#ifdef DEBUG_RANDOM_SOURCE_LIST
        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            for (int j = 0; j < NODES_PER_RACK; ++j) {
                for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                    printf("%d ", source_list[i][j][k]);
                }
                printf("\n");
            }
            printf("\n\n");
        }
        exit(0);
#endif
        }

/*---------------------------------------------------------------------------*/
                                  //SPINE -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            spine_t spine = spines[i];
            int16_t spine_index = spine->spine_index;

            //record spine port queue lengths
            for (int j = 0; j < SPINE_PORT_COUNT; ++j) {
                int16_t offset = j * TOR_PORT_COUNT_LOW;
                int32_t size = 0;
                for (int k = 0; k < TOR_PORT_COUNT_LOW; ++k) {
                    size += bounded_buffer_num_of_elements
                        (spine->pkt_buffer[offset + k]);
                }
                assert(size <= SPINE_PORT_BUFFER_LEN);
                ++(spine->queue_stat.queue_len_histogram[size]);
            }

            //re-initialize schedulable to 1 at the start of every epoch
            if (curr_timeslot % epoch_len == 0) {
                for (int16_t i = 0; i < NUM_OF_NODES; ++i) {
                    spine->src_schedulable[i] = OVERSUBSCRIPTION;
                    spine->dst_schedulable[i] = OVERSUBSCRIPTION;
                }
                for (int16_t i = 0; i < NUM_OF_TORS; ++i) {
                    spine->tor_src_schedulable[i] = epoch_len;
                    spine->tor_dst_schedulable[i] = epoch_len;
                }
            }

            //send the packet
            int j = curr_timeslot % NODES_PER_RACK;
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                int16_t dst_node = source_list[spine_index][j][k];
                int16_t dst_tor = dst_node / NODES_PER_RACK;
                packet_t pkt = pkt_to_send_from_spine_port
                    (spine, dst_node, dst_tor);
                pkt->time_to_dequeue_from_link = curr_timeslot +
                    per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->spine_to_tor_link[spine_index][dst_tor], pkt);
            }
        }

/*---------------------------------------------------------------------------*/
                                  //HOST -- SEND
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            node_t node = nodes[i];
            int16_t node_index = node->node_index;

            add_host_flows(node);

            packet_t pkt = get_host_packet(node);

#ifdef DEBUG_DRIVER
            if (pkt->src_node == 228 && pkt->dst_node == 140) {
                printf("%ld/%ld: (%d->%d) PKT SENT\n",
                    curr_timeslot, curr_epoch, pkt->src_node, pkt->dst_node);
                fflush(stdout);
            }
#endif

            //enq packet
            int16_t dst_tor = node_index / NODES_PER_RACK;
            pkt->time_to_dequeue_from_link = curr_timeslot +
                per_hop_propagation_delay_in_timeslots;
            link_enqueue(links->host_to_tor_link[node_index][dst_tor], pkt);
        }

/*---------------------------------------------------------------------------*/
                          //ToR -- SEND TO HOST AND SPINE
/*---------------------------------------------------------------------------*/

        for (int i = 0; i < NUM_OF_TORS; ++i) {
            tor_t tor = tors[i];
            int16_t tor_index = tor->tor_index;

            //record upstream port queue lengths
            for (int j = 0; j < NUM_OF_SPINES; ++j) {
                int32_t size = bounded_buffer_num_of_elements
                        (tor->upstream_pkt_buffer[j]);
                ++(tor->queue_stat.upstream_queue_len_histogram[size]);
            }

            //record downstream port queue lengths
            for (int j = 0; j < NODES_PER_RACK; ++j) {
                int32_t size = bounded_buffer_num_of_elements
                        (tor->downstream_pkt_buffer[j]);
                ++(tor->queue_stat.downstream_queue_len_histogram[size]);
            }

            //send to each spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                packet_t pkt = send_to_spine(tor, tor_port);
                assert(pkt != NULL);
                pkt->time_to_dequeue_from_link = curr_timeslot +
                    per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->tor_to_spine_link[tor_index][tor_port], pkt);
            }

            //send to each host
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_LOW; ++tor_port) {
                packet_t pkt = send_to_host(tor, tor_port);
                assert(pkt != NULL);
                int16_t dst_host = (tor_index * TOR_PORT_COUNT_LOW) + tor_port;
                pkt->time_to_dequeue_from_link = curr_timeslot +
                    per_hop_propagation_delay_in_timeslots;
                link_enqueue(links->tor_to_host_link[tor_index][dst_host],pkt);
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
                assert(pkt != NULL);
                if (pkt->time_to_dequeue_from_link == curr_timeslot) {
                    pkt = (packet_t)
                        link_dequeue(links->host_to_tor_link[src_host][tor_index]);
                    extract_flow_notification(tor, pkt);
                }
            }

            //Recv packet from spine
            for (int tor_port = 0; tor_port < TOR_PORT_COUNT_UP; ++tor_port) {
                //deq packet from the link
                int16_t src_spine = tor_port;
                packet_t pkt = (packet_t)
                    link_peek(links->spine_to_tor_link[src_spine][tor_index]);
                assert(pkt != NULL);
                if (pkt->time_to_dequeue_from_link == curr_timeslot) {
                    pkt = (packet_t)
                        link_dequeue(links->spine_to_tor_link[src_spine][tor_index]);
                    extract_pull_req(tor, pkt);
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
                assert(pkt != NULL);
                if (pkt->time_to_dequeue_from_link == curr_timeslot) {
                    pkt = (packet_t) link_dequeue
                        (links->tor_to_spine_link[src_tor][spine_index]);

                    spine_ingress_processing(spine, pkt);

                    //enq packet in the virtual queue
                    int16_t dst_host = pkt->dst_node;
                    if (dst_host != -1) {
                        assert(pkt->data_spine_id == spine_index);
                        pkt->time_when_added_to_spine_queue = curr_epoch;
                        assert(bounded_buffer_put(spine->pkt_buffer[dst_host], pkt)
                                != -1);
                        clear_protocol_fields(pkt);
                        //if first pkt in virtual q, insert in rr queue
                        if (bounded_buffer_num_of_elements
                            (spine->pkt_buffer[dst_host]) == 1) {
                            int16_t dst_spine_port = dst_host / NODES_PER_RACK;
                            assert(bounded_buffer_num_of_elements
                                (spine->dst_to_send_rr[dst_spine_port])
                                    < NODES_PER_RACK);
                            int16_t* x = (int16_t*) malloc(sizeof(int16_t));
                            *x = dst_host;
                            assert(bounded_buffer_put
                                (spine->dst_to_send_rr[dst_spine_port], x) != -1);
                        }

                        if (dst_host == -1) {
                            assert(pkt->data_spine_id == -1);
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
            assert(pkt != NULL);
            if (pkt->time_to_dequeue_from_link == curr_timeslot) {
                pkt = (packet_t)
                    link_dequeue(links->tor_to_host_link[src_tor][node_index]);

                assert(pkt->dst_node == -1 || pkt->dst_node == node_index);

                update_pkt_counters(node, pkt);

                assert(pkt->data_spine_id == -1);

                if (pkt->notf_spine_id != -1) {
                    assert(pkt->new_flow_dst_id[0] == node_index);
                    assert(pkt->new_flow_src_id[0] != -1);
                    recvd_pull_request_t recvd_pull_req[OVERSUBSCRIPTION];
                    for (int k = 0; k < OVERSUBSCRIPTION; ++k) {
                        recvd_pull_req[k] = (recvd_pull_request_t)
                            malloc(sizeof(struct recvd_pull_request));
                        recvd_pull_req[k]->spine_id = pkt->notf_spine_id;
                        recvd_pull_req[k]->destination_id = pkt->new_flow_src_id[0];
                        assert(bounded_buffer_num_of_elements
                            (node->recvd_pull_req_queue)<RECVD_PULL_REQ_QUEUE_LEN);
                        assert(bounded_buffer_put
                            (node->recvd_pull_req_queue, recvd_pull_req[k]) != -1);
                    }
                } else {
                    assert(pkt->new_flow_dst_id[0] == -1);
                    assert(pkt->new_flow_src_id[0] == -1);
                }

                //update flow stats
                if (pkt->dst_node == node_index) {
#ifdef DEBUG_DRIVER
                    if (pkt->src_node == 10 && pkt->dst_node == 30) {
                        printf("%ld/%ld: (%d->%d) PKT RECVD\n",
                            curr_timeslot, curr_epoch, pkt->src_node, node_index);
                        fflush(stdout);
                    }
#endif
                    update_stats_on_pkt_recv
                        (node, pkt, static_workload, start_logging);
                }

                free_packet(pkt);
            }
        }

/*---------------------------------------------------------------------------*/
                //Data logging and state updates before next iteration
/*---------------------------------------------------------------------------*/

        //checking if there are any active flows
        int16_t finished = 1;

        for (int i = 0; i < NUM_OF_NODES; ++i) {
            if (nodes[i]->num_of_active_host_flows > 0) {
                finished = 0;
                break;
            }
        }

        if (static_workload == 1) {
            //if (flow_trace_scanned_completely
            //    && time_to_start_logging == INT64_MAX) {
            //    time_to_start_logging = curr_timeslot + (10 * epoch_len);
            //}
            if (time_to_start_logging == INT64_MAX) {
                time_to_start_logging = curr_timeslot;
            }

            if (curr_timeslot == time_to_start_logging) {
                start_logging = 1;
                printf("Started logging at epoch = %ld\n", curr_epoch);
                fflush(stdout);
            }
        }

        if (finished && flow_trace_scanned_completely) terminate = 1;

        //stop after certain number of flows have finished
        if (num_of_flows_finished >= num_of_flows_to_finish) terminate1 = 1;

        //stop after certain number of flows have started
        if (total_flows_started >= num_of_flows_to_start) terminate2 = 1;

        ++curr_timeslot;
        if (curr_timeslot % epoch_len == 0) {
            ++curr_epoch;
            if (curr_epoch % 10 == 0) {
                int count = 0;
                int count1 = 0;
                for (int j = 0; j < NUM_OF_NODES; ++j) {
                    count += nodes[j]->num_of_active_host_flows;
                }
                for (int j = 0; j < NUM_OF_NODES; ++j) {
                    count1 += nodes[j]->num_of_active_network_host_flows;
                }
#ifdef PRINT_STATS
                printf("epoch = %ld active app flows = %d", curr_epoch, count);
                printf(" started flows = %ld", total_flows_started);
                printf(" finished flows = %ld\n", num_of_flows_finished);
                fflush(stdout);
#endif

                print_network_tput(pkt_size, header_overhead, timeslot_len);
            }
        }

        read_from_tracefile();

        if (run_till_max_epoch
        && curr_timeslot >= (max_epochs_to_run * epoch_len)) {
            terminate3 = 1;
        }

        //Terminate experiment
        if (terminate || terminate1 || terminate2 || terminate3) {
            return;
        }
    }
}

static inline void time_elapsed(uint32_t sec)
{
    uint32_t hour = sec/3600;
    uint32_t min = (sec % 3600)/60;
    sec = (sec % 3600) % 60;
    printf("\nTime elapsed = %u:%u:%u\n", hour, min, sec);
    fflush(stdout);
}

static inline void usage()
{
    printf("usage: ./<exec> -f <filename>\n");
    printf("filename must be <500 char long\n");
    exit(1);
}

int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    //parse command-line arguments
    int opt;
    char filename[500] = "";
    add_host_flows_t func = NULL;
    char out_filename[500] = "";

    while ((opt = getopt(argc, argv, "f:w:e:b:c:h:d:n:m:")) != -1) {
        switch(opt) {
            case 'w': static_workload = atoi(optarg);
                      if(static_workload == 0){
                         printf("Running a non-static workload file!\n");
                      } else{
                         printf("Running a static workload file!\n");
                      }
                      break;
            case 'c': pkt_size = atoi(optarg);
                      printf("Running with a pkt size of: %dB\n", pkt_size);
                      break;
            case 'b': link_bandwidth = atof(optarg);
                      printf("Running with a link bandwidth of: %fGbps\n",
                              link_bandwidth);
                      break;
            case 'h': header_overhead = atoi(optarg);
                      printf("Header overhead of: %dB\n", header_overhead);
                      break;
            case 'd': per_hop_propagation_delay_in_ns = atof(optarg);
                      break;
            case 'n': num_of_flows_to_finish = atoi(optarg);
                      printf("Stop experiment after %ld flows have finished\n",
                              num_of_flows_to_finish);
                      break;
            case 'm': num_of_flows_to_start = atoi(optarg);
                      printf("Stop experiment after %ld flows have started\n",
                              num_of_flows_to_start);
                      break;
            case 'e': max_epochs_to_run = atoi(optarg);
                      if (max_epochs_to_run != 0) {
                          run_till_max_epoch = 1;
                          printf("\nRunning till max epoch = %d\n",
                                  max_epochs_to_run);
                      }
                      break;
            case 'f': if (strlen(optarg) < 500)
                          strcpy(filename, optarg);
                      else
                          usage();
                      func = tracefile;
                      flow_trace_scanned_completely = 0;
                      break;
            default: printf("wrong command line argument");
                     exit(1);
        }
    }

    timeslot_len = (pkt_size * 8) / link_bandwidth;
    printf("Running with a slot length of: %fns\n", timeslot_len);

    per_hop_propagation_delay_in_timeslots
        = round((float)per_hop_propagation_delay_in_ns / (float)timeslot_len);
    printf("Per hop propagation delay: %f ns (%d timeslots)\n",
          per_hop_propagation_delay_in_ns, per_hop_propagation_delay_in_timeslots);

    printf("\n");

    int fd;
    struct stat fstat;
    char* saved_ptr;
    if (strcmp(filename, "")) {
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(1);
        } else {
            if (stat(filename, &fstat) == -1) {
                perror("stat");
                exit(1);
            }

            ptr = mmap((caddr_t)0, fstat.st_size, PROT_READ, MAP_SHARED, fd, 0);
            if (ptr == (caddr_t)-1) {
                perror("mmap");
                exit(1);
            }
            saved_ptr = ptr;

            read_from_tracefile();

            strcpy(out_filename, filename);
            strcat(out_filename, ".out");
            out = fopen(out_filename, "w");
        }
    } else {
        usage();
    }

    struct timeval tv1;
    struct timeval tv2;

    gettimeofday(&tv1, 0);

    initialize_source_list();

#ifdef DEBUG_SOURCE_LIST
    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        for (int j = 0; j < NODES_PER_RACK; ++j) {
            for (int k = 0; k < SPINE_PORT_COUNT; ++k) {
                printf("%d ", source_list[i][j][k]);
            }
            printf("\n");
        }
        printf("\n\n");
    }
    exit(0);
#endif

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

    add_host_flows = func;

    work_per_timeslot();

    print_summary_stats(pkt_size,
                    header_overhead,
                    link_bandwidth,
                    timeslot_len,
                    per_hop_propagation_delay_in_timeslots,
                    time_to_start_logging,
                    static_workload,
                    run_till_max_epoch,
                    out);

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

    free_source_list();

    if (munmap(saved_ptr, fstat.st_size) == -1) {
        perror("unmap");
        exit(1);
    }

    close(fd);
    fclose(out);

    printf("\nAll done..\n");

    gettimeofday(&tv2, 0);
    time_elapsed(tv2.tv_sec - tv1.tv_sec);
    printf("\n");

    return 0;
}
