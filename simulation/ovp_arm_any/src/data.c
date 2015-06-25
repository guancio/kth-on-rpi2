/*
 * data 
 */

#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "icm/icmCpuManager.h"

struct data_header {
    unsigned long start;
    unsigned long size;
    char name[64];
};

struct data_context {
    struct data_header header;
    FILE *fp;
    icmProcessorP proc;    
};

/* -------------------------------------- */
static void data_get_name(char *output, const char *input)
{
    sprintf(output, "ovp_%s.data", input);    
}

static FILE *data_get_file(const char *name, int write)
{
    char buffer[1024];
    data_get_name(buffer, name);
    return fopen(buffer, write ? "wb" : "rb" );
}

static void data_init(struct data_context *ctx, icmProcessorP proc, const char *name)
{
    strcpy(ctx->header.name, name);
    ctx->proc = proc;
}
static void data_end(struct data_context *ctx)
{
    fclose(ctx->fp);    
    
}
static int data_open_write(struct data_context *ctx, int adr, int size  )
{
    char buffer[1024];
    
    data_get_name(buffer, ctx->header.name);
    ctx->fp = fopen(buffer, "wb");
    if(ctx->fp) {
        // write header
        ctx->header.start = adr;
        ctx->header.size = size;
        fwrite ( &ctx->header, sizeof(ctx->header), 1, ctx->fp);        
        return 1;
    }
    return 0;
}

static int data_open_read(struct data_context *ctx )
{
    char buffer[1024];
    
    data_get_name(buffer, ctx->header.name);
    ctx->fp = fopen(buffer, "rb");
    if(ctx->fp) {
        fread ( & (ctx->header), sizeof(ctx->header), 1, ctx->fp);        
        printf("Data %s ADR=0x%08lx SIZE=0x%08lx\n", 
               ctx->header.name, ctx->header.start, ctx->header.size);        
        
        return 1;
    }
    return 0;
}

static int data_write_file(struct data_context *ctx, unsigned int *data)
{
    return fwrite( data, 4, 1, ctx->fp) == 4;    
}

static int data_read_file(struct data_context *ctx, unsigned int *data)
{
    return fread( data, 4, 1, ctx->fp) == 4;    
}

static int data_write_memory(struct data_context *ctx, unsigned int adr, unsigned int *data)
{
    return icmDebugWriteProcessorMemory( ctx->proc, (Addr) (0xFFFFFFFF & adr), 
                                    data, 4, 1, ICM_HOSTENDIAN_HOST);
}

static int data_read_memory(struct data_context *ctx, unsigned int adr, unsigned int *data)
{
    return icmDebugReadProcessorMemory( ctx->proc, (Addr) (0xFFFFFFFF & adr), 
                                    data, 4, 1, ICM_HOSTENDIAN_HOST);
}

/* ---------------------------------------------------------------- */ 
void data_save(icmProcessorP proc, const char *name, int adr, int size)
{
    int i, data, failed = 0;
    struct data_context ctx;
    
    /* align to 4 */
    size += (adr & 3);
    adr = adr & ~3;
    
    data_init(&ctx, proc, name);
    
    
    if(data_open_write(&ctx, adr, size)) {
        for(i = 0; i < size ; i += 4) {
            if(!data_read_memory(&ctx, adr, &data))
                failed = 1;
            data_write_file(&ctx, &data);
            adr += 4;
        }
        data_end(&ctx);
        
        if(failed) {
            printf("NOTE: there were memory access errors. Saved data is invalid\n");
        }        
    }  else printf( "Unable to create '%s' data file\n", name);        
}

void data_load(icmProcessorP proc, const char *name, int adr, int has_adr)
{    
    int i, data, failed = 0;
    struct data_context ctx;    
    
    data_init(&ctx, proc, name);        
    if(data_open_read(&ctx)) {
        
        printf("Loaded %s %08lx %08lx\n", ctx.header.name, ctx.header.start, ctx.header.size);        
        if(!has_adr) adr = ctx.header.start;
        
        for(i = 0; i < ctx.header.size ; i += 4) {
            data_read_file(&ctx, &data);
            if(!data_write_memory(&ctx, adr, &data))
                failed = 1;
            
            adr += 4;
        }
        data_end(&ctx);
        
        if(failed) {
            printf("NOTE: there were memory access errors. Memory may be invalid\n");
        }        
    }  else printf( "Unable to open '%s' data file\n", name);     
}

void data_compare(icmProcessorP proc, const char *name, int adr, int has_adr)
{
    int i, data1, data2, cnt = 0, failed = 0;
    struct data_context ctx;    
    char *symbol;
    
    data_init(&ctx, proc, name);        
    if(data_open_read(&ctx)) {        
        if(!has_adr) adr = ctx.header.start;
        
        for(i = 0; i < ctx.header.size ; i += 4) {
            data_read_file(&ctx, &data1);
            if(!data_read_memory(&ctx, adr, &data2))
                failed = 1;
            else if(data1 != data2 && cnt < 8) {
                printf("Data changed at 0x%08lx:  OLD=0x%08lx  NEW=0x%08lx", adr, data1, data2);                    
                symbol = getSymbolFromAddress(adr);
                if(symbol) printf(" (%s)", symbol);                
                printf("\n");
                    
                cnt ++;                
                if(cnt == 8) printf("... (additional changes not reported)\n");                
            }
            adr += 4;
        }
        data_end(&ctx);
        
        if(cnt > 0) {
            printf(" *** DATA COMPARISION: There were %d changes.\n", cnt);
        } else {
            printf(" *** DATA COMPARISION: Saved data was identical to memory.\n");
        }
    
        if(failed) {
            printf("NOTE: there were memory access errors. Comparision my be incorrect.\n");
        }
    }  else printf( "Unable to open '%s' data file\n", name);     
            
}


void data_delete(const char *name)
{
    char buffer[1024];
    data_get_name(buffer, name);
    unlink(buffer);
}


void data_show(const char *name)
{
    int i, data;
    unsigned int adr;
    struct data_context ctx;    
    
    data_init(&ctx, 0, name);        
    if(data_open_read(&ctx)) {
                
        adr = ctx.header.start;
        for(i = 0; i < ctx.header.size ; i += 4) {
            data_read_file(&ctx, &data);
            if(i == 0 || (adr & 31) == 0) {
                printf("\n%08lx: ", adr);
            }
            printf("%08lx ", data);            
            adr += 4;
        }
        data_end(&ctx);
        printf("\n");
        
    }  else printf( "Unable to open '%s' data file\n", name);         
}
