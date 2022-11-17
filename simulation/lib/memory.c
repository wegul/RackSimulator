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

void push(lru_node_t ** head_ptr, lru_node_t ** tail_ptr, lru_node_t * node) {
    node->next = (*head_ptr);
    node->prev = NULL;

    if (*tail_ptr == NULL) {
        (*tail_ptr) = node;
    }
    else {
        (*head_ptr)->prev = node;
    }

    (*head_ptr) = node;
}

lru_node_t * pop(lru_node_t ** head_ptr, lru_node_t ** tail_ptr) {
    if (*head_ptr == *tail_ptr) {
        lru_node_t * node = *tail_ptr;
        (*head_ptr) = NULL;
        (*tail_ptr) = NULL;
        return node;
    }
    lru_node_t * new_tail = NULL;
    if ((*tail_ptr) != NULL) {
        new_tail = (*tail_ptr)->prev;
    }
    new_tail->next = NULL;
    lru_node_t * node = *tail_ptr;
    (*tail_ptr) = new_tail;
    return node;
}

lru_node_t * remove_node(lru_node_t ** head_ptr, lru_node_t ** tail_ptr, int64_t flow_id) {
    lru_node_t * curr = *head_ptr;

    if (*head_ptr == *tail_ptr) {
        if (*head_ptr == NULL) {
            return NULL;
        }
        if ((*head_ptr)->flow_id == flow_id) {
            (*head_ptr) = NULL;
            (*tail_ptr) = NULL;
            return curr;
        }
    }
    while (curr != NULL) {
        if (curr->flow_id == flow_id) {
            if (curr == (*head_ptr) && curr == (*tail_ptr)) {
                (*head_ptr) = NULL;
                (*tail_ptr) = NULL;
                return curr;
            }
            else if (curr == (*head_ptr)) {
                lru_node_t * new_head = (*head_ptr)->next;
                new_head->prev = NULL;
                (*head_ptr) = new_head;
                return curr;
            }
            else if (curr == (*tail_ptr)) {
                lru_node_t * new_tail = (*tail_ptr)->prev;
                new_tail->next = NULL;
                (*tail_ptr) = new_tail;
                return curr;
            }
            else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                return curr;
            }
        }
        else {
            curr = curr->next;
        }
    }
    return NULL;
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

void initialize_dm_sram(dm_sram_t * sram) {
    for (int i = 0; i < sram->capacity; i++) {
        sram->flow_ids[i] = i;
    }
}

int64_t evict_from_sram(sram_t * sram, dram_t * dram) {
    lru_node_t * tail = sram->tail;
    if (tail != NULL) {
        lru_node_t * evicted = pop(&(sram->head), &(sram->tail));
        sram->count--;
        int64_t flow_id = evicted->flow_id;
        int64_t val = evicted->val;
        dram->memory[flow_id % DRAM_SIZE] = val;
        free(evicted);
        return flow_id;
    }
    else {
        // Nothing to evict
        return -1;
    }
}

int64_t pull_from_dram(sram_t * sram, dram_t * dram, int64_t flow_id) {
    lru_node_t * node = create_lru_node(flow_id, dram->memory[flow_id % DRAM_SIZE]);
    push(&(sram->head), &(sram->tail), node);
    sram->count++;
    if (sram->count > sram->capacity) {
        evict_from_sram(sram, dram);
    }
    return node->val;
}

int64_t access_sram(sram_t * sram, int64_t flow_id) {
    lru_node_t * node = remove_node(&(sram->head), &(sram->tail), flow_id);
    if (node != NULL) {
        // Cache hit
        node->val++;
        push(&(sram->head), &(sram->tail), node);
        return node->val;
    }
    // Cache miss
    return -1;
}

int64_t evict_from_dm_sram(dm_sram_t * sram, dram_t * dram, int64_t flow_id) {
    dram->memory[flow_id] = sram->memory[flow_id % sram->capacity];
    return flow_id;
}

int64_t dm_pull_from_dram(dm_sram_t * sram, dram_t * dram, int64_t flow_id) {
    evict_from_dm_sram(sram, dram, flow_id);
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