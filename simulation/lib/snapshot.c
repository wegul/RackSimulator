#include "snapshot.h"

snapshot_t * create_snapshot(buffer_t * buffer, int16_t * pkts_recorded) {
    snapshot_t * snapshot = malloc(sizeof(snapshot_t));

    int curr_pkt = 0;
    for (int i = 0; i < SNAPSHOT_SIZE; i++) {
        packet_t pkt = (packet_t) buffer_peek(buffer, curr_pkt);

        while (pkt != NULL && (pkt->control_flag == 1 || pkt->snapshotted == 1)) {
            curr_pkt++;
            pkt = (packet_t) buffer_peek(buffer, curr_pkt);
        }

        if (pkt != NULL) {
            snapshot->flow_id[i] = pkt->flow_id;
            pkt->snapshotted = 1;
            pkts_recorded++;
            curr_pkt++;
        }
        else {
            snapshot->flow_id[i] = -1;
        }
    }

    if (snapshot->flow_id[0] == -1) {
        free(snapshot);
        return NULL;
    }

    snapshot->time_to_dequeue_from_link = 0;

    return snapshot;
}

print_snapshot(snapshot_t * snapshot) {
    printf("[");
    for(int i = 0; i < SNAPSHOT_SIZE; i++) {
        if (snapshot->flow_id[i] == -1) {
            printf("x");
        }
        else {
            printf("%d", snapshot->flow_id[i]);
        }
        if (i < SNAPSHOT_SIZE - 1) {
            printf(",");
        }
    }
    printf("]\n");
}