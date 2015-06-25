#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>


#include "icm/icmCpuManager.h"

/***********************************************************
 * command history
 ***********************************************************/

typedef struct command_history_ {
    int id;
    struct command_history_ *next;
    char cmd[0];
} command_history_t;

static int history_cnt = 0, history_id;
static command_history_t *history_root = 0, *history_last = 0;

void addHistory(char *cmd)
{
    command_history_t *ch = malloc( sizeof(command_history_t) + strlen(cmd) + 1);
    
    /* dont grow too much */
    if(history_cnt++ > 50) {        
        command_history_t *tmp = history_root;
        history_root = tmp->next;
        free(tmp);
    }
    
    ch->id = ++history_id;
    ch->next = 0;
    strcpy(ch->cmd, cmd);
    
    if(history_last) {
        history_last->next = ch;
        history_last = ch;
    } else {
        history_last = history_root = ch;
    }        
}

command_history_t *findHistory(int id)
{
    command_history_t *tmp = history_root;
    while(tmp) {
        if(tmp->id == id) return tmp;
        tmp = tmp->next;
    }
    return 0;
}

command_history_t *lastHistory()
{
    return history_last;
}

void showHistory()
{
    command_history_t *tmp = history_root;    
    while(tmp) {
        printf("%3d: %s\n", tmp->id, tmp->cmd);
        tmp = tmp->next;
    }
}

void executeHistory(icmProcessorP processor, char *cmd)
{
    int id;
    command_history_t *his;
    
    if(cmd == 0 || *cmd == '\0') {
        showHistory();        
    } else if(!strcmp(cmd, "!")) {
        if( his = lastHistory() ) {
            executeCommand(processor, his->cmd);
        } else {
            printf("ERROR: no previous command found\n");
        }                    
    } else {
        if(!getNumberFromString(cmd, & id)) {
            if( his = findHistory(id) ) {
                executeCommand(processor, his->cmd);
            } else {
                printf("ERROR: could not find history '%d'\n", id);
            }            
        } else {
            printf("ERROR: Bad number: '%s'\n", cmd);                                        
        }                
    }
}

void historySave(const char *arg1)
{    
    if(history_root) {
        FILE *fp = fopen(arg1 ? arg1 : "debugger.his", "wb");
        if(fp) {        
            command_history_t *tmp;            
            for(tmp = history_root; tmp; tmp = tmp->next)
                fprintf(fp, "%s\n", tmp->cmd);
                
            printf("History saved\n");
            fclose(fp);    
        } else fprintf(stderr, "ERROR: unable to open history file for writing!\n");
    } else fprintf(stderr, "WARNING: found no history to save!\n");
}

void historyReplay(icmProcessorP processor, const char *arg1)
{
    FILE *fp;
    char *filename = arg1 ? arg1 : "debugger.his";
    
    fp= fopen(filename, "rb");
    if(fp) {        
        char *tmp, buffer[1024];
        
        while( fgets(buffer, 1024-1, fp)) {
            buffer[1024-1] = '\0';
            
            
            tmp = trimString(buffer);
            if(!tmp) continue; /* empty string */
            
            if(executeCommand(processor, tmp))
                break;
        }
        fclose(fp);    
    } else fprintf(stderr, "ERROR: unable to open history file '%s' for reading!\n", filename);
}    

