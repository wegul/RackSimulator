#include "tor.h"

tor_t create_tor(int16_t tor_index, int32_t sram_size, int16_t init_sram)
{
    tor_t self = (tor_t) malloc(sizeof(struct tor));
    MALLOC_TEST(self, __LINE__);

    self->tor_index = tor_index;

    for (int i = 0; i < NODES_PER_RACK; ++i) {
        self->downstream_pkt_buffer[i]
            = create_buffer(TOR_DOWNSTREAM_BUFFER_LEN);
        self->downstream_queue_stat[i] = create_timeseries();
        self->downstream_snapshot_list[i] = create_buffer(100);
    }

    for (int i = 0; i < NUM_OF_SPINES; ++i) {
        self->upstream_pkt_buffer[i]
            = create_buffer(TOR_UPSTREAM_BUFFER_LEN);
        self->upstream_queue_stat[i] = create_timeseries();
        self->snapshot_idx[i] = 0;
        self->upstream_snapshot_list[i] = create_buffer(100);
    }
    
    create_routing_table(self->routing_table);

    self->sram = create_sram(sram_size, init_sram);
    self->dm_sram = create_dm_sram(sram_size, init_sram);
    self->dram = create_dram(DRAM_SIZE, DRAM_DELAY);
    
    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL) {

        for (int i = 0; i < NODES_PER_RACK; ++i) {
            free_buffer(self->downstream_pkt_buffer[i]);
            free_timeseries(self->downstream_queue_stat[i]);
            free_buffer(self->downstream_snapshot_list[i]);
        }

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            free_buffer(self->upstream_pkt_buffer[i]);
            free_timeseries(self->upstream_queue_stat[i]);
            free_buffer(self->upstream_snapshot_list[i]);
        }

        free_sram(self->sram);
        free_dm_sram(self->dm_sram);
        free_dram(self->dram);

        free(self);
    }
}

packet_t send_to_spine(tor_t tor, int16_t spine_id, int64_t * cache_misses, int64_t * cache_hits)
{
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[spine_id], 0);
    if (pkt != NULL) {
        int is_fresh = 0;
        int64_t val = access_sram(tor->sram, pkt->flow_id, &is_fresh);
        // Cache miss
        if (val < 0) {
            tor->dram->accessible[pkt->flow_id] += tor->dram->accesses;
            if (tor->dram->accessible[pkt->flow_id] >= tor->dram->delay) {
                (*cache_misses)++;
                tor->dram->accessible[pkt->flow_id] = 0;
                pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
            }
            return NULL;
        }
        // Cache hit
        else {
            //printf("Tor %d Cache Hit Flow %d: %d\n", tor->tor_index, (int) pkt->flow_id, (int) val);
            if (is_fresh == 0) {
                (*cache_hits)++;
            }
            
            if (tor->snapshot_idx[spine_id] > 0) {
                tor->snapshot_idx[spine_id]--;
            }
            pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);
        }
    }

    return pkt;
}

packet_t send_to_spine_dm(tor_t tor, int16_t spine_id, int64_t * cache_misses, int64_t * cache_hits)
{
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[spine_id], 0);
    if (pkt != NULL) {
        int is_fresh = 0;
        int64_t val = access_dm_sram(tor->dm_sram, pkt->flow_id, &is_fresh);
        // Cache miss
        if (val < 0) {
            (*cache_misses)++;
            //printf("Tor %d Cache Miss Flow %d\n", tor->tor_index, (int) pkt->flow_id);
            dm_pull_from_dram(tor->dm_sram, tor->dram, pkt->flow_id);
            return NULL;
        }
        // Cache hit
        else {
            (*cache_hits)++;
            //printf("Tor %d Cache Hit Flow %d: %d\n", tor->tor_index, (int) pkt->flow_id, (int) val);
            if (tor->snapshot_idx[spine_id] > 0) {
                tor->snapshot_idx[spine_id]--;
            }
            pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);
        }
    }

    return pkt;
}

packet_t send_to_spine_dram_only(tor_t tor, int16_t spine_id, int64_t * cache_misses) {
    // Grab the top packeet in the virtual queue if a DRAM request was given last timeslot
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[spine_id], 0);
    if (pkt != NULL) {
        if (tor->dram->accessible[pkt->flow_id] > 0) {
            tor->dram->accessible[pkt->flow_id] = 0;
            //printf("Send on flow %d\n", pkt->flow_id);
            pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);
            return pkt;
        }
        else {
            (*cache_misses)++;
            //printf("Enabled flow %d\n", pkt->flow_id);
            tor->dram->accessible[pkt->flow_id] = 1;
            return NULL;
        }
    }
    
    return NULL;
}

