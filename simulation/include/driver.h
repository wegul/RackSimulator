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
void initialize_flow(int flow_id, int isMemFlow, int memType, int src, int dst, int flow_size_bytes, int rreq_bytes, int timeslot);
int comp(const void *elem1, const void *elem2);
int cmp_link(const void *elem1, const void *elem2);
void open_switch_outfiles(char *base_filename);
void open_host_outfiles(char *base_filename);
int max_priority(int a, int b)
{
    // Return the value with higher priority
    // Priority 0>999>1>2>-1>998
    if (a == 0)
    {
        return a;
    }
    else if (a == 999 && b != 0)
    {
        return a;
    }
    else if (a == 1 && b != 0 && b != 999)
    {
        return a;
    }
    else if (a == 2 && b != 0 && b != 999 && b != 1)
    {
        return a;
    }
    else if (a == -1 && b == 998)
    {
        return a;
    }
    else if (a == 998 && b == 998)
    {
        return a;
    }

    return b;
};

#endif