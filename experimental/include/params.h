#ifndef __PARAMS_H__
#define __PARAMS_H__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#define NULL_TEST(ptr, line_num) \
    if (ptr == NULL) \
        {printf("error: %s:%d Null pointer exception\n", __FILE__, line_num); \
        exit(0);}

#define MALLOC_TEST(ptr, line_num) \
    if (ptr == NULL) \
        {printf("error: %s:%d malloc() failed\n", __FILE__, line_num); \
        exit(0);}

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define OVERSUBSCRIPTION 1
#define NUM_OF_RACKS 32
#define NODES_PER_RACK 16
#define NUM_OF_NODES (NUM_OF_RACKS * NODES_PER_RACK)
#define NUM_OF_SPINES (NODES_PER_RACK/OVERSUBSCRIPTION)
#define SPINE_PORT_COUNT NUM_OF_RACKS
#define NUM_OF_TORS NUM_OF_RACKS
#define TOR_PORT_COUNT_LOW NODES_PER_RACK
#define TOR_PORT_COUNT_UP NUM_OF_SPINES

#define SPINE_PORT_BUFFER_LEN (4 * NODES_PER_RACK)
#define TOR_UPSTREAM_BUFFER_LEN (3 * NODES_PER_RACK)
#define TOR_DOWNSTREAM_BUFFER_LEN (5 * NODES_PER_RACK)

#define LINK_CAPACITY 50

#define MAX_FLOW_ID 1000

extern int16_t*** source_list;
extern int16_t epoch_len;
extern volatile int64_t curr_timeslot;
extern volatile int64_t curr_epoch;
extern int8_t flow_trace_scanned_completely;
extern char* ptr;
extern volatile int64_t total_flows_started;
extern int64_t num_of_pkt_injected;
extern volatile int64_t num_of_flows_finished;

#endif