packet_t send_to_host(tor_t tor, int16_t host_within_tor, int16_t fa_sram, int64_t * cache_misses, int64_t * cache_hits)
{
    packet_t pkt = NULL;
    
    pkt = (packet_t) buffer_peek(tor->downstream_pkt_buffer[host_within_tor], 0);
    if (pkt != NULL) {
        int64_t val = -1;
        int is_fresh = 0;
        if (fa_sram > 0) {
            val = access_sram(tor->sram, pkt->flow_id, &is_fresh);
        }
        else {
            val = access_dm_sram(tor->dm_sram, pkt->flow_id, &is_fresh);
        }
        // Cache miss
        if (val < 0) {
            //printf("Tor %d Cache Miss Flow %d\n", tor->tor_index, (int) pkt->flow_id);

            tor->dram->accessible[pkt->flow_id] += tor->dram->accesses;
            if (tor->dram->accessible[pkt->flow_id] >= tor->dram->delay) {
                (*cache_misses)++;
                tor->dram->accessible[pkt->flow_id] = 0;
                if (fa_sram > 0) {
                    pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
                }
                else {
                    dm_pull_from_dram(tor->dm_sram, tor->dram, pkt->flow_id);
                }
                //printf("tor %d pulling flow %d from access\n", tor->tor_index, pkt->flow_id);
                //print_sram(tor->sram);
            }
            return NULL;
        }
        // Cache hit
        else {
            //printf("Tor %d Cache Hit Flow %d: %d\n", tor->tor_index, (int) pkt->flow_id, (int) val);
            if (is_fresh == 0) {
                (*cache_hits)++;
            }

            pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[host_within_tor]);

            if (tor->downstream_snapshot_list[host_within_tor]->num_elements > 0) {
                int64_t id = pkt->flow_id;
                for(int i = 0; i < tor->downstream_snapshot_list[host_within_tor]->num_elements; i++) {
                    buff_node_t * node = buffer_peek(tor->downstream_snapshot_list[host_within_tor], i);
                    if (node->val == id) {
                        node = buffer_remove(tor->downstream_snapshot_list[host_within_tor], i);
                        free(node);
                        break;
                    }
                }
            }   
        }
    }

    return pkt;
}

packet_t send_to_host_dram_only(tor_t tor, int16_t host_within_tor, int64_t * cache_misses) {
    // Grab the top packeet in the virtual queue if a DRAM request was given last timeslot
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->downstream_pkt_buffer[host_within_tor], 0);
    if (pkt != NULL) {
        if (tor->dram->accessible[pkt->flow_id] > 0) {
            tor->dram->accessible[pkt->flow_id] = 0;
            pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[host_within_tor]);
            return pkt;
        }
        else {
            (*cache_misses)++;
            tor->dram->accessible[pkt->flow_id] = 1;
            return NULL;
        }
    }
    
    return pkt;
}

packet_t send_to_spine_baseline(tor_t tor, int16_t spine_id) {
    packet_t pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[spine_id]);
    return pkt;
}

packet_t send_to_host_baseline(tor_t tor, int16_t host_within_tor) {
    packet_t pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[host_within_tor]);
    return pkt;
}

snapshot_t * snapshot_to_spine(tor_t tor, int16_t spine_id)
{
    int16_t pkts_recorded = 0;
    snapshot_t * snapshot = create_snapshot(tor->upstream_pkt_buffer[spine_id], &pkts_recorded);
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

int64_t * linearize_tor_downstream_queues(tor_t tor, int * q_len){ 
    int lin_queue_len = 0;
    for (int i = 0; i < NODES_PER_RACK; i++) {
        lin_queue_len += tor->downstream_snapshot_list[i]->num_elements;
    }

    if (lin_queue_len < 1) {
        return 0;
    }

    int64_t * lin_queue = malloc(sizeof(int64_t) * lin_queue_len);

    int idx = 0;
    int depth = 0;
    int valid_entry = 1;
    while (valid_entry) {
        valid_entry = 0;
        for (int i = 0; i < NODES_PER_RACK; i++) {
            buffer_t * ss_list = tor->downstream_snapshot_list[i];
            if (depth < ss_list->num_elements) {
                buff_node_t * node = (buff_node_t *) buffer_peek(ss_list, depth);
                lin_queue[idx] = node->val;
                idx++;
                valid_entry = 1;
            }
        }
        depth++;
    }

    *q_len = lin_queue_len;
    return lin_queue;
}
