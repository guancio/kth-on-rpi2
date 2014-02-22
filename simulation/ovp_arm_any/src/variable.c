/*
 * simple variable implementation to be used as memory
 */

#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "icm/icmCpuManager.h"

#define MAX_VARIABLES_COUNT         64
#define MAX_VARIABLE_NAME_LENGTH    48

struct variable {
    char name[MAX_VARIABLE_NAME_LENGTH];
    unsigned long value;
};

struct variable variables[MAX_VARIABLES_COUNT];
int variables_count = 0;

/* ------------------------------------------------------------ */

static struct variable * find(char *name)
{
    int i;
    for(i = 0; i < variables_count; i++)
        if( !strcmp(variables[i].name, name))
            return & variables[i];
    
    return 0;    
}

static void add(char *name, unsigned long value)
{
    if(variables_count >= MAX_VARIABLES_COUNT-1) {
        printf("Too many variables, cannot register '%s'\n", name);
        return 0;
    }
    
    if(strlen(name) >= MAX_VARIABLE_NAME_LENGTH-1) {
        printf("Variable name is too long: '%s'\n", name);
        return 0;        
    }
    
    strcpy(variables[variables_count].name, name);
    variables[variables_count].value = value;
    variables_count++;
}


/* ------------------------------------------------------------ */
void show(struct variable *v)
{
    printf("= %s 0x%08lX\n", v->name, v->value);  
}

void doShowVariables()
{
    int i;
    
    printf("Variable list:\n");
    
    for(i = 0; i < variables_count; i++)
        show( & variables[i]);
    
    printf("\n");
}


void doDefineVariable(icmProcessorP processor)
{
    int value;
    
    /* show all */
    if(!params_has(1)) {
        doShowVariables();
        return;
    }
    
    char *name = params_next(0);
    
    if(params_has(1) ) {
        char *value = params_next(0);
        
        /* set this */
        if(getNumber(processor, value, &value)) {
            struct variable * tmp = find(name);
            if(tmp) tmp->value = value;
            else add(name, value);
        } else {
            printf("Could not find the value of '%s'\n", value);
        }        
    } else {
        /* show this */        
        struct variable * tmp = find( name);
        if(tmp) show(tmp);
        else printf("Unknown varialbe '%s'\n", name);
        return;
    }    
}

int getNumberFromVariable(char *name, int *result)
{
    struct variable * tmp = find(name);
    if(tmp) {
        *result = tmp->value;
        return 1;
    }
    
    return 0;
}
