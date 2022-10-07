#include "system_stats.h"

void print_system_stats(spine_t * spines, tor_t * tors) {
    print_spine_stats(spines);
    print_tor_stats(tors);
}

void print_spine_stats(spine_t * spines) {
    for (int i = 0; i < NUM_OF_SPINES; i++) {
        printf("Max Queue Lens at Spine %d:\n[", i);
        for (int j = 0; j < SPINE_PORT_COUNT; j++) {
            if (j != 0) {
                printf(", ");
            }
            printf("%d", (int) spines[i]->queue_stat[j]->max_val);
        }
        printf("]\n\n");
    }
}

void print_tor_stats(tor_t * tors) {
    for (int i = 0; i < NUM_OF_TORS; i++) {
        printf("Max Upstream Queue Lens at Tor %d:\n[", i);
        for(int j = 0; j < NUM_OF_SPINES; j++) {
            if (j != 0) {
                printf(", ");
            }
            printf("%d", (int) tors[i]->upstream_queue_stat[j]->max_val);
        }
        printf("]\n");

        printf("Max Downstream Queue Lens at Tor %d:\n[", i);
        for(int j = 0; j < NODES_PER_RACK; j++) {
            if (j != 0) {
                printf(", ");
            }
            printf("%d", (int) tors[i]->downstream_queue_stat[j]->max_val);
        }
        printf("]\n\n");
    }
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

FILE * open_timeseries_outfile(char * filename) {
    FILE * timeslot_fp = fopen(filename, "w");
    assert(timeslot_fp != NULL);
    return timeslot_fp;
}

void write_to_timeseries_outfile(FILE * fp, spine_t * spines, tor_t * tors, int64_t final_timeslot, int64_t num_datapoints) {
    int ts_per_datapoint = final_timeslot / num_datapoints;
    if (ts_per_datapoint == 0) {
        ts_per_datapoint = 1;
    }

    for (int i = 0; i < final_timeslot; i += ts_per_datapoint) {
        for (int j = 0; j < NUM_OF_SPINES; j++) {
            spine_t spine = spines[j];
            for (int k = 0; k < SPINE_PORT_COUNT; k++) {
                int len = (int) spine->queue_stat[k]->list[i];
                fprintf(fp, "%d,", len);
            }
        }

        for (int j = 0; j < NUM_OF_TORS; j++) {
            tor_t tor = tors[j];
            for(int k = 0; k < NUM_OF_SPINES; k++) {
                int len = (int) tor->upstream_queue_stat[k]->list[i];
                fprintf(fp, "%d,", len);
            }

            for (int k = 0; k < NODES_PER_RACK; k++) {
                int len = tor->downstream_queue_stat[k]->list[i];
                fprintf(fp, "%d,", len);
            }
        }

        fprintf(fp, "\n");
    }
}