#include "routing_table.h"
#include <stdio.h>

int create_routing_table(rnode_t * routing_table) {
    int i;
    for (i = 0; i < RTABLE_SIZE; i++) {
        routing_table[i].port = -1;
    }
    return i;
}

int hash(int key) {
    return key % RTABLE_SIZE;
}
