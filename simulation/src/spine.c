#include "spine.h"

spine_t create_spine(int16_t spine_index)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < SPINE_PORT_COUNT; i++) {
        self->pkt_buffer[i] = create_buffer(SPINE_PORT_BUFFER_LEN);
        self->queue_stat[i] = create_timeseries();
        self->snapshot_idx[i] = 0;
    }

    self->sram = create_sram(SRAM_SIZE, 1);
    initialize_sram(self->sram);
    self->dm_sram = create_dm_sram(SRAM_SIZE, 1);
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

packet_t send_to_tor(spine_t spine, int16_t tor_num)
{
    //Grab the top packet in the virtual queue if the packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(spine->pkt_buffer[tor_num], 0);
    if (pkt != NULL) {
        int64_t val = access_sram(spine->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
#ifdef DEBUG_MEMORY
            printf("Spine %d Cache Miss Flow %d", spine->spine_index, (int) pkt->flow_id);
#endif
            pull_from_dram(spine->sram, spine->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
#ifdef DEBUG_MEMORY
            printf("Spine %d Cache Hit Flow %d: %d", spine->spine_index, (int) pkt->flow_id, (int) val);
#endif
            if (spine->snapshot_idx[tor_num] > 0) {
                spine->snapshot_idx[tor_num]--;
            }
            pkt = (packet_t) buffer_get(spine->pkt_buffer[tor_num]);
        }
    }
    
    return pkt;
}

snapshot_t * snapshot_to_tor(spine_t spine, int16_t tor_num)
{
    int16_t pkts_recorded = 0;
    snapshot_t * snapshot = create_snapshot(spine->pkt_buffer[tor_num], spine->snapshot_idx[tor_num], &pkts_recorded);
    spine->snapshot_idx[tor_num] += pkts_recorded;
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