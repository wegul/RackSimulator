#include "system_stats.h"

void print_system_stats(spine_t * spines, tor_t * tors) {
    print_spine_stats(spines);
    print_tor_stats(tors);
}

void print_spine_stats(spine_t * spines) {
    int * max_spines = malloc(sizeof(int) * NUM_OF_SPINES);
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        spine_t spine = spines[i];
        spine_queue_stats_t spine_stat = spine->queue_stat;
        int64_t * histogram = spine_stat.queue_len_histogram;
        int max_queue_len = 0;
        for (int j = 0; j < MAX_HISTOGRAM_LEN; j++) {
            int num_in_len = histogram[j];
            if (num_in_len > 0) {
                max_queue_len = j;
            }
        }
        max_spines[i] = max_queue_len;
    }

    printf("Max Queue Len at each Spine:\n[");
    for(int i = 0; i < NUM_OF_SPINES; i++) {
        if (i != 0) {
            printf(", ");
        }
        printf("%d", max_spines[i]);
    }
    printf("]\n");

    free(max_spines);
    printf("\n");
}

void print_tor_stats(tor_t * tors) {
    int * up_tor_max = malloc(sizeof(int) * NUM_OF_TORS);
    int * down_tor_max = malloc(sizeof(int) * NUM_OF_TORS);
    for (int i = 0; i < NUM_OF_TORS; i++) {
        tor_t tor = tors[i];
        tor_queue_stats_t tor_stat = tor->queue_stat;
        int64_t * up_histogram = tor_stat.upstream_queue_len_histogram;
        int64_t * down_histogram = tor_stat.downstream_queue_len_histogram;

        int max_up_queue_len = 0;
        for (int j = 0; j < MAX_HISTOGRAM_LEN; j++) {
            int num_in_len = up_histogram[j];
            if (num_in_len > 0) {
                max_up_queue_len = j;
            }
        }

        int max_down_queue_len = 0;
        for (int j = 0; j < MAX_HISTOGRAM_LEN; j++) {
            int num_in_len = down_histogram[j];
            if (num_in_len > 0) {
                max_down_queue_len = j;
            }
        }

        up_tor_max[i] = max_up_queue_len;
        down_tor_max[i] = max_down_queue_len;
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

    free(up_tor_max);
    free(down_tor_max);
    printf("\n");
}

FILE * open_outfile(char * filename) {
    FILE * out_fp = fopen(filename, "w");
    assert(out_fp != NULL);
    fprintf(out_fp, "Flow ID,Src,Dst,Flow Size(pkts),Flow Completion Time(sec),Slowdown(sec),Throughput(Gbps)\n");
    return out_fp;
}

void write_to_outfile(FILE * fp, flow_t * flow, int timeslot_len, int bandwidth) {
    assert(fp != NULL);
    double sec_per_timeslot = timeslot_len / 1e9;

    int flow_id = (int) flow->flow_id;
    int src = (int) flow->src;
    int dst = (int) flow->dst;
    int flow_size = (int) flow->flow_size;
    double flow_completion = (double) flow->finish_timeslot * sec_per_timeslot;
    int timeslots_real = flow->finish_timeslot - flow->timeslot;
    int timeslots_processing = flow->finish_timeslot - flow->start_timeslot;
    int timeslots_ideal = flow->flow_size + 2;
    double slowdown = (double) (timeslots_real - timeslots_ideal) * sec_per_timeslot;
    double tput = (double) bandwidth * ((timeslots_ideal / timeslots_processing));

    fprintf(fp, "%d,%d,%d,%d,%0.9f,%0.9f,%0f\n", flow_id, src, dst, flow_size, flow_completion, slowdown, tput);
}