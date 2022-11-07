#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <dirent.h>
#include <errno.h>
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
#include "timeseries.h"
#include "memory.h"

void read_tracefile(char * filename);
void initialize_flow(int flow_id, int src, int dst, int flow_size_pkts, int flow_size_bytes, int timeslot);
void open_switch_outfiles(char * base_filename);

#endif