#ifndef __ROUTING_TABLE_H__
#define __ROUTING_TABLE_H__

#include <stdlib.h>

#define RTABLE_SIZE 100

typedef struct rnode {
    int addr;
    int flow_id;
    int port;
} rnode_t;

int create_routing_table(rnode_t * routing_table);
int hash(rnode_t * routing_table, int key);

#endif