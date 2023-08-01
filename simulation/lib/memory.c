#include "memory.h"

lru_node_t * create_lru_node(int64_t flow_id, int64_t val) {
    lru_node_t * node = malloc(sizeof(lru_node_t));
    MALLOC_TEST(node, __LINE__);
    node->flow_id = flow_id;
    node->val = val;
    node->next = NULL;

    return node;
}

void push(lru_node_t ** head_ptr, lru_node_t * node) {
    node->next = (*head_ptr);
    (*head_ptr) = node;
}

lru_node_t * pop(lru_node_t ** head_ptr) {
    if (*head_ptr == NULL) {
        return NULL;
    }

    lru_node_t * node = (*head_ptr);
    lru_node_t * next = node->next;

    if (next == NULL) {
        (*head_ptr) = NULL;
        return node;
    }

    while(next->next != NULL) {
        node = node->next;
        next = node->next;
    }

    node->next = NULL;
    return next;
}

void insert_node(lru_node_t ** head_ptr, lru_node_t * node, int index) {
    if ((*head_ptr) == NULL || index == 0) {
        node->next = (*head_ptr);
        (*head_ptr) = node;
        return;
    }

    int curr_index = 1;
    lru_node_t * curr_node = (*head_ptr);

    while (curr_node->next != NULL && curr_index < index) {
        curr_node = curr_node->next;
        curr_index++;
    }

    node->next = curr_node->next;
    curr_node->next = node;
}

lru_node_t * remove_node(lru_node_t ** head_ptr, int64_t flow_id) {
    if (*head_ptr == NULL) {
        return NULL;
    }

    lru_node_t * curr = *head_ptr;
    lru_node_t * next = curr->next;

    if (curr->flow_id == flow_id) {
        (*head_ptr) = next;
        curr->next = NULL;
        return curr;
    }

    while (next != NULL) {
        if (next->flow_id == flow_id) {
            curr->next = next->next;
            next->next = NULL;
            return next;
        }
        curr = next;
        next = next->next;
    }
    
    return NULL;
}

lru_node_t * remove_node_return_index(lru_node_t ** head_ptr, int64_t flow_id, int * index) {
    int idx = 0;

    if (*head_ptr == NULL) {
        (*index) = idx;
        return NULL;
    }

    lru_node_t * curr = *head_ptr;
    lru_node_t * next = curr->next;

    if (curr->flow_id == flow_id) {
        (*head_ptr) = next;
        curr->next = NULL;
        (*index) = idx;
        return curr;
    }

    while (next != NULL) {
        idx++;
        if (next->flow_id == flow_id) {
            curr->next = next->next;
            next->next = NULL;
            (*index = idx);
            return next;
        }
        curr = next;
        next = next->next;
    }
    
    return NULL;
}

sram_t * create_sram(int32_t size, int16_t initialize) {
    sram_t * sram = malloc(sizeof(sram_t));
    MALLOC_TEST(sram, __LINE__);
    sram->capacity = size;
    sram->count = 0;
    sram->head = NULL;

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
            if (i == 0) {
                sram->flow_ids[i] = 1;
            }
            else {
                sram->flow_ids[i] = 0;
            }
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
    dram->accesses = 1;
    dram->memory = malloc(sizeof(int64_t) * size); // Contains value associated with flow_id (i.e. val)
    dram->accessible = malloc(sizeof(int) * size); // When using DRAM-only, shows which values are accessible at any given moment
    for (int i = 0; i < size; i++) {
        dram->memory[i] = 0;
        dram->accessible[i] = 0;
    }

    dram->lock = 0;
    dram->accessing = -1;
    dram->placement_idx = 0;
    
    return dram;
}

void initialize_sram(sram_t * sram) {
    for (int i = 0; i < sram->capacity; i++) {
        lru_node_t * node = create_lru_node(i, 0);
        node->next = sram->head;
        sram->head = node;
    }
    sram->count = sram->capacity;
}

void initialize_dm_sram(dm_sram_t * sram) {
    for (int i = 0; i < sram->capacity; i++) {
        sram->flow_ids[i] = i;
    }
}

