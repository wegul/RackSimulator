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
        self->upstream_snapshot_list[i] = create_buffer(100);
    }
    
    create_routing_table(self->routing_table);

    self->sram = create_sram(sram_size, init_sram);
    self->dm_sram = create_dm_sram(sram_size, init_sram);
    self->dram = create_dram(DRAM_SIZE, DRAM_DELAY);

    self->access_on_this_timeslot = 0;
    
    return self;
}

void free_tor(tor_t self)
{
    if (self != NULL) {

        for (int i = 0; i < NODES_PER_RACK; ++i) {
            free_buffer(self->upstream_pkt_buffer[i]);
            free_buffer(self->downstream_send_buffer[i]);
            free_timeseries(self->downstream_queue_stat[i]);
            free_buffer(self->downstream_snapshot_list[i]);
        }

        for (int i = 0; i < NUM_OF_SPINES; ++i) {
            free_buffer(self->downstream_pkt_buffer[i]);
            free_buffer(self->upstream_send_buffer[i]);
            free_timeseries(self->upstream_queue_stat[i]);
            free_buffer(self->upstream_snapshot_list[i]);
        }

        free_sram(self->sram);
        free_dm_sram(self->dm_sram);
        free_dram(self->dram);

        free(self);
    }
}

packet_t process_packets_up(tor_t tor, int16_t port, int64_t * cache_misses, int64_t * cache_hits)
{
    // Grab top packet in virtual queue if packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[port], 0);

    // Packet buffer on this port is not empty
    if (pkt != NULL) {
        // Do not process packet if marked by control flag
        if (pkt->control_flag == 1) {
            pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[port]);
            int dst_spine = hash(tor->routing_table, pkt->flow_id);
            assert(buffer_put(tor->upstream_send_buffer[dst_spine], pkt) != -1);
            return process_packets_up(tor, port, cache_misses, cache_hits);
        }
        int64_t val = access_sram(tor->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
            // printf("ToR %d Cache Miss Flow %d\n", tor->tor_index, (int) pkt->flow_id);
            // If access is successful, pull flow up to SRAM and return while logging cache miss
            if (tor->dram->accessible[pkt->flow_id] >= tor->dram->delay) {
                (*cache_misses)++;
                tor->dram->accessible[pkt->flow_id] = 0;
                pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
                return move_to_up_send_buffer(tor, port);
            }
            // Otherwise, continue trying to access and return NULL
            else {
                tor->dram->accessible[pkt->flow_id] += tor->dram->accesses;
                return NULL;
            }
        }
        // Cache hit
        else {
            // printf("ToR %d Cache Hit Flow %d\n", tor->tor_index, (int) pkt->flow_id, (int) val);
            (*cache_hits)++;
            return move_to_up_send_buffer(tor, port);
        }
    }

    return NULL;
}

packet_t move_to_up_send_buffer(tor_t tor, int16_t port) {
    // Get packet from buffer
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(tor->upstream_pkt_buffer[port]);

    // Remove entry from snapshot list (if present)
    if (tor->upstream_snapshot_list[port]->num_elements > 0) {
        int64_t id = pkt->flow_id;

        buff_node_t * first = buffer_peek(tor->upstream_snapshot_list[port], 0);
        if (first->val == id) {
            first = buffer_get(tor->upstream_snapshot_list[port]);
            free(first);
        }
    }

    // Move packet to send buffer
    int dst_spine = hash(tor->routing_table, pkt->flow_id);
    assert(buffer_put(tor->upstream_send_buffer[dst_spine], pkt) != -1);
    return pkt;
}

packet_t send_to_spine(tor_t tor, int16_t spine_id)
{
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(tor->upstream_send_buffer[spine_id]);
    return pkt;
}

packet_t send_to_spine_dm(tor_t tor, int16_t spine_id, int64_t * cache_misses, int64_t * cache_hits)
{
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->upstream_pkt_buffer[spine_id], 0);
    if (pkt != NULL) {
        int64_t val = access_dm_sram(tor->dm_sram, pkt->flow_id);
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

packet_t process_packets_down(tor_t tor, int16_t port, int64_t * cache_misses, int64_t * cache_hits)
{
    //Grab the top packet in the virtual queue if the packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(tor->downstream_pkt_buffer[port], 0);
    // Packet buffer on this port is not empty
    if (pkt != NULL) {
        // Do not process packet if marked by control flag
        if (pkt->control_flag == 1) {
            pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[port]);
            int dst_host = pkt->dst_node % NODES_PER_RACK;
            assert(buffer_put(tor->downstream_send_buffer[dst_host], pkt) != -1);
            return process_packets_down(tor, port, cache_misses, cache_hits);
        }
        int64_t val = access_sram(tor->sram, pkt->flow_id);
        // Cache miss
        if (val < 0) {
            // If access is successful, pull flow up to SRAM and return while logging cache miss
            if (tor->dram->accessible[pkt->flow_id] >= tor->dram->delay) {
                (*cache_misses)++;
                tor->dram->accessible[pkt->flow_id] = 0;
                pull_from_dram(tor->sram, tor->dram, pkt->flow_id);
                return move_to_down_send_buffer(tor, port);
            }
            // Otherwise, continue trying to access and return NULL
            else {
                tor->dram->accessible[pkt->flow_id] += tor->dram->accesses;
                return NULL;
            }
        }
        // Cache hit
        else {
            (*cache_hits)++;
            return move_to_down_send_buffer(tor, port);
        }
    }

    return NULL;
}

packet_t move_to_down_send_buffer(tor_t tor, int16_t port) {
    // Get packet from buffer
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[port]);

    // Remove entry from snapshot list (if present)
    if (tor->downstream_snapshot_list[port]->num_elements > 0) {
        int64_t id = pkt->flow_id;
        buff_node_t * first = buffer_peek(tor->downstream_snapshot_list[port], 0);
        if (first->val == id) {
            first = buffer_get(tor->downstream_snapshot_list[port]);
            free(first);
        }
    }

    // Move packet to send buffer
    int dst_host = pkt->dst_node % NODES_PER_RACK;
    assert(buffer_put(tor->downstream_send_buffer[dst_host], pkt) != -1);
    return pkt;
}

packet_t send_to_host(tor_t tor, int16_t host_within_tor)
{
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(tor->downstream_pkt_buffer[host_within_tor]);
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
    packet_t pkt = (packet_t) buffer_get(tor->upstream_send_buffer[spine_id]);
    return pkt;
}

packet_t send_to_host_baseline(tor_t tor, int16_t host_within_tor) {
    packet_t pkt = (packet_t) buffer_get(tor->downstream_send_buffer[host_within_tor]);
    return pkt;
}

snapshot_t * snapshot_to_spine(tor_t tor, int16_t spine_id)
{
    int16_t pkts_recorded = 0;
    snapshot_t * snapshot = create_snapshot(tor->upstream_send_buffer[spine_id], &pkts_recorded);
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
