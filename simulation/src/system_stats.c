#include "system_stats.h"

// void print_system_stats(spine_t *spines, tor_t *tors, int64_t total_bytes, int64_t total_pkts, int64_t ns, int64_t cache_misses, int64_t cache_hits, float avg_delay_at_spine, float avg_max_delay_at_spine, float timeslot_len)
// {
//     // print_spine_stats(spines);
//     // print_tor_stats(tors);
//     print_network_tput(total_bytes, total_pkts, ns);
//     // print_cache_stats(cache_misses, cache_hits);
//     print_per_cache_stats(spines, tors);
//     print_delay_stats(avg_delay_at_spine, avg_max_delay_at_spine, timeslot_len);
// }

void print_network_tput(int64_t total_bytes, int64_t total_pkts, int64_t ns)
{
    printf("Total bytes received: %ld B\n", total_bytes);
    printf("Full Network Throughput: %0.2f Gbps\n", (double)total_bytes * 8 / ns);
    printf("Packets per ns: %0.2f ppns\n", (double)total_pkts / ns);
}

// void print_cache_stats(int64_t cache_misses, int64_t cache_hits)
// {
//     printf("Number of cache accesses: %ld\n", cache_misses + cache_hits);
//     printf("Number of cache hits: %ld\n", cache_hits);
//     printf("Number of cache misses: %ld\n", cache_misses);
//     if (cache_misses + cache_hits > 0)
//     {
//         printf("Percent of cache access misses: %0.2f%%\n", (double)cache_misses * 100 / (cache_misses + cache_hits));
//     }
// }

// void print_per_cache_stats(spine_t *spines, tor_t *tors)
// {
//     printf("Per-Spine SRAM cache misses: [%ld", spines[0]->cache_misses);
//     for (int i = 1; i < NUM_OF_SPINES; i++)
//     {
//         printf(", %ld", spines[i]->cache_misses);
//     }
//     printf("]\n");
//     printf("Per-ToR SRAM cache misses: [%ld", tors[0]->cache_misses);
//     for (int i = 1; i < NUM_OF_RACKS; i++)
//     {
//         printf(", %ld", tors[i]->cache_misses);
//     }
//     printf("]\n");
// }

// void print_tor_stats(tor_t * tors) {
//     for (int i = 0; i < NUM_OF_TORS; i++) {
//         printf("Max Upstream Queue Lens at Tor %d:\n[", i);
//         for(int j = 0; j < NUM_OF_SPINES; j++) {
//             if (j != 0) {
//                 printf(", ");
//             }
//             printf("%d", (int) tors[i]->upstream_queue_stat[j]->max_val);
//         }
//         printf("]\n");

//         printf("Max Downstream Queue Lens at Tor %d:\n", i);
//         for(int j = 0; j < NODES_PER_RACK; j++) {
//             if (j != 0) {
//                 printf(", ");
//             }
//             printf("%d", (int) tors[i]->downstream_queue_stat[j]->max_val);
//         }
//         printf("]\n\n");
//     }
// }

void print_delay_stats(float avg_delay_at_spine, float avg_max_delay_at_spine, float timeslot_len)
{
    printf("Avg delay at spine: %0.3f timeslots %0.3f ns\n", avg_delay_at_spine, (avg_delay_at_spine * timeslot_len));
    printf("Avg max delay at spine: %0.3f timeslots %0.3f ns\n", avg_max_delay_at_spine, (avg_max_delay_at_spine * timeslot_len));
}

FILE *open_outfile(char *filename)
{
    char out_filename[520] = "out/";
    strncat(out_filename, filename, 500);
    FILE *out_fp = fopen(out_filename, "w");
    assert(out_fp != NULL);
    fprintf(out_fp, "FlowID,FlowType,Src,Dst,FlowSize,Create,NotifTime,GrantTime,Start,Finish,FCT,Slowdown,Xput\n");
    return out_fp;
}

void write_to_outfile(FILE *fp, flow_t *flow, float timeslot_len, int bandwidth)
{
    assert(fp != NULL);
    double sec_per_timeslot = timeslot_len / 1e9;

    int flow_id = (int)flow->flow_id;
    int src = (int)flow->src;
    int dst = (int)flow->dst;
    int bytes_received = (int)flow->bytes_received;
    // double flow_completion = (double)flow->finish_timeslot * sec_per_timeslot;
    int start_time = flow->start_timeslot;
    int finish_time = flow->finish_timeslot;
    int flow_completion = flow->finish_timeslot - flow->timeslot;
    double slowdown = (double)(flow_completion) / (double)(flow->expected_runtime);
    double tput = (double)flow->bytes_received * 8 / (flow->timeslots_active * timeslot_len * 1.0);
    fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%.2f,%.2f\n",
            flow_id, flow->flowType, src, dst, bytes_received, flow->timeslot, flow->notifTime, flow->grantTime, start_time, finish_time, flow_completion, slowdown, tput);
    fflush(fp);
}

// FILE * open_timeseries_outfile(char * filename) {
//     char out_filename[520] = "out/";
//     strncat(out_filename, filename, 500);
//     FILE * timeslot_fp = fopen(out_filename, "w");
//     assert(timeslot_fp != NULL);
//     return timeslot_fp;
// }

// void write_to_timeseries_outfile(FILE * fp, spine_t * spines, tor_t * tors, int64_t final_timeslot, int64_t num_datapoints) {
//     int ts_per_datapoint = final_timeslot / num_datapoints;
//     if (ts_per_datapoint == 0) {
//         ts_per_datapoint = 1;
//     }

//     for (int i = 0; i < final_timeslot; i += ts_per_datapoint) {
//         for (int j = 0; j < NUM_OF_SPINES; j++) {
//             spine_t spine = spines[j];
//             for (int k = 0; k < SPINE_PORT_COUNT; k++) {
//                 int len = (int) spine->queue_stat[k]->list[i];
//                 fprintf(fp, "%d,", len);
//             }
//         }

//         for (int j = 0; j < NUM_OF_TORS; j++) {
//             tor_t tor = tors[j];
//             for(int k = 0; k < NUM_OF_SPINES; k++) {
//                 int len = (int) tor->upstream_queue_stat[k]->list[i];
//                 fprintf(fp, "%d,", len);
//             }

//             for (int k = 0; k < NODES_PER_RACK; k++) {
//                 int len = tor->downstream_queue_stat[k]->list[i];
//                 fprintf(fp, "%d,", len);
//             }
//         }

//         fprintf(fp, "\n");
//     }
// }