
#include <lib.h>
#include <types.h>
#include "hyper_config.h"

// for our minimal context system

/* CPU context */
typedef struct context_
{
    uint32_t reg[13];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} context;

context tasks[4];
uint32_t stacks[1024 * 4];
int current_task;


void task1()
{
    int i, j;
    for(i = 0; ;i++ ) {
        printf("TASK 1, round %d\n", i);
        for(j = 0; j < 500000; j++) asm("nop");

        asm("mov r0, #1");
        //asm("SWI #0");

        printf("TASK 1, continued...\n");
        for(j = 0; j < 500000; j++) asm("nop");
//        for(;;); // DEBUG

    }
}

void task2()
{
    int i, j;
    for(i = 0; ; i ++ ) {
        printf("TASK 2, round %d\n", i);

        for(j = 0; j < 100000; j++) asm("nop");

        asm("mov r0, #2");
        //asm("SWI #1");

        //asm("SWI #0");

        printf("TASK 2, continued...\n");
        for(j = 0; j < 10000; j++); asm("nop");

    }
}

void task3()
{
    unsigned int *invalid_data = (unsigned int *) 0x02000000;
    //cpu_callback invalid_prog = (cpu_callback) invalid_data;

    int i, j;
    for(i = 0; ; i ++ ) {
        printf("TASK 3, round %d\n", i);

        for(j = 0; j < 80000; j++); asm("nop");

        asm("mov r0, #3");
        //asm("SWI #0");

        printf("TASK 3, continued...\n");
        for(j = 0; j < 50000; j++) asm("nop");


        // now do invalid memory access:
        printf("TASK 3: invalid data access start..\n");
        *invalid_data = 32;
        j = *invalid_data;
        printf("TASK 3: invalid data access end..\n");
        asm(".word 0xDEADDEAD");

//        for(;;);
    }
}

void setup_tasks()
{
    int i, j;
    printf("Setting up initial task\n");
    for(j = 0; j < 1024 * 4; j++)
        stacks[j] = 0xDEAD0000 + j;

    for(i = 0; i < 4; i++)  {
        for(j = 0; j < 13; j++)
            tasks[i].reg[j] =  0xC0DE000 | (i << 8) | j;
        tasks[i].sp = (uint32_t)&stacks[(4-i) * 1024];
        tasks[i].lr = 0;
        tasks[i].psr = 16; // user mode
    }

    tasks[0].pc = (uint32_t)&task1;
    tasks[1].pc = (uint32_t)&task2;
    tasks[2].pc = (uint32_t)&task3;

    //    printf("%x %x %x\n", tasks[0].pc, tasks[1].pc, tasks[2].pc);
    //    for(;;);

}


void _main()
{

//    int i;

    printf("Inside guest!\n");
    printf("Registrering guest tick handler\n");

#define TEST_TASKS
#define TEST_SWI

#ifdef TEST_TASKS
    // task stuff:
    setup_tasks();
#endif

#ifdef TEST_SWI
    printf("Performing software interrupt...\n");
    asm("mov r0, #0x00");
    asm("mov r1, #0x01");
    asm("mov r2, #0x02");
    asm("mov r3, #0x03");
    asm("mov r4, #0x04");
    asm("mov r5, #0x05");
    asm("mov r6, #0x06");
    asm("mov r7, #0x07");
    asm("mov r8, #0x08");
    asm("mov r9, #0x09");
    asm("mov r10, #0x10");
    asm("mov r11, #0x11");
    asm("mov r12, #0x12");

    asm("mov R0, #0");
    asm("mov R1, #1");
    asm("mov R2, #2");
    asm("mov R3, #3");

//    asm("swi #0x123");

    asm("mov R0, #10");
    asm("mov R1, #11");
    asm("mov R2, #12");
    asm("mov R3, #13");
#endif
    //Start task1 to begin with, scheduler take care of rest
    task1();
    //Will never reach here
    for(;;)
        ;
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
}
