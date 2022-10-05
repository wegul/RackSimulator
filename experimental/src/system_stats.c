#include "system_stats.h"

void print_system_stats(spine_t * spines, tor_t * tors) {
    print_spine_stats(spines);
    print_tor_stats(tors);
}

void print_spine_stats(spine_t * spines) {
    printf("Spine stats: (spine_id : avg, max)\n");
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
        avg_queue_len = sum / count;
        printf("%d : %0.3f, %d\n", i, avg_queue_len, max_queue_len);
    }
    printf("\n");
}

void print_tor_stats(tor_t * tors) {
    printf("ToR stats: (tor_id : up_avg, up_max, down_avg, down_max)\n");
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
        avg_up_queue_len = sum / count;

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
        avg_down_queue_len = sum / count;

        printf("%d : %0.3f, %d, %0.3f, %d\n", i, avg_up_queue_len, max_up_queue_len, avg_down_queue_len, max_down_queue_len);
    }
    printf("\n");
}