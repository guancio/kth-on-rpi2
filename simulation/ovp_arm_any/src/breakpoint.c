/*
 * breakpoints
 */

#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "icm/icmCpuManager.h"

#define BREAKPOINT_COUNT 64
struct breakpoint{
    int adr;
    struct breakpoint *next, *prev;
};

static struct breakpoint *bp_root = 0;

struct breakpoint *findBreakpoint(int adr)
{
    struct breakpoint *tmp = bp_root;
    while(tmp) {
        if(tmp->adr == adr) return tmp;
        tmp = tmp->next;
    }
    return 0;
}

void listAllBreakpoints()
{
    struct breakpoint *tmp = bp_root;
    int i = 1;
    
    if(tmp) {
        printf("Current breakoints are\n");
        
        while(tmp) {
            char *symbol = getSymbolFromAddress(tmp->adr);
            if(symbol)                
                printf("  %d  - 0x%08lx - %s\n", i, tmp->adr, symbol);
            else
                printf("  %d  - 0x%08lx\n", i, tmp->adr);
            i++;
            tmp = tmp->next;
        }
        printf("\n");
    } else {
        printf("No breakpoints were set\n");
    }
}

int isBreakpoint(int adr)
{
    return findBreakpoint(adr) != 0;
}

int removeBreakpoint(icmProcessorP processor, int adr)
{
    struct breakpoint *bp = findBreakpoint(adr);    
    if(!bp) {
        printf("INTERNAL ERROR: could not find breakpoint at 0x%08lx\n", adr);
        return 0;
    }
    
    icmClearAddressBreakpoint(processor, (Addr) ( 0xFFFFFFFF & adr));
    
    if(bp->next) bp->next->prev = bp->prev;
    if(bp->prev) bp->prev->next = bp->next;
    else bp_root = bp->next;
    
    free(bp);
    return 1;
}

int setBreakpoint(icmProcessorP processor, int adr)
{    
    struct breakpoint *bp = findBreakpoint(adr);    
    if(bp) {
        printf("Breakpoint at 0x%08lx already exists, ignoring...\n", adr);
        return 0;
    }
    
    bp =  malloc( sizeof(struct breakpoint));
    if(!bp) return 0;
    
    icmSetAddressBreakpoint(processor, (Addr) ( 0xFFFFFFFF & adr));            
    bp->adr = adr;
    bp->next = bp_root;
    bp->prev = 0;
    if(bp_root) bp_root->prev = bp;
    bp_root = bp;
    return 1;
}