int64_t evict_from_sram(sram_t * sram, dram_t * dram) {
    if (sram->head != NULL) {
        lru_node_t * evicted = pop(&(sram->head));
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
    lru_node_t * node = create_lru_node(flow_id, dram->memory[flow_id]);
    insert_node(&(sram->head), node, dram->placement_idx);
    //push(&(sram->head), node);
    sram->count++;
    if (sram->count > sram->capacity) {
        evict_from_sram(sram, dram);
    }
    return node->val;
}

int64_t access_sram(sram_t * sram, int64_t flow_id) {
    lru_node_t * node = remove_node(&(sram->head), flow_id);
    if (node != NULL) {
        // Cache hit
        node->val++;
        push(&(sram->head), node);
        return node->val;
    }
    // Cache miss
    return -1;
}

int64_t access_sram_return_index(sram_t * sram, int64_t flow_id) {
    int idx = 0;
    lru_node_t * node = remove_node_return_index(&(sram->head), flow_id, &idx);
    if (node != NULL) {
        // Cache hit
        node->val++;
        push(&(sram->head), node);
        return idx;
    }
    // Cache miss
    return -1;
}

int64_t evict_from_dm_sram(dm_sram_t * sram, dram_t * dram, int64_t flow_id) {
    int64_t evict_id = sram->flow_ids[flow_id % sram->capacity];
    dram->memory[evict_id] = sram->memory[evict_id % sram->capacity];
    return evict_id;
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
        return sram->memory[(flow_id % sram->capacity)]; // SUCCESS!!!
    }
    // Cache miss
    return -1; // FAILURE!!!
}

int64_t reorganize_sram(sram_t * sram, buffer_t * buffer) {
    // buffer must be a buffer of int64_t from snapshot
    int k = -1;
    for (int i = buffer->num_elements - 1; i >= 0; i--) {
        int64_t * id = (int64_t *) buffer_peek(buffer, i);
        int result = access_sram(sram, *id);
        if (result < 0) {
            k = *id;
        }
    }
    return k;
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

void print_sram(sram_t * sram) {
    lru_node_t * head = sram->head;
    printf("Current sram state\n");
    while (head != NULL) {
        printf("%d, %d || ", (int) head->flow_id, (int) head->val);
        head = head->next;
    }
    printf("\n");
}

void print_dm_sram(dm_sram_t * sram) {
    printf("Current sram state\n");
    for (int i = 0; i < sram->capacity; i++) {
        printf("%d, %d ||", (int) sram->flow_ids[i], (int) sram->memory[i]);
    }
}

int64_t belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len) {
    if (q_len > 0) {
        for (int j = 0; j < q_len; j++) {
            int64_t id = lin_queue[j];
            if (access_sram(sram, id) < 0) {
                lru_node_t * node = create_lru_node(id, dram->memory[id]);
                push(&(sram->head), node);
                sram->count++;
                if (sram->count > sram->capacity) {
                    evict_belady(sram, dram, lin_queue, q_len);
                }
                return 0;
            }
        }
    }
    return -1;
}

int64_t evict_belady(sram_t * sram, dram_t * dram, int64_t * lin_queue, int q_len) {
    if (q_len > 0) {
        // Find flow in SRAM latest to be accessed
        for (int i = q_len - 1; i >= 0; i--) {
            int64_t id = lin_queue[i];
            access_sram(sram, id);
        }
        // Last index will be latest to access
        lru_node_t * evicted = pop(&(sram->head));
        sram->count--;
        int64_t flow_id = evicted->flow_id;
        int64_t val = evicted->val;
        dram->memory[flow_id % DRAM_SIZE] = val;
        free(evicted);
        return flow_id;
    }
    else {
        evict_from_sram(sram, dram);
    }
    return -1;
}

void free_dm_sram(dm_sram_t * sram) {
    free(sram->flow_ids);
    free(sram->memory);
    free(sram);
}

void free_dram(dram_t * dram) {
    free(dram->memory);
    free(dram->accessible);
    free(dram);
}
