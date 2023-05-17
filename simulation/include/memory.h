#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "params.h"
#include "buffer.h"

typedef struct lru_node {
    int64_t flow_id;
    int64_t val;
    struct lru_node * next;
} lru_node_t;

// Fully-associative SRAM
typedef struct sram {
    int32_t capacity;
    int32_t count;
    lru_node_t * head;
} sram_t;

// Direct-mapped SRAM
typedef struct dm_sram {
    int32_t capacity;
    int64_t * flow_ids;
    int64_t * memory;
} dm_sram_t;

typedef struct dram {
    int32_t delay;
    int32_t accesses;
    int32_t capacity;
    int64_t * memory;
    int * accessible;
} dram_t;

lru_node_t * create_lru_node(int64_t flow_id, int64_t val);
sram_t * create_sram(int32_t size, int16_t initialize);
dm_sram_t * create_dm_sram(int32_t size, int16_t initialize);
dram_t * create_dram(int32_t size, int32_t delay);

void push(lru_node_t ** head_ptr, lru_node_t * node);
lru_node_t * pop(lru_node_t ** head_pt);
lru_node_t * remove_node(lru_node_t ** head_ptr, int64_t flow_id);

void initialize_sram(sram_t * sram);
void initialize_dm_sram(dm_sram_t * dm_sram);

int64_t evict_from_sram(sram_t * sram, dram_t * dram);
int64_t pull_from_dram(sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t access_sram(sram_t * sram, int64_t flow_id);
int64_t access_sram_no_fresh(sram_t * sram, int64_t flow_id);

int64_t evict_from_dm_sram(dm_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t dm_pull_from_dram(dm_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t access_dm_sram(dm_sram_t * sram, int64_t flow_id);

int64_t reorganize_sram(sram_t * sram, buffer_t * buffer);
int64_t belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len);
int64_t evict_belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len);

void print_sram(sram_t * sram);
void print_dm_sram(dm_sram_t * sram);

void free_sram(sram_t * sram);
void free_dm_sram(dm_sram_t * sram);
void free_dram(dram_t * dram);

#endif