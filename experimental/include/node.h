#ifndef __NODE_H__
#define __NODE_H__

#include "params.h"
#include "arraylist.h"
#include "buffer.h"

/* struct flow_stats {
    int64_t flow_id;
    int16_t src;
    int16_t dst;
    int64_t flow_size_bytes;
    int64_t flow_size;
};

typedef struct flow_stats* flow_stats_t; */

/* typedef struct stats {
    int64_t host_pkt_received;
    int64_t dummy_pkt_received;
    int64_t total_time_active;
    arraylist_t flow_stat_list;
} stats_t;
 */
struct node {
    int16_t node_index;

   /*  //for keeping track of experiment progress
    int64_t num_of_active_host_flows;
    int64_t num_of_active_network_host_flows;
    int64_t curr_num_of_sending_nodes;

    stats_t stat;
*/
    int64_t seq_num[NUM_OF_NODES]; //to be put into the packets 
    int64_t curr_seq_num[NUM_OF_NODES];
};

typedef struct node* node_t;

extern node_t* nodes;

node_t create_node(int16_t);
void free_node(node_t);

#endif
