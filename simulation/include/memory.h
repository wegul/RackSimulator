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
    int32_t capacity; // maximum number of values that can be held
    int32_t count; // number of values held currently
    lru_node_t * head; // head node of linked list
} sram_t;

// Direct-mapped SRAM
typedef struct dm_sram {
    int32_t capacity;
    int64_t * flow_ids;
    int64_t * memory;
} dm_sram_t;

// LFU SRAM
typedef struct lfu_node {
    int64_t flow_id;
    int64_t val;
    int frequency;
} lfu_node_t;

typedef struct lfu_sram {
    int32_t capacity;
    int32_t count;
    lfu_node_t ** cache;
} lfu_sram_t;

// ARC SRAM
typedef struct arc_node {
    int64_t flow_id;
    int64_t val;
    int frequency;
} arc_node_t;

typedef struct arc_sram {
    int32_t capacity;
    int32_t l1_count;
    int32_t l2_count;
    arc_node_t ** l1_cache;
    arc_node_t ** l2_cache;
} arc_sram_t;

// S3-FIFO SRAM
typedef struct s3f_node {
    int64_t flow_id;
    int64_t val;
    int frequency;
} s3f_node_t;

typedef struct s3f_queue {
    int head;
    int tail;
    int size;
    s3f_node_t ** data;
} s3f_queue_t;

typedef struct s3f_sram {
    int32_t capacity; // Total max capacity of SRAM
    s3f_queue_t * s_fifo; // Small (S) FIFO queue
    s3f_queue_t * m_fifo; // Main (M) FIFO queue
    s3f_queue_t * g_fifo; // Ghost (G) FIFO queue
} s3f_sram_t;

typedef struct dram {
    int32_t delay;
    int32_t accesses;
    int32_t capacity;
    int64_t * memory;
    int * accessible;

    int lock; // 0: free, 1+: locked
    int64_t accessing; // flow being currently accessed while locked
    int placement_idx; // index at which flow being accessed is placed
} dram_t;

lru_node_t * create_lru_node(int64_t flow_id, int64_t val);
sram_t * create_sram(int32_t size, int16_t initialize);
lfu_node_t * create_lfu_node(int64_t flow_id, int64_t val);
lfu_sram_t * create_lfu_sram(int32_t size, int16_t initialize);
arc_node_t * create_arc_node(int64_t flow_id, int64_t val);
arc_sram_t * create_arc_sram(int32_t size, int16_t initialize);
s3f_node_t * create_s3f_node(int64_t flow_id, int64_t val);
s3f_queue_t * create_s3f_queue(int size);
s3f_sram_t * create_s3f_sram(int32_t size, int16_t initialize);
dm_sram_t * create_dm_sram(int32_t size, int16_t initialize);
dram_t * create_dram(int32_t size, int32_t delay);

void push(lru_node_t ** head_ptr, lru_node_t * node);
lru_node_t * pop(lru_node_t ** head_pt);
lru_node_t * remove_node(lru_node_t ** head_ptr, int64_t flow_id);
lru_node_t * remove_node_return_index(lru_node_t ** head_ptr, int64_t flow_id, int * idx);

void insert_L1_arc_node(arc_sram_t * sram, arc_node_t * node, int index);
void insert_L2_arc_node(arc_sram_t * sram, arc_node_t * node, int index);
arc_node_t * remove_L1_arc_node(arc_sram_t * sram, int index);
arc_node_t * remove_L2_arc_node(arc_sram_t * sram, int index);
void reinsert_L1_arc_node(arc_sram_t * sram, int index);
void reinsert_L2_arc_node(arc_sram_t * sram, int index);

void push_s3f_node(s3f_queue_t * fifo, s3f_node_t * node);
s3f_node_t * pop_s3f_node(s3f_queue_t * fifo);

void initialize_sram(sram_t * sram);
void initialize_lfu_sram(lfu_sram_t * sram);
void initialize_arc_sram(arc_sram_t * sram);
void initialize_s3f_sram(s3f_sram_t * sram);
void initialize_dm_sram(dm_sram_t * dm_sram);

int64_t evict_from_sram(sram_t * sram, dram_t * dram);
int64_t evict_from_lfu_sram(lfu_sram_t * sram, dram_t * dram);
int64_t evict_from_arc_sram(arc_sram_t * sram, dram_t * dram);
int64_t evict_from_s3f_sram_s(s3f_sram_t * sram, dram_t * dram);
int64_t evict_from_s3f_sram_m(s3f_sram_t * sram, dram_t * dram);
int64_t pull_from_dram(sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t pull_from_dram_lfu(lfu_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t pull_from_dram_arc(arc_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t pull_from_dram_s3f(s3f_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t access_sram(sram_t * sram, int64_t flow_id);
int64_t access_sram_return_index(sram_t * sram, int64_t flow_id);
int64_t access_lfu_sram(lfu_sram_t * sram, int64_t flow_id);
int64_t access_arc_sram(arc_sram_t * sram, int64_t flow_id);
int64_t access_s3f_sram(s3f_sram_t * sram, int64_t flow_id);

int64_t evict_from_dm_sram(dm_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t dm_pull_from_dram(dm_sram_t * sram, dram_t * dram, int64_t flow_id);
int64_t access_dm_sram(dm_sram_t * sram, int64_t flow_id);

int64_t reorganize_sram(sram_t * sram, buffer_t * buffer);

void print_sram(sram_t * sram);
void print_lfu_sram(lfu_sram_t * sram);
void print_arc_sram(arc_sram_t * sram);
void print_dm_sram(dm_sram_t * sram);

int64_t belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len);
int64_t evict_belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len);

void free_sram(sram_t * sram);
void free_lfu_sram(lfu_sram_t * sram);
void free_arc_sram(arc_sram_t * sram);
void free_dm_sram(dm_sram_t * sram);
void free_dram(dram_t * dram);

#endif