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
        int64_t val = access_sram(spine->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
            // printf("Spine %d Cache Miss Flow %d\n", spine->spine_index, (int) pkt->flow_id);
            (*cache_misses)++;
            pull_from_dram(spine->sram, spine->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
            // printf("Spine %d Cache Hit Flow %d: %d\n", spine->spine_index, (int) pkt->flow_id, (int) val);
            (*cache_hits)++;
            if (spine->snapshot_idx[tor_num] > 0) {
                spine->snapshot_idx[tor_num]--;
            }
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
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
        int64_t val = access_dm_sram(spine->dm_sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
            //printf("Spine %d Cache Miss Flow %d\n", spine->spine_index, (int) pkt->flow_id);
            (*cache_misses)++;
            dm_pull_from_dram(spine->dm_sram, spine->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
            //printf("Spine %d Cache Hit Flow %d: %d\n", spine->spine_index, (int) pkt->flow_id, (int) val);
            (*cache_hits)++;
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
        if (spine->dram->accessible[pkt->flow_id] > 0) {
            spine->dram->accessible[pkt->flow_id] = 0;
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
            return pkt;
        }
        else {
            (*cache_misses)++;
            spine->dram->accessible[pkt->flow_id] = 1;
            return NULL;
        }
    }
    
    return pkt;
}

snapshot_t * snapshot_to_tor(spine_t spine, int16_t tor_num)
{
    //int16_t pkts_recorded = 0;
    snapshot_t * snapshot = NULL;
    // snapshot_t * snapshot = create_snapshot(spine->pkt_buffer[tor_num], spine->snapshot_idx[tor_num], &pkts_recorded);
    // spine->snapshot_idx[tor_num] += pkts_recorded;
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
