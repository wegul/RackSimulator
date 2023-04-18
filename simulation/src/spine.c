#include "spine.h"

spine_t create_spine(int16_t spine_index, int32_t sram_size, int16_t init_sram)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < SPINE_PORT_COUNT; i++) {
        self->pkt_buffer[i] = create_buffer(SPINE_PORT_BUFFER_LEN);
        self->queue_stat[i] = create_timeseries();
        self->snapshot_idx[i] = 0;
        self->snapshot_list[i] = create_buffer(100);
    }

    self->sram = create_sram(sram_size, init_sram);
    self->dm_sram = create_dm_sram(sram_size, init_sram);
    self->dram = create_dram(DRAM_SIZE, DRAM_DELAY);

    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < SPINE_PORT_COUNT; ++i) {
            if (self->pkt_buffer[i] != NULL) {
                free_buffer(self->pkt_buffer[i]);
            }
            free_buffer(self->snapshot_list[i]);
            free_timeseries(self->queue_stat[i]);
        }

        free_sram(self->sram);
        free_dm_sram(self->dm_sram);
        free_dram(self->dram);
        free(self);
    }
}

packet_t send_to_tor(spine_t spine, int16_t tor_num, int64_t * cache_misses, int64_t * cache_hits)
{
    //Grab the top packet in the virtual queue if the packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(spine->pkt_buffer[tor_num], 0);
    if (pkt != NULL) {
        int is_fresh = 0;
        int64_t val = access_sram(spine->sram, pkt->flow_id, &is_fresh);
        // Cache miss
        if (val < 0) {
            // printf("Spine %d Cache Miss Flow %d\n", spine->spine_index, (int) pkt->flow_id);
            
            spine->dram->accessible[pkt->flow_id] += spine->dram->accesses;
            if (spine->dram->accessible[pkt->flow_id] >= spine->dram->delay) {
                (*cache_misses)++;
                spine->dram->accessible[pkt->flow_id] = 0;
                pull_from_dram(spine->sram, spine->dram, pkt->flow_id);
                //pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
                //return pkt;
            }
            return NULL;
        }
        // Cache hit
        else {
            // printf("Spine %d Cache Hit Flow %d: %d\n", spine->spine_index, (int) pkt->flow_id, (int) val);
            if (is_fresh == 0) {
                (*cache_hits)++;
            }
            
            if (spine->snapshot_idx[tor_num] > 0) {
                spine->snapshot_idx[tor_num]--;
            }
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);

            if (spine->snapshot_list[tor_num]->num_elements > 0) {
                buffer_get(spine->snapshot_list[tor_num]);

                int64_t * snapshot_id = buffer_peek(spine->snapshot_list[tor_num], 0);
                printf("%d\n", *snapshot_id);

                if (snapshot_id != NULL && pkt->flow_id == *snapshot_id) {
                    printf("popped off snapshot\n");
                    buffer_get(spine->snapshot_list[tor_num]);
                    
                }
                else {
                    printf("Incorrect!!\n");
                }
            }   
        }
    }
    
    return pkt;
}

packet_t send_to_tor_dm(spine_t spine, int16_t tor_num, int64_t * cache_misses, int64_t * cache_hits)
{
    //Grab the top packet in the virtual queue if the packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(spine->pkt_buffer[tor_num], 0);
    if (pkt != NULL) {
        int is_fresh = 0;
        int64_t val = access_dm_sram(spine->dm_sram, pkt->flow_id, &is_fresh);
        // Cache miss
        if (val < 0) {
            //printf("Spine %d Cache Miss Flow %d\n", spine->spine_index, (int) pkt->flow_id);
            spine->dram->accessible[pkt->flow_id] += spine->dram->accesses;
            if (spine->dram->accessible[pkt->flow_id] >= spine->dram->delay) {
                (*cache_misses)++;
                spine->dram->accessible[pkt->flow_id] = 0;
                dm_pull_from_dram(spine->dm_sram, spine->dram, pkt->flow_id);
                pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
                return pkt;
            }
            
            return NULL;
        }
        // Cache hit
        else {
            //printf("Spine %d Cache Hit Flow %d: %d\n", spine->spine_index, (int) pkt->flow_id, (int) val);
            if (is_fresh == 0) {
                (*cache_hits)++;
            }
            
            if (spine->snapshot_idx[tor_num] > 0) {
                spine->snapshot_idx[tor_num]--;
            }
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
        }
    }
    
    return pkt;
}

packet_t send_to_tor_dram_only(spine_t spine, int16_t tor_num, int64_t * cache_misses) {
    // Grab the top packeet in the virtual queue if a DRAM request was given last timeslot
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(spine->pkt_buffer[tor_num], 0);
    if (pkt != NULL) {
        if (spine->dram->accessible[pkt->flow_id] == spine->dram->delay) {
            //for (int i = 0; i < DRAM_SIZE; i++) {
            //    spine->dram->accessible[i] = 0;
            //}
            (*cache_misses)++;
            spine->dram->accessible[pkt->flow_id] = 0;
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
            return pkt;
        }
        else {
            
            spine->dram->accessible[pkt->flow_id]++;
            return NULL;
        }
    }
    
    return pkt;
}

snapshot_t * snapshot_to_tor(spine_t spine, int16_t tor_num)
{
    int16_t pkts_recorded = 0;
    snapshot_t * snapshot = create_snapshot(spine->pkt_buffer[tor_num], &pkts_recorded);
    return snapshot;
}

int64_t spine_buffer_bytes(spine_t spine, int port)
{
    int64_t bytes = 0;
    for (int i = 0; i < spine->pkt_buffer[port]->num_elements; i++) {
        packet_t pkt = buffer_peek(spine->pkt_buffer[port], i);
        bytes += pkt->size;
    }
    return bytes;
}

int64_t * linearize_spine_queues(spine_t spine, int * q_len) {
    int lin_queue_len = 0;
    for (int i = 0; i < NUM_OF_TORS; i++) {
        lin_queue_len += spine->snapshot_list[i]->num_elements;
    }
    int64_t * lin_queue = malloc(sizeof(int64_t) * lin_queue_len);

    int idx = 0;
    int depth = 0;
    int valid_entry = 1;
    while (valid_entry) {
        valid_entry = 0;
        for (int i = 0; i < NUM_OF_TORS; i++) {
            buffer_t * ss_list = spine->snapshot_list[i];
            if (depth < ss_list->num_elements) {
                lin_queue[idx] = *((int64_t *) buffer_peek(ss_list, depth));
                idx++;
                valid_entry = 1;
            }
        }
        depth++;
    }

    *q_len = lin_queue_len;
    return lin_queue;
}