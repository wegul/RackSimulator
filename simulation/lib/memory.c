#include "memory.h"

lru_node_t * create_lru_node(int64_t flow_id, int64_t val) {
    lru_node_t * node = malloc(sizeof(lru_node_t));
    MALLOC_TEST(node, __LINE__);
    node->flow_id = flow_id;
    node->val = val;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

sram_t * create_sram(int32_t size, int16_t initialize) {
    sram_t * sram = malloc(sizeof(sram_t));
    MALLOC_TEST(sram, __LINE__);
    sram->capacity = size;
    sram->count = 0;
    sram->head = NULL;
    sram->tail = NULL;

    if (initialize == 1) {
        initialize_sram(sram);
    }

    return sram;
}

dm_sram_t * create_dm_sram(int32_t size, int16_t initialize) {
    dm_sram_t * sram = malloc(sizeof(dm_sram_t));
    MALLOC_TEST(sram, __LINE__);
    sram->capacity = size;
    sram->flow_ids = malloc(sizeof(int64_t) * size);
    sram->memory = malloc(sizeof(int64_t) * size);
    for (int i = 0; i < size; i++) {
        if (initialize == 1) {
            sram->flow_ids[i] = i;
        }
        else {
            sram->flow_ids[i] = 0;
        }
        sram->memory[i] = 0;
    }

    return sram;
}

dram_t * create_dram(int32_t size, int32_t delay) {
    dram_t * dram = malloc(sizeof(dram_t));
    MALLOC_TEST(dram, __LINE__);
    dram->capacity = size;
    dram->delay = delay;
    dram->memory = malloc(sizeof(int64_t) * size); // Contains value associated with flow_id (i.e. val)
    for (int i = 0; i < size; i++) {
        dram->memory[i] = 0;
    }

    return dram;
}

void initialize_sram(sram_t * sram) {
    for (int i = 0; i < sram->capacity; i++) {
        lru_node_t * node = create_lru_node(i, 0);
        if (sram->head == NULL) {
            sram->head = node;
        }
        if (sram->tail != NULL) {
            sram->tail->next = node;
            node->prev = sram->tail;
        }
        sram->tail = node;
    }
    sram->count = sram->capacity;
}

int64_t evict_from_sram(sram_t * sram, dram_t * dram) {
    lru_node_t * tail = sram->tail;
    if (tail != NULL) {
        sram->tail = tail->prev;
        if (sram->tail != NULL) {
            sram->tail->next = NULL;
        }
        int64_t flow_id = tail->flow_id;
        int64_t val = tail->val;
        free(tail);
        sram->count--;
        dram->memory[flow_id] = val;
        return flow_id;
    }
    else {
        // Nothing to evict
        return -1;
    }
}

int64_t pull_from_dram(sram_t * sram, dram_t * dram, int64_t flow_id) {
    lru_node_t * node = create_lru_node(flow_id, dram->memory[flow_id]);
    if (sram->head != NULL) {
        sram->head->prev = node;
        node->next = sram->head;
    }
    if (sram->tail == NULL) {
        sram->tail = node;
    }
    sram->head = node;
    sram->count++;
    if (sram->count > sram->capacity) {
        evict_from_sram(sram, dram);
    }
    return node->val;
}

int64_t access_sram(sram_t * sram, int64_t flow_id) {
    lru_node_t * node = sram->head;
    while (node != NULL) {
        if (node->flow_id == flow_id) {
            // Cache hit, bring to front
            if (node != sram->head) {
                node->prev->next = node->next;
                if (node != sram->tail) {
                    node->next->prev = node->prev;
                }
                node->prev = NULL;
                node->next = sram->head;
                sram->head->prev = node;
                sram->head = node;
            }
            node->val++;
            return node->val; // SUCCESS!!!
        }
        node = node->next;
    }
    // Cache miss
    return -1; // FAILURE!!!
}

int64_t evict_from_dm_sram(dm_sram_t * sram, dram_t * dram, int64_t flow_id) {
    dram->memory[flow_id] = sram->memory[flow_id % sram->capacity];
    return flow_id;
}

int64_t dm_pull_from_dram(dm_sram_t * sram, dram_t * dram, int64_t flow_id) {
    evict_from_sram(sram, dram, flow_id);
    sram->flow_ids[flow_id % sram->capacity] = flow_id;
    sram->memory[flow_id % sram->capacity] = dram->memory[flow_id];
    return sram->memory[flow_id % sram->capacity];
}

int64_t access_dm_sram(dm_sram_t * sram, int64_t flow_id) {
    if (sram->flow_ids[flow_id % sram->capacity] == flow_id) {
        // Cache hit
        sram->memory[flow_id % sram->capacity]++;
        return sram->memory[flow_id % sram->capacity]; // SUCCESS!!!
    }
    // Cache miss
    return -1; // FAILURE!!!
}

void free_sram(sram_t * sram) {
    lru_node_t * head = sram->head;
    while (head != NULL) {
        //printf("free sram node %d\n", (int) head->flow_id);
        lru_node_t * next = head->next;
        free(head);
        head = next;
    }
    free(sram);
}

void free_dm_sram(dm_sram_t * sram) {
    free(sram->flow_ids);
    free(sram->memory);
    free(sram);
}

void free_dram(dram_t * dram) {
    free(dram->memory);
    free(dram);
}