#ifndef __SYSTEM_STATS_H__
#define __SYSTEM_STATS_H__

#include "params.h"
#include "tor.h"
#include "spine.h"
#include "flow.h"

void print_system_stats(spine_t * spines, tor_t * tors);
void print_spine_stats(spine_t * spines);
void print_tor_stats(tor_t * tors);
FILE * open_outfile(char * filename);
void write_to_outfile(FILE * fp, flow_t * flow, int timeslot_len, int bandwidth);

#endif