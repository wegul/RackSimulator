#include "tor.h"

tor_t create_tor(int16_t tor_index)
{
    tor_t self = (tor_t) malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);

    self->tor_index = tor_index;

    for (int i = 0; i < NODES_PER_RACK; ++i) {
        self->downstream_pkt_buffer[i]
            = create_buffer(TOR_DOWNSTREAM_BUFFER_LEN);
        self->downstream_queue_stat[i] = create_timeseries();
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        self->upstream_pkt_buffer[i]
            = create_buffer(TOR_UPSTREAM_BUFFER_LEN);
        self->upstream_queue_stat[i] = create_timeseries();
        self->snapshot_idx[i] = 0;
    }
    
    create_routing_table(self->routing_table);

    self->sram = create_sram(SRAM_SIZE, 1);
    initialize_sram(self->sram);
    self->dm_sram = create_dm_sram(SRAM_SIZE, 1);
    self->dram = create_dram(DRAM_SIZE, DRAM_DELAY);
    
    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL) {

        for (int i = 0; i < NODES_PER_RACK; ++i) {
            free_buffer(self->downstream_pkt_buffer[i]);
            free_timeseries(self->downstream_queue_stat[i]);
        }

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            free_buffer(self->upstream_pkt_buffer[i]);
            free_timeseries(self->upstream_queue_stat[i]);
        }

        free_sram(self->sram);
        free_dm_sram(self->dm_sram);
        free_dram(self->dram);

        free(self);
    }
}

packet_t send_to_spine(tor_t tor, int16_t spine_id)
{
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[spine_id], 0);
    if (pkt != NULL) {
        int64_t val = access_sram(tor->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
#ifdef DEBUG_MEMORY
            printf("Tor %d Cache Miss Flow %d", tor->tor_index, (int) pkt->flow_id);
#endif
            pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
#ifdef DEBUG_MEMORY
            printf("Tor %d Cache Hit Flow %d: %d", tor->tor_index, (int) pkt->flow_id, (int) val);
#endif
            if (tor->snapshot_idx[spine_id] > 0) {
                tor->snapshot_idx[spine_id]--;
            }
            pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);
        }
    }

    return pkt;
}

packet_t send_to_host(tor_t tor, int16_t host_within_tor)
{
    packet_t pkt = NULL;
    
    pkt = (packet_t) buffer_peek(tor->downstream_pkt_buffer[host_within_tor], 0);
    if (pkt != NULL) {
        int64_t val = access_sram(tor->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
#ifdef DEBUG_MEMORY
            printf("Tor %d Cache Miss Flow %d", tor->tor_index, (int) pkt->flow_id);
#endif
            pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
#ifdef DEBUG_MEMORY
            printf("Tor %d Cache Hit Flow %d: %d", tor->tor_index, (int) pkt->flow_id, (int) val);
#endif
            pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[host_within_tor]);
        }
    }

    return pkt;
}

snapshot_t * snapshot_to_spine(tor_t tor, int16_t spine_id)
{
    int16_t pkts_recorded = 0;
    snapshot_t * snapshot = create_snapshot(tor->upstream_pkt_buffer[spine_id], tor->snapshot_idx[spine_id], &pkts_recorded);
    tor->snapshot_idx[spine_id] += pkts_recorded;
    return snapshot;
}

int64_t tor_up_buffer_bytes(tor_t tor, int port)
{
    int64_t bytes = 0;
    for (int i = 0; i < tor->upstream_pkt_buffer[port]->num_elements; i++) {
        packet_t pkt = buffer_peek(tor->upstream_pkt_buffer[port], i);
        bytes += pkt->size;
    }
    return bytes;
}

int64_t tor_down_buffer_bytes(tor_t tor, int port)
{
    int64_t bytes = 0;
    for (int i = 0; i < tor->downstream_pkt_buffer[port]->num_elements; i++) {
        packet_t pkt = buffer_peek(tor->downstream_pkt_buffer[port], i);
        bytes += pkt->size;
    }
    return bytes;
}