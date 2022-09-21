#ifndef __SYS_STATS_H__
#define __SYS_STATS_H__

#include "params.h"
#include "node.h"
#include "packet.h"
#include "flow.h"
#include "spine.h"
#include "tor.h"

void update_pkt_counters(node_t, packet_t);
void update_stats_on_pkt_recv(node_t, packet_t, int8_t, int8_t);
void print_network_tput(int, int16_t, float);
void log_active_flows(int, int16_t, float, int64_t, int, FILE*);
void print_summary_stats(int, int16_t, float, float, int, int64_t, int, int8_t, FILE*);

#endif
