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
#include "links.h"
#include "system_stats.h"
#include "timeseries.h"
#include "memory.h"

void read_tracefile(char *filename);
void initialize_flow(int flow_id, int flowType, int src, int dst, int flow_size_bytes, int rreq_bytes, int timeslot);
int comp(const void *elem1, const void *elem2);
int cmp_ntf(const void *a, const void *b);
void open_switch_outfiles(char *base_filename);
void open_host_outfiles(char *base_filename);
int calculate_priority(flow_t *flow)
{
    if (flow->finished)
    {
        return -1;
    }

    if (flow->flowType == RREQ_TYPE && flow->bytes_sent == 0) // Check token
    {
        node_t node = nodes[flow->src];
        if (node->tokenArr[flow->dst] == 0) // No token
        {
            return -1;
        }
    }

    return (flow->flowType * flow->grantState);
};

#endif