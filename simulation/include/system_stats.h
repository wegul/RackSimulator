#ifndef __SYSTEM_STATS_H__
#define __SYSTEM_STATS_H__

#include "params.h"
#include "tor.h"
#include "spine.h"
#include "flow.h"

void print_system_stats(spine_t * spines, tor_t * tors, int64_t bytes, int64_t pkts, int64_t ns, int64_t cache_misses, int64_t cache_hits, float avg_delay_at_spine, float avg_max_delay_at_spine, float timeslot_len);
void print_network_tput(int64_t bytes, int64_t pkts, int64_t ns);
void print_cache_stats(int64_t cache_misses, int64_t cache_hits);
void print_spine_stats(spine_t * spines);
void print_tor_stats(tor_t * tors);
void print_delay_stats(float avg_delay_at_spine, float avg_max_delay_at_spine, float timeslot_len);
FILE * open_outfile(char * filename);
void write_to_outfile(FILE * fp, flow_t * flow, int timeslot_len, int bandwidth);
FILE * open_timeseries_outfile(char * filename);
void write_to_timeseries_outfile(FILE * fp, spine_t * spines, tor_t * tors, int64_t final_timeslot, int64_t num_datapoints);

#endif