/*
 * breakpoints
 */

#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "icm/icmCpuManager.h"

#define WATCHPOINT_COUNT 64

icmWatchPointP watchpoints[WATCHPOINT_COUNT];
int watchpoints_cnt = 0;

/* -------------------------------------------------- */

static void add_watchpoint(icmWatchPointP w)
{
    if(watchpoints_cnt >= WATCHPOINT_COUNT -1) {
        printf("No more watchpoints available\n");
        return 0;
    }
    
    watchpoints[watchpoints_cnt++] = w;
}

static void del_watchpoint(icmWatchPointP w)
{
    int i, j;
    
    for(i = 0, j = 0; i < watchpoints_cnt; i++) {
        i++;
        if(watchpoints[i] != w) j++;
        if(i != j) watchpoints[j] = i;
    }
    watchpoints_cnt = j;
}

static icmWatchPointP find_watchpoint(Addr start, Addr end)
{
    int i;
    
    for(i = 0;  i < watchpoints_cnt; i++)
        if(icmGetWatchPointLowAddress(watchpoints[i]) == start && 
           icmGetWatchPointHighAddress(watchpoints[i]) == end)
            return watchpoints[i];
    return 0;
}

static void print_watchpoint(icmWatchPointP w, int active)
{
    printf("Watch %08lx-%08lx type=%08lx\n",
           icmGetWatchPointLowAddress(w),
           icmGetWatchPointHighAddress(w),
           icmGetWatchPointType(w)
           );
           
}
/* -------------------------------------------------- */
void listAllWatchpoints()
{
    int i;
    
    printf("Current watchpoints are:\n");
    
    for(i = 0;  i < watchpoints_cnt; i++) {
        printf(" %d - ", i + 1);
        print_watchpoint(watchpoints[i], 0);
    }
    
    printf("\n");
}
int setWatchpoint(icmProcessorP processor, int start, int end, 
                  int write_access, int physical)
{    
    icmWatchPointP x;
    Bool phy = physical ? True : False;
    
    Addr low  = 0xFFFFFFFF & (Addr) start;
    Addr high = 0xFFFFFFFF & (Addr) end;
    
    // is this a delete operation?
    x = find_watchpoint(low, high);
    if(x)  {
        printf("Deleting watchpoint ");
        print_watchpoint(x, 0);
        
        icmDeleteWatchPoint(x);
        del_watchpoint(x);
        return;
    }    

    if(write_access)
        x = icmSetProcessorAccessWatchPoint(processor, phy, low, high, 0, 0);
    else
        x = icmSetProcessorReadWatchPoint(processor, phy, low, high, 0, 0);
    
    if(!x) {
        printf("Could not set watchpoint\n");
        return;
    }
    
    add_watchpoint(x);    
    print_watchpoint(x, 0);
}

void processWatchpoints(icmProcessorP processor)
{
    icmWatchPointP x;
    int first = 0;
    
    for(;;) {
        x = icmGetNextTriggeredWatchPoint();
        if(!x) break;
        
        if(first) {
            printf("Break by watchpoint:\n");
            first = 0;
        }
        
        print_watchpoint(x, 1);
        icmResetWatchPoint(x);
        
    }
}
