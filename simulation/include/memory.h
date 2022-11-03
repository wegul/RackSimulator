#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "params.h"

typedef struct lru_node {
    int64_t flow_id;
    int64_t val;
    struct lru_node * next;
    struct lru_node * prev;
} lru_node_t;

typedef struct sram {
    int32_t capacity;
    int32_t count;
    lru_node_t * head;
    lru_node_t * tail;
} sram_t;

typedef struct dram {
    int32_t delay;
    int32_t capacity;
    int64_t * memory;
} dram_t;

lru_node_t * create_lru_node(int64_t flow_id, int64_t val);
sram_t * create_sram(int32_t size);
dram_t * create_dram(int32_t size, int32_t delay);

void initialize_sram(sram_t * sram);
int64_t evict_from_sram(sram_t * sram, dram_t * dram);
int64_t pull_from_dram(sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t access_sram(sram_t * sram, int64_t flow_id);

void free_sram(sram_t * sram);
void free_dram(dram_t * dram);

#endif