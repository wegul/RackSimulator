#include "snapshot.h"

snapshot_t * create_snapshot(buffer_t * buffer, int16_t start_idx, int16_t * pkts_recorded) {
    int snapshot_size = buffer->num_elements - start_idx;
    if (snapshot_size > SNAPSHOT_SIZE) {
        snapshot_size = SNAPSHOT_SIZE;
    }
    if (snapshot_size < 1) {
        *pkts_recorded = 0;
        return NULL;
    }
    snapshot_t * snapshot = malloc(sizeof(snapshot_t));
    int64_t queue_delay_up_to_start = 0;
    for (int i = 0; i < start_idx; i++) {
        packet_t pkt = (packet_t) buffer_peek(buffer, i);
        queue_delay_up_to_start += pkt->size;
    }
    for (int i = 0; i < SNAPSHOT_SIZE; i++) {
        if (i < snapshot_size) {
            packet_t pkt = (packet_t) buffer_peek(buffer, start_idx + i);
            if (pkt != NULL) {
                snapshot->flow_id[i] = pkt->flow_id;
                snapshot->queue_delay_kb[i] = (int8_t) (queue_delay_up_to_start / 1000);
                queue_delay_up_to_start += pkt->size;
            }
        }
        else {
            snapshot->flow_id[i] = 0;
            snapshot->queue_delay_kb[i] = 0;
        }
    }
    snapshot->time_to_dequeue_from_link = 0;

    *pkts_recorded = snapshot_size;

    return snapshot;
}