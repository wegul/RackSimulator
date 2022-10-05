#include "system_stats.h"

void print_system_stats(spine_t * spines, tor_t * tors) {
    print_spine_stats(spines);
    print_tor_stats(tors);
}

void print_spine_stats(spine_t * spines) {
    int * max_spines = malloc(sizeof(int) * NUM_OF_SPINES);
    double * avg_spines = malloc(sizeof(double) * NUM_OF_SPINES);
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        spine_t spine = spines[i];
        spine_queue_stats_t spine_stat = spine->queue_stat;
        int64_t * histogram = spine_stat.queue_len_histogram;

        double avg_queue_len = 0;
        int max_queue_len = 0;
        int sum = 0;
        int count = 0;
        for (int j = 0; j < SPINE_PORT_BUFFER_LEN+1; j++) {
            int num_in_len = histogram[j];
            sum += num_in_len * j;
            count += num_in_len;
            if (num_in_len > 0) {
                max_queue_len = j;
            }
        }
        avg_queue_len = (double) sum / count;
        max_spines[i] = max_queue_len;
        avg_spines[i] = avg_queue_len;
    }

    printf("Max Queue Len at each Spine:\n[");
    for(int i = 0; i < NUM_OF_SPINES; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%d", max_spines[i]);
    }
    printf("]\n");

    printf("Avg Queue Len at each Spine:\n[");
    for(int i = 0; i < NUM_OF_SPINES; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%0.4f", avg_spines[i]);
    }
    printf("]\n");

    free(max_spines);
    free(avg_spines);
    printf("\n");
}

void print_tor_stats(tor_t * tors) {
    int * up_tor_max = malloc(sizeof(int) * NUM_OF_TORS);
    double * up_tor_avg = malloc(sizeof(double) * NUM_OF_TORS);
    int * down_tor_max = malloc(sizeof(int) * NUM_OF_TORS);
    double * down_tor_avg = malloc(sizeof(double) * NUM_OF_TORS);
    for (int i = 0; i < NUM_OF_TORS; i++) {
        tor_t tor = tors[i];
        tor_queue_stats_t tor_stat = tor->queue_stat;
        int64_t * up_histogram = tor_stat.upstream_queue_len_histogram;
        int64_t * down_histogram = tor_stat.downstream_queue_len_histogram;

        double avg_up_queue_len = 0;
        int max_up_queue_len = 0;
        int sum = 0;
        int count = 0;
        for (int j = 0; j < TOR_UPSTREAM_BUFFER_LEN+1; j++) {
            int num_in_len = up_histogram[j];
            sum += num_in_len * j;
            count += num_in_len;
            if (num_in_len > 0) {
                max_up_queue_len = j;
            }
        }
        avg_up_queue_len = (double) sum / count;

        double avg_down_queue_len = 0;
        int max_down_queue_len = 0;
        sum = 0;
        count = 0;
        for (int j = 0; j < TOR_DOWNSTREAM_BUFFER_LEN+1; j++) {
            int num_in_len = down_histogram[j];
            sum += num_in_len * j;
            count += num_in_len;
            if (num_in_len > 0) {
                max_down_queue_len = j;
            }
        }
        avg_down_queue_len = (double) sum / count;

        up_tor_max[i] = max_up_queue_len;
        up_tor_avg[i] = avg_up_queue_len;
        down_tor_max[i] = max_down_queue_len;
        down_tor_avg[i] = avg_down_queue_len;
    }

    printf("Max Upstream Queue Len at each ToR:\n[");
    for(int i = 0; i < NUM_OF_TORS; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%d", up_tor_max[i]);
    }
    printf("]\n");

    printf("Max Downstream Queue Len at each ToR:\n[");
    for(int i = 0; i < NUM_OF_TORS; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%d", down_tor_max[i]);
    }
    printf("]\n");

    printf("Avg Upstream Queue Len at each ToR:\n[");
    for(int i = 0; i < NUM_OF_TORS; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%0.4f", up_tor_avg[i]);
    }
    printf("]\n");

    printf("Avg Downstream Queue Len at each ToR:\n[");
    for(int i = 0; i < NUM_OF_TORS; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%0.4f", down_tor_avg[i]);
    }
    printf("]\n");

    free(up_tor_max);
    free(up_tor_avg);
    free(down_tor_max);
    free(down_tor_avg);
    printf("\n");
}