#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "params.h"
#include "arraylist.h"
#include "buffer.h"
#include "flow.h"
#include "flowlist.h"
#include "link.h"
#include "packet.h"
#include "node.h"
#include "tor.h"
#include "spine.h"
#include "links.h"
#include "system_stats.h"

void read_tracefile(char * filename);
void initialize_flow(int flow_id, int src, int dst, int flow_size_pkts, int timeslot);


#endif