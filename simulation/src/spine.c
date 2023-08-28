#include "spine.h"

spine_t create_spine(int16_t spine_index, int32_t sram_size, int16_t init_sram)
{
    spine_t self = (spine_t) malloc(sizeof(struct spine));
    MALLOC_TEST(self, __LINE__);

    self->spine_index = spine_index;

    for (int i = 0; i < SPINE_PORT_COUNT; i++) {
        self->pkt_buffer[i] = create_buffer(SPINE_PORT_BUFFER_LEN);
        self->send_buffer[i] = create_buffer(SPINE_PORT_BUFFER_LEN);
        self->queue_stat[i] = create_timeseries();
        self->snapshot_list[i] = create_buffer(100);
    }

    self->cache_hits = 0;
    self->cache_misses = 0;

    self->sram = create_sram(sram_size, init_sram);
    self->lfu_sram = create_lfu_sram(sram_size, init_sram);
    self->arc_sram = create_arc_sram(sram_size, init_sram);
    self->dm_sram = create_dm_sram(sram_size, init_sram);
    self->dram = create_dram(DRAM_SIZE, DRAM_DELAY);

    self->access_on_this_timeslot = 0;

    return self;
}

void free_spine(spine_t self)
{
    if (self != NULL) {
        for (int i = 0; i < SPINE_PORT_COUNT; ++i) {
            if (self->pkt_buffer[i] != NULL) {
                free_buffer(self->pkt_buffer[i]);
                free_buffer(self->send_buffer[i]);
            }
            free_buffer(self->snapshot_list[i]);
            free_timeseries(self->queue_stat[i]);
        }

        free_sram(self->sram);
        free_lfu_sram(self->lfu_sram);
        free_arc_sram(self->arc_sram);
        free_dm_sram(self->dm_sram);
        free_dram(self->dram);
        free(self);
    }
}

packet_t process_packets(spine_t spine, int16_t port, int64_t * cache_misses, int64_t * cache_hits, int sram_type)
{
    //Grab the top packet in the virtual queue if the packet's flow_id is in the SRAM, otherwise pull from DRAM due to cache miss
    packet_t pkt = NULL;

    pkt = (packet_t) buffer_peek(spine->pkt_buffer[port], 0);
    // Packet buffer on this port is not empty
    if (pkt != NULL) {
        // Do not process packet if marked by control flag
        if (pkt->control_flag == 1) {
            pkt = (packet_t) buffer_get(spine->pkt_buffer[port]);
            int dst_host = pkt->dst_node;
            int dst_tor = dst_host / NODES_PER_RACK;
            assert(buffer_put(spine->send_buffer[dst_tor], pkt) != -1);
            return process_packets(spine, port, cache_misses, cache_hits, sram_type);
        }
        int64_t val = -1;
        // Determine if flow id is in SRAM
        if (sram_type == 1) {
            val = access_sram(spine->sram, pkt->flow_id);
        }
        else if (sram_type == 2) {
            val = access_lfu_sram(spine->lfu_sram, pkt->flow_id);
        }
        else if (sram_type == 3) {
            val = access_arc_sram(spine->arc_sram, pkt->flow_id);
        }
        
        // Cache miss
        if (val < 0) {
            // Memory is not locked, can begin DRAM access on this timeslot
            if (spine->dram->lock == 0) { 
                (*cache_misses)++;
                spine->cache_misses++;

                spine->dram->lock = 1;
                spine->dram->accessing = pkt->flow_id;
                spine->dram->placement_idx = 0;
            }
        }
        // Cache hit
        else {
            // printf("Spine %d Cache Hit Flow %d: %d\n", spine->spine_index, (int) pkt->flow_id, (int) val);
            (*cache_hits)++;
            spine->cache_hits++;
            return move_to_send_buffer(spine, port);
        }
    }
    
    return NULL;
}

packet_t move_to_send_buffer(spine_t spine, int16_t port) {
    // Get packet from buffer
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(spine->pkt_buffer[port]);

    // Remove entry from snapshot list (if present)
    if (spine->snapshot_list[port]->num_elements > 0) {
        int64_t id = pkt->flow_id;

        buff_node_t * first = buffer_peek(spine->snapshot_list[port], 0);
        if (first->val == id) {
            first = buffer_get(spine->snapshot_list[port]);
            free(first);
        }
    }

    // Move packet to send buffer
    int dst_host = pkt->dst_node;
    int dst_tor = dst_host / NODES_PER_RACK;
    assert(buffer_put(spine->send_buffer[dst_tor], pkt) != -1);
    return pkt;
}

packet_t send_to_tor(spine_t spine, int16_t tor_num)
{
    // Check if there are packets in send buffer destined to tor
    packet_t pkt = NULL;
    pkt = (packet_t) buffer_get(spine->send_buffer[tor_num]);
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
            (*cache_hits)++;
            
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
    snapshot_t * snapshot = create_snapshot(spine->send_buffer[tor_num], &pkts_recorded);
    return snapshot;
}

snapshot_t ** snapshot_array_spine(spine_t spine) 
{
    // Allocate array
    snapshot_t ** snapshot_array = malloc(sizeof(snapshot_t *) * NUM_OF_RACKS);
    for (int i = 0; i < NUM_OF_RACKS; i++) {
        snapshot_array[i] = malloc(sizeof(snapshot_t));
        for (int j = 0; j < SNAPSHOT_SIZE; j++) {
            snapshot_array[i]->flow_id[j] = -1;
        }
        snapshot_array[i]->time_to_dequeue_from_link = 0;
    }

    // Populate array
    int depth = 0;
    int detected = 1;
    while (detected) {
        detected = 0;
        for (int i = 0; i < NUM_OF_RACKS; i++) {
            packet_t pkt = NULL;
            pkt = buffer_peek(spine->pkt_buffer[i], depth);
            if (pkt != NULL && pkt->control_flag != 1) {
                int dst_tor = pkt->dst_node / NODES_PER_RACK;
                // Find where in snapshot to place packet
                for (int placement = 0; placement < SNAPSHOT_SIZE; placement++) {
                    if (snapshot_array[dst_tor]->flow_id[placement] == -1) {
                        detected = 1;
                        snapshot_array[dst_tor]->flow_id[placement] = pkt->flow_id;
                        break;
                    }
                }
            }
        }
        depth++;
    }

    return snapshot_array;
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