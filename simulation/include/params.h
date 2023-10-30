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
#include <dirent.h>

#define NULL_TEST(ptr, line_num)                                             \
    if (ptr == NULL)                                                         \
    {                                                                        \
        printf("error: %s:%d Null pointer exception\n", __FILE__, line_num); \
        exit(0);                                                             \
    }

#define MALLOC_TEST(ptr, line_num)                                    \
    if (ptr == NULL)                                                  \
    {                                                                 \
        printf("error: %s:%d malloc() failed\n", __FILE__, line_num); \
        exit(0);                                                      \
    }

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define OVERSUBSCRIPTION 1
#define NUM_OF_RACKS 1
#define NODES_PER_RACK 64
#define NUM_OF_NODES (NUM_OF_RACKS * NODES_PER_RACK)

// #define NUM_OF_SPINES (NODES_PER_RACK / OVERSUBSCRIPTION)
#define NUM_OF_SPINES 0
// #define SPINE_PORT_COUNT NUM_OF_RACKS
#define SPINE_PORT_COUNT 0
#define NUM_OF_TORS NUM_OF_RACKS
#define TOR_PORT_COUNT_LOW NODES_PER_RACK
// #define TOR_PORT_COUNT_UP NUM_OF_SPINES
#define TOR_PORT_COUNT_UP 0

// Queue Size
#define TOR_UPSTREAM_BUFFER_LEN 50000
#define TOR_DOWNSTREAM_BUFFER_LEN 50000
#define TOR_UPSTREAM_MEMBUF_LEN 8192
#define TOR_DOWNSTREAM_MEMBUF_LEN 8192

#define LINK_CAPACITY 1024

#define MAX_FLOW_ID 8192
#define RTABLE_SIZE MAX_FLOW_ID
#define CC_BOUND 100 // CONGESTION thresholds
#define ECN_CUTOFF_TOR_UP 40000
#define ECN_CUTOFF_TOR_DOWN 40000
#define ECN_WIDTH 1500
#define TIMEOUT 4000
#define BLK_SIZE 8
#define SSTHRESH_START 8192
#define INTERPACKET_GAP 96

extern int16_t ***source_list;
extern int16_t epoch_len;
extern volatile int64_t curr_timeslot;
extern int8_t flow_trace_scanned_completely;
extern char *ptr;
extern volatile int64_t total_flows_started;
extern int64_t num_of_pkt_injected;
extern volatile int64_t num_of_flows_finished;

#endif
