
/*
 * minimal test framework
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "test.h"


// ----------------------------------------


// ----------------------------------------
void fail(const char *msg)
{
    if(msg) fprintf(stderr, "FAILED: '%s'\n", msg);
    exit(20);
}

// ----------------------------------------

void test_init(test_context_t * ctx)
{
    ctx->errors_total = 0;
    ctx->errors_group = 0;
    ctx->tests_total = 0;
    ctx->tests_group = 0;
    ctx->count_group = 0;
    ctx->group_name = 0;
}

void test_mark(test_context_t * ctx, char *msg)
{
    printf("- %s\n"
           "-----------------------------------------------------\n",
           msg);
}

void test_cleanup(test_context_t * ctx)
{
    if(ctx->tests_total == 0) 
        fail("No tests where perfoemed");
    else if(ctx->errors_total > 0) {
        fprintf(stderr, "FAILED: %d tests in %d gorups. %d errors where found\n",
                ctx->tests_total, ctx->count_group, ctx->errors_total);
        
        fail(0);
    } else {
        /* all fine and dandy... */
        
        
        if(ctx->errors_total > 0) {
            printf("   [ TEST SUMMARY: %d groups, %d test (FAILED: %d) ]\n",
                   ctx->count_group, ctx->tests_total, ctx->errors_total);                                  
        } else {
            printf("   [ TEST SUMMARY: %d groups, %d test OK ]\n",
                   ctx->count_group, ctx->tests_total);              
        }
        printf("\n");
        
    }
}

void context_report_group(test_context_t * ctx)
{
    if(ctx->tests_group == 0) {
        fail("No tests were performed in this group");
    } else if(ctx->errors_group > 0) {
        fprintf(stderr, "  %03d ERRORS in %03d tests in '%s'\n",
                ctx->errors_group, ctx->tests_group, ctx->group_name);        
    } else {
        /* no probelms */        
        fprintf(stderr, "   GROUP '%s': all %03d test%s ok\n", 
                ctx->group_name, 
                ctx->tests_group,
                ctx->tests_group > 1 ? "s" : ""
                );
                
    }
}




void test_update(test_context_t *ctx, const char *msg, int succ, 
                 const char *filename, int line, 
                 const char *format, ...)
{
    va_list args;
    
    if(!ctx->group_name) fail("Test does not belong to any group");
    
    va_start(args, format);

    ctx->tests_total++;
    ctx->tests_group++;
    
    
    if(succ) {
        /* yes, this is the opposite of murphys law... */        
    } else {
        ctx->errors_total++;
        ctx->errors_group++;        
        
        fprintf(stderr, " ERROR %d: %s:%d: ", ctx->errors_total, filename, line);
        vfprintf(stderr, format, args);
        
        fprintf(stderr, ". %s\n", msg);        
    }
    
    
    va_end(args);
    
}



void test_group_start(test_context_t * ctx, char *name)
{
    if(ctx->group_name) fail("Already in a test group");
    
    ctx->count_group++;    
    ctx->errors_group = 0;
    ctx->tests_group = 0;    
    ctx->group_name= name;
}
void test_group_end(test_context_t * ctx)
{
    if(!ctx->group_name) fail("Not in a test group");
    context_report_group(ctx);
    ctx->group_name = 0;
}

