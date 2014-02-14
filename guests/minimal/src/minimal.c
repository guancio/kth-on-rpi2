
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

uint32_t h_l1_unnmap(uint32_t va) {
  uint32_t res;
  asm("mov  r0, %[value] \n\t"
      :
      :[value] "r" (va)/*input*/
      : /* No clobbers */);
  asm("SWI #1022");
	
  asm("mov  %[result],r0 \n\t"
      :[result] "=r" (res)
      : /*input*/
      : /* No clobbers */);
  return res;
}

uint32_t h_l1_sec_map(uint32_t va, uint32_t pa, uint32_t attrs) {
  uint32_t res;
  asm("mov  r0, %[value] \n\t"
      :
      :[value] "r" (va)/*input*/
      : /* No clobbers */);
  asm("mov  r1, %[value] \n\t"
      :
      :[value] "r" (pa)/*input*/
      : /* No clobbers */);
  asm("mov  r2, %[value] \n\t"
      :
      :[value] "r" (attrs)/*input*/
      : /* No clobbers */);
  asm("SWI #1023");
	
  asm("mov  %[result],r0 \n\t"
      :[result] "=r" (res)
      : /*input*/
      : /* No clobbers */);
  return res;
}

void test_l1_unnmap()
{
  int i, j, k;
    for(i = 0; ;i++ ) {
        printf("test_l1_unnmap, round %d\n", i);
        for(j = 0; j < 500000; j++) asm("nop");

	uint32_t va, res;
	// I can not unmap 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
	res = h_l1_unnmap(va);
	if (res == 1)
	  printf("test 0: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 0: FAIL, add %x, res %d\n", va, res);

	// I can not unmap 0xf0000000, since it is reserved by the hypervisor code
	va = 0xf0000000;
	res = h_l1_unnmap(va);
	if (res == 1)
	  printf("test 1: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 1: FAIL, add %x, res %d\n", va, res);

	// Unmapping 0xc0300000 has no effect, since this page is unmapped
	va = 0xc0300000;
	res = h_l1_unnmap(va);
	if (res == 2)
	  printf("test 2: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 2: FAIL, add %x, res %d\n", va, res);

	// Unmapping 0xc0200000 is ok, since it is the page containing the active page table
	va = 0xc0200000;
	res = h_l1_unnmap(va);
	if (res == 0)
	  printf("test 3: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 3: FAIL, add %x, res %d\n", va, res);

	// Unmapping 0xc0100000 is ok, but the guest will not be able to write in this part of the memory
	// Additional test: the access must work initially
	// k = (*((int*)(0xc0100000)));
	// printf("test 4: PRE OK, I'm still accessing the page\n");
	va = 0xc0100000;
	res = h_l1_unnmap(va);
	if (res == 0)
	  printf("test 4: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 4: FAIL, add %x, res %d\n", va, res);
	// Additional test: the access must fails
	// k = (*((int*)(0xc0100000)));
	// printf("test 4: FAIL, I'm still accessing the page\n");

	// Unmapping 0xc0000000 is ok, but this is the page where the guest code resides
	//printf("test 5: THIS WILL BRAKE THE GUEST\n");
	//va = 0xc0000000;
	//res = h_l1_unnmap(va);
	//if (res == 0)
	//  printf("test 5: SUCCESS, add %x, res %d\n", va, res);
	//else
	// printf("test 5: FAIL, add %x, res %d\n", va, res);
		
        // I can not map 0, since it is reserved by the hypervisor to access the guest page tables
	va = 0x0;
        uint32_t pa, attrs;
        pa = 0x0;
 	attrs = 0x0;
	res = h_l1_sec_map(va, pa, attrs);
	if (res == 1)
	  printf("test 6: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 6: FAIL, add %x, res %d\n", va, res);

	// I can not map 0xf0000000, since it is reserved by the hypervisor code
	va = 0xf0000000;
	pa = 0x0;
 	attrs = 0x0;
	res = h_l1_sec_map(va, pa, attrs);
	if (res == 1)
	  printf("test 7: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 7: FAIL, add %x, res %d\n", va, res);

	// mapping 0xc0200000 is ok, since it is the page containing the active page table
        // This test should fail, because we are not allowed to map in a physical address outside the guest allowed range
	va = 0xc0200000;
	pa = 0x0;
	attrs = 0x0;
	res = h_l1_sec_map(va, pa, attrs);
	if (res == 3)
	  printf("test 9: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 9: FAIL, add %x, res %d\n", va, res);
	
        // mapping 0xc0000000 is ok, since it is the page containing the active page table
        // This test should fail, because this is already mapped
	va = 0xc0000000;
	pa = 0x81000000;
	attrs = 0x0;
	res = h_l1_sec_map(va, pa, attrs);
	if (res == 4)
	  printf("test 10: SUCCESS, add %x, res %d\n", va, res);
	else
	  printf("test 10: FAIL, add %x, res %d\n", va, res);


        printf("test_l1_unnmap, continued...\n");
        for(j = 0; j < 500000; j++) asm("nop");
    }
}



void task1()
{
  int i, j, k;
  // ok now the guest fail to read the second mapping
     //(*((int*)(0x00100000))) = 666;
    for(i = 0; ;i++ ) {
        printf("TASK 1, round %d\n", i);
	k = (*((int*)(0xc0100000)));
	// correctly abort
	//k = (*((int*)(0xc0500000)));
        printf("VALUE 1, round %d\n", k);
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
    test_l1_unnmap();
    //Will never reach here
    for(;;)
        ;
}

/*Each guest must provide a handler rpc*/
void handler_rpc(unsigned callNum, void *params)
{
}
