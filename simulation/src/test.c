#include "../include/driver.h"

// Default values for simulation
static int pkt_size = BLK_SIZE;    // in bytes
static float link_bandwidth = 100; // in Gbps
static float timeslot_len;         // in ns
static int bytes_per_timeslot = 8;
int total_grant = 0;
float per_hop_propagation_delay_in_ns = 10;
int per_hop_propagation_delay_in_timeslots;
float per_sw_delay_in_ns = 0;
int per_sw_delay_in_timeslots;

volatile int64_t curr_timeslot = 0; // extern var
int packet_counter = 0;

int burst_size = 1; // = 64Byte Number of blocks to send in a burst

int64_t total_bytes_rcvd = 0;
int64_t total_pkts_rcvd = 0;
float avg_flow_completion_time = 0;
float avg_mem_queue_len[NODES_PER_RACK] = {0}, max_mem_queue_len[NODES_PER_RACK] = {0}, avg_net_queue_len[NODES_PER_RACK] = {0}, max_net_queue_len[NODES_PER_RACK] = {0};

static volatile int8_t terminate0 = 0;
static volatile int8_t terminate1 = 0;

volatile int64_t num_of_flows_finished = 0; // extern var
int CHUNK_SIZE = 256;
volatile int64_t total_flows_started = 0; // extern var

// Output files
FILE *out_fp = NULL;
FILE *sw_queue_fp = NULL;
FILE *tor_outfiles[NUM_OF_RACKS];
FILE *host_outfiles[NUM_OF_NODES];

// Network
node_t *nodes;
tor_t *tors;
links_t links;
flowlist_t *flowlist;
int resp2req[MAX_FLOW_ID] = {0};
int req2resp[MAX_FLOW_ID] = {0};

tor_t *tors;
int cmp_remaining_size(const void *a, const void *b)
{
    packet_t pkt1 = *(packet_t*)a;
    packet_t pkt2 = *(packet_t*)b;
    if (pkt1->pkt_id < pkt2->pkt_id)
    {
        return -1;
    }
    else if (pkt1->pkt_id > pkt2->pkt_id)
        return 1;
    else
        return 0;
}
int main()
{
    tors = (tor_t *)malloc(NUM_OF_TORS * sizeof(tor_t));
    MALLOC_TEST(tors, __LINE__);
    for (int i = 0; i < NUM_OF_TORS; ++i)
    {
        tors[i] = create_tor(i);
    }
    tor_t tor = tors[0];
    packet_t pkt = NULL;
    for (int i = 0; i < 6; i++)
    {
        pkt = create_packet(0, 0, 0 /*id*/, 1 /*size*/, 11 /*seq*/, 10 - i);
        int drop = pkt_recv(tor->downstream_send_buffer[0], pkt);
        printf("drop: %d, num: %d, id= %d\n", drop, tor->downstream_send_buffer[0]->num_elements, pkt->pkt_id);
        qsort(tor->downstream_send_buffer[0]->buffer, tor->downstream_send_buffer[0]->num_elements, sizeof(void *), cmp_remaining_size);
    }
    for (int j = 0; j < tor->downstream_send_buffer[0]->num_elements; j++)
    {
        packet_t tmp = (packet_t)buffer_peek(tor->downstream_send_buffer[0], j);
        printf("%d\n", tmp->pkt_id);
    }
}
