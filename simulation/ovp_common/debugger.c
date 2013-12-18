#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>


#include "icm/icmCpuManager.h"

#define MAX_CYCLES 0x0FFFFFFF
#define MEMORY_DUMP_SIZE 32

extern void createPlatform(int debug, int verbose, icmProcessorP *cpu_, icmBusP *bus_) ;

extern void doLoadSymbols(const char *filename);

extern int getAddressFromSymbol(char *str, int *result);
extern char *getSymbolFromAddress(int adr);
extern char *symbol_in_instruction(icmProcessorP processor, Uns32 adr);


/***********************************************************/
#define REGISTER_INDEX_CPSR 16
#define REGISTER_INDEX_SPSR 17
#define REGISTER_COUNT_NORMAL 16
#define REGISTER_COUNT_ALL (17 + 24)
#define REGISTERS_BANKED_START (REGISTER_INDEX_SPSR + 1)
#define REGISTERS_BANKED_END REGISTER_COUNT_ALL

static char *reg_names [] = {
    "R0", "R1", "R2", "R3", "R4","R5", "R6", "R7", "R8", "R9",
    "R10", "R11", "R12", "SP", "LR", "PC", "CPSR", "SPSR",
    
    "SP_USR", "LR_USR", "SPSR_USR",
    "SP_SVC", "LR_SVC", "SPSR_SVC",
    "SP_ABT", "LR_ABT", "SPSR_ABT",
    "SP_UNDEF", "LR_UNDEF", "SPSR_UNDEF",
    "SP_IRQ", "LR_IRQ", "SPSR_IRQ",        
    "SP_FIQ", "LR_FIQ", "SPSR_FIQ",
    "R8_FIQ", "R9_FIQ", "R10_FIQ", "R11_FIQ", "R12_FIQ",
};

static char *reg_names_lower [] = {
    "r0", "r1", "r2", "r3", "r4","r5", "r6", "r7", "r8", "r9",
    "r10", "r11", "r12", "sp", "lr", "pc", "cpsr", "spsr",
    
    "sp_usr", "lr_usr", "spsr_usr",
    "sp_svc", "lr_svc", "spsr_svc",
    "sp_abt", "lr_abt", "spsr_abt",
    "sp_undef", "lr_undef", "spsr_undef",
    "sp_irq", "lr_irq", "spsr_irq",        
    "sp_fiq", "lr_fiq", "spsr_fiq",
    "r8_fiq", "r9_fiq", "r10_fiq", "r11_fiq", "r12_fiq",
};

static Uns32 reg_values[REGISTER_COUNT_ALL] = { -1 };
static int reg_changed[REGISTER_COUNT_ALL] = {0 };
static int reg_valid[REGISTER_COUNT_ALL] = { 0 };



/**********************************************************
 * number parser
 **********************************************************/

int getNumberFromRegister(icmProcessorP processor, char *str, int *result)
{    
    Uns32 reg;
    
    if(str != 0 && icmReadReg(processor, str, &reg)) {
        *result = reg;
        return 1;
    }
    return 0;
}

int getNumberFromPointer(icmProcessorP processor, char *str, int *result)
{
    /* get valud from within pointer, eg [r0] [0x1234] */
    int adr, len = strlen(str);
    if(len < 3 || str[0] != '[' || str[len-1] != ']') return 0;
    
    str[len-1] = '\0';
    str++;
    
    if(!getNumber(processor, str, &adr))
        return 0;
    
    if(!icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr), result, 4, 1, ICM_HOSTENDIAN_HOST))
        return 0;
    
    return 1;    
}
int getNumber(icmProcessorP processor, char *str, int *result)
{
    if(!str || !result) return 0;
    
    if(getNumberFromPointer(processor, str, result)) return 1;    
    if(getNumberFromString(str, result)) return 1;
    if(getNumberFromRegister(processor, str, result)) return 1;    
    if(getAddressFromSymbol(str, result)) return 1;
    if(getNumberFromVariable(str, result)) return 1;
    
    return 0;
}


/**********************************************************
 * printing
 **********************************************************/

void print_dissambly(icmProcessorP processor, Uns32 adr, char *marker)
{
    char *symbol = getSymbolFromAddress(adr);
        
    if(symbol)
        printf("%s:\n", symbol);
        
    printf("%s 0x%08x %s %s", 
           isBreakpoint(adr) ? "*b*" : "   ",
           adr, 
           marker,
           icmDisassemble(processor, adr));
    /* instruction symbol ? */
    symbol = symbol_in_instruction(processor, adr);
    if(symbol)
        printf("\t(%s)", symbol);

    printf("\n");
}

void show_labels_near(icmProcessorP processor, int adr, char *message)
{
    char *label_name;
    int label_adr;
    
    if(message) printf("\n%s:\n", message);
    
    if(getClosestSymbolBefore(adr, &label_adr, &label_name)) {
        printf("%s: ( at %08lx)\n\t....\n", label_name, label_adr); 
    }    
    print_dissambly(processor, adr, ">>>");
    
    if(getClosestSymbolAfter(adr, &label_adr, &label_name)) {
        printf("\t...\n%s: ( at %08lx)\n", label_name, label_adr); 
    }         
}


void print_memory_line(int adr, unsigned int *val, unsigned int *flags, int count)
{
    int i, j;
    if(count < 1) return;
    
    /* address */
    printf("%08x: ", adr);
    
    /* values */
    for(i = 0; i < count; i++)  {
        printf(" %c", (flags[i] & 1) ? '>' : ' ');                
        printf((flags[i] & 2) ? "????????": "%08lx", val[i]);
    }
    for(; i < 4; i++) printf("          "); /* space filler if less than 4*/
    
    /* ascii */
    printf("    ");
    for(i = 0; i < count; i++) {
        unsigned int cs = val[i];
        for(j = 0; j < 4; j++) {
            unsigned int c = (cs >> 24) & 0xFF;
            cs <<= 8;
            printf("%c", !(flags[i] & 2) && (c >= 32 && c < 128) ? c : '.');
        }
        printf(" ");        
    }
    printf("\n");    
}

void print_status_register(char *name, unsigned int value)
{
    int tmp;
    char *tmp2;
    
    printf("%s: %08x", name, value);
    
    printf(" flags=%c%c%c%c%c",
           ( value & (1L << 31)) ? 'N' : '-',
           ( value & (1L << 30)) ? 'Z' : '-',
           ( value & (1L << 29)) ? 'C' : '-',
           ( value & (1L << 28)) ? 'V' : '-',
           ( value & (1L <<  5)) ? 'T' : '-'
           );
    
    printf(" IRQ %-8s", (value & 0x80) ? "disabled" : "enabled");
    printf(" FIQ %-8s", (value & 0x40) ? "disabled" : "enabled");
    
    tmp = value & 31;
    switch(tmp) {
    case 16: tmp2 = "user"; break;
    case 17: tmp2 = "FIQ"; break;
    case 18: tmp2 = "IRQ"; break;
    case 19: tmp2 = "supervisor"; break;
    case 23: tmp2 = "abort"; break;
    case 27: tmp2 = "undefined"; break;
    case 31: tmp2 = "system"; break;
    default: tmp2 = "??";
    }
    printf(" mode=%s", tmp2);
    
    printf("\n");                    
}

static Uns32 work_regs1[REGISTER_COUNT_ALL], work_regs2[REGISTER_COUNT_ALL];
static Uns32 *work1 = &work_regs1;
static Uns32 *work2 = &work_regs2;
void work_registers_get_state(icmProcessorP processor)
{
    int i;
    Uns32 *tmp;
    
    tmp = work1;
    work1 = work2;
    work2 = tmp;
    
    for(i = 0; i < REGISTER_COUNT_NORMAL-1; i++) {
        icmReadReg(processor, reg_names[i], & work1[i]);
    }
    
}

void work_registers_print_changed(icmProcessorP processor)
{
    int i;
    for(i = 0; i < REGISTER_COUNT_NORMAL-1; i++) {        
        if(work1[i] != work2[i]) {        
            printf("%8s=%08lx\n", reg_names[i], work1[i]);
        }
    }    
}


void print_stack_frame(icmProcessorP processor, int sp, char *msg)
{    
    int i, j, data, adr;
    char *sym;
        
    printf("%s (SP=0x%08lx):\n", msg, sp);        
    
    for(i = 0, j = 0; i < 400 && j < 20; i++) {
        
        if(!icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & sp + i * 4), 
                                         &data, 4, 1, ICM_HOSTENDIAN_HOST)) 
            continue;
        
        if(getClosestSymbolBefore(data + 4, &adr, &sym)) {
            
            /* ignore possible crap or __xxx__ address */
            if(adr == 0) continue;
            int len = strlen(sym);
            if(len > 4 && (sym[0] | sym[1] | sym[len-2] | sym[len-1]) == '_')
                continue;
            
            j++;                
            /* not to far ? */
            if( adr > data - 1024) {
                printf(" S[%03d] = 0x%08lx  ==> (0x%08lx) %s\n", i, data, adr, sym);
            }
        }                        
    }
    printf("\n");       
}

/***********************************************************
 * Simulator functions
 ***********************************************************/
int reason_to_stop(int c)
{
    switch(c) {
    case ICM_SR_HALT:
    case ICM_SR_EXIT:
    case ICM_SR_FINISH:
        return 1;
    default:
        return 0;
    }
}

int reason_to_break(int c)
{
    switch(c) {
    case ICM_SR_INTERRUPT:
    case ICM_SR_WATCHPOINT:
    case ICM_SR_BP_ICOUNT:
    case ICM_SR_BP_ADDRESS:
        return 1;
    default:
        return reason_to_stop(c);
    }
}

char *symbol_in_instruction(icmProcessorP processor, Uns32 adr)
{
    Uns32 data, tmp, rel_adr, abs_adr;
    if(!icmDebugReadProcessorMemory( processor, adr, &data, 4, 1, ICM_HOSTENDIAN_HOST))
        return 0;
    
    if( (data & 0x0A000000) == 0x0A000000){ 
        /* B or BL or BLX */
        rel_adr = data & 0x00FFFFFF;
        if(rel_adr & 0x00800000) rel_adr |= 0xFF000000; /* sign extends */
        
        abs_adr = adr + rel_adr * 4 + 8;
        return getSymbolFromAddress(abs_adr);
    }
    
    // LDR/STR rn, [PC, #n]
    if( (data & 0x0E0F0000) == 0x040F0000) {
        rel_adr = data & 0xFFF;
        if(!(data & 0x00800000)) rel_adr = -rel_adr; /* up/down */
        abs_adr = adr + rel_adr + 8;       
        
        if(icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & abs_adr), 
                                        &tmp, 4, 1, ICM_HOSTENDIAN_HOST)) {
            return getSymbolFromAddress(tmp);                    
        }
        
    }
    return 0;
}


int run_one_cycle(icmProcessorP processor, int bypass_breakpoints)
{
    Uns32 pc = (Uns32)icmGetPC(processor); 
    int reason;
    
    print_dissambly( processor, pc, "   ");
                     
    reason = icmSimulate(processor, 1);  
    
    if(reason == ICM_SR_WATCHPOINT) {
        processWatchpoints(processor);
    }
    
    // try to run the instruction again, now without the breakpoint
    if(reason == ICM_SR_BP_ADDRESS && bypass_breakpoints) {
        int found_bp = removeBreakpoint(processor, pc);
        reason = icmSimulate(processor, 1);
        if(found_bp) setBreakpoint(processor, pc);            
    }
    
    return reason;
}



/* -- */

void doShowWhere(icmProcessorP processor)
{
    int sp, pc, tmp, mode;
    if(params_has(1)) {
        char *arg1 = params_next(0);
        if(getNumber(processor, arg1, & tmp)) {         
            show_labels_near(processor, tmp, arg1);    
        } else {
            printf("ERROR: bad address '%s'\n", arg1);        
        }
        return;
    }
    
    icmReadReg(processor, "CPSR", & tmp);
    mode = tmp & 31;        
    
    pc = icmGetPC(processor);
    show_labels_near(processor, pc, "Current location");    
    
    if(icmReadReg(processor, "lr", & tmp) & tmp != 0)
       show_labels_near(processor, tmp - 8, "Caller");    
    
    
    /* show frame */
    icmReadReg(processor, "sp", & tmp);
    print_stack_frame(processor, tmp, "Possible current stack frame");
    
    if(mode != 0x10) {
        icmReadReg(processor, "sp_usr", & tmp);
        print_stack_frame(processor, tmp, "Possible user stack frame");        
    }
    
    if(mode != 0x13) {
        icmReadReg(processor, "sp_svc", & tmp);
        print_stack_frame(processor, tmp, "Possible svc stack frame");        
        
    }

}

void do_list_program(icmProcessorP processor, Uns32 pos, int len, Uns32 mark)
{
    int i;    
    Uns32 pc = (Uns32)icmGetPC(processor);    
    
    for(i = 0; i < len; i++) {        
        char *marker = (pos == pc) ?  ">>>" :  ((pos == mark) ? "-->" : " : ");
        
        print_dissambly(processor, pos, marker);
        pos += 4;
    }
}
    

void do_list_here(icmProcessorP processor)
{
    Uns32 pc = (Uns32)icmGetPC(processor);
    do_list_program(processor, pc - 8, 5, pc);
}                     

void doListProgram(icmProcessorP processor)
{
    int i, len;
    Uns32 pos, mark;
    
    
    len = -1;    
    pos = (Uns32)icmGetPC(processor);
    
    if(params_has(1)) {
        if(getNumber(processor, params_next(0), & pos)) {
            if(!getNumberFromString(params_next(0), &len))
                len = -1;
            
        } else {
            printf("ERROR: unknown address/register\n");
            return;
        }        
    } 
    
    mark = pos;
    
    if(params_has_flag('f')) {
        /* try to show the function we are in */
        int adr1, adr2, len2;
        
        if(!getClosestSymbolBefore(pos, &adr1, 0)) adr1 = pos;
        else if( (unsigned int)(pos - adr1) > 256)  adr1 = pos - 256;
        
        if(!getClosestSymbolAfter(pos, &adr2, 0)) adr2 = pos + 12;
        else if( (unsigned int)(adr2 - pos) > 128)  adr2 = pos + 128;        
        
        pos = adr1;
        len2 = (adr2 - adr1) / 4 + 1;
        if(len2 > len) len = len2;
    }
    
    if(len < 12) len = 12;
    pos &= ~3;
    len = (len + 3) & ~3;
    
    do_list_program(processor, pos, len, mark );
}

void doShowMemory(icmProcessorP processor)
{
    int data, adr, i, count, mod;
    int show_symbols;
    unsigned int val[4], flags[4];
    int has_size;
    
    show_symbols = params_has_flag('s');
    if(!getNumber(processor, params_next(0), & adr)) {
        printf("ERROR: need an address\n");
        return;            
    }
    
    
    has_size = params_has(1);
    if(!getNumberFromString(params_next(0), &count))
        count = MEMORY_DUMP_SIZE;
    
    
    /* show it as ascii */
    if(params_has_flag('a')) {        
        for(i = 0; !has_size || i < count; i++) {
            if(!icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr + i), &data, 1, 1, ICM_HOSTENDIAN_HOST))
                break;
            data &= 0xFF;
            if(data == 0) break;
            printf("%c", data);
        }
        printf("\n");
        return;
    }
    
    if(count < 4)
        count = 4;
    
    adr &= ~3;                
    int org_adr = adr;
    adr &= ~(4 * 4 - 1);
    
    /* make it start at x 16 address */
    count += (adr & 15) / 4;
    adr &= ~15;
    
    /* show previous line too */
    adr -= 16;
    count += 16 / 4;
    
    /* show full lines */
    count = (count + 3) & (~3);    
    
    for(mod = 0, i = 0; i < count; i ++) {       
        char *symbol = show_symbols ? getSymbolFromAddress(adr) : 0;
        
        if(symbol) { 
            print_memory_line( adr - 4 * mod, val, flags, mod);
            mod = 0;
            printf("\n%s:\n", symbol);
        }
        
        flags[mod] = (adr == org_adr) ? 1 : 0;
        
        if(!icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr), &data, 4, 1, ICM_HOSTENDIAN_HOST)) {
            flags[mod] |= 2;
        }
        adr += 4;
        
        val[mod] = data;
        if(mod++ >= 3) {
            print_memory_line( adr - 4 * mod, val, flags, mod);
            mod = 0;
        }            
    }        
    printf("\n");        
    
}    

void doWriteMemory(icmProcessorP processor)
{
    char buffer[1024];
    int adr, data, i;
    
    
    if(!getNumber(processor, params_next(0), & adr)) {
        printf("ERROR: need an address\n");
        return;
    } 
    adr &= ~3;
    
    for(;;) {
        fprintf(stdout, "0x%08lx := ", adr);
        fflush(stdout);
        
        if(!readLine(buffer, 1024)) return;
        
        if(strlen(buffer) < 1) return;
        
        if(!getNumberFromString(buffer, & data)) {
            printf("ERROR: bad number: '%s'\n", buffer);
        } else {
            if(!icmDebugWriteProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr), &data, 4, 1, ICM_HOSTENDIAN_HOST)) {
                printf("ERROR: could not write 0x%08lx to address 0x%08lx\n", data, adr);
                return;
            } 
            
            // readback:
            if(!icmDebugReadProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr), &data, 4, 1, ICM_HOSTENDIAN_HOST)) {
                printf("0x%08lx == ?????\n", adr);
            } else {                
                printf("0x%08lx == 0x%08x\n", adr, data);
            }
            adr += 4;
        }
    }
}


void doShowRegisters(icmProcessorP processor)
{
    unsigned int i, tmp;
    int user_mode = 1;
    
    // first, update all registers
    for(i = 0; i < REGISTER_COUNT_ALL; i++) {
        reg_valid[i] = icmReadReg(processor, reg_names[i], & tmp);
        reg_changed[i] = tmp != reg_values[i];
        reg_values[i] = tmp;
        
        if(i == REGISTER_INDEX_CPSR) {
            if((tmp & 31) != 16)
                user_mode = 0;
            else            
                break;          
        }
    }
    
    
    for(i = 0; i < REGISTER_COUNT_NORMAL; i++) {
        printf("%8s=%08lx%c",
               reg_changed[i] ? reg_names[i] : reg_names_lower[i],
               reg_values[i],
               (i & 3) == 3 ? '\n' : ' '
               );
    }    
    
    print_status_register(reg_changed[REGISTER_INDEX_CPSR] ? 
                          reg_names[REGISTER_INDEX_CPSR] : reg_names_lower[REGISTER_INDEX_CPSR], 
                          reg_values[REGISTER_INDEX_CPSR]);    
    
    if(!user_mode) {
        printf("Banked registers:\n");
        
        for(i = REGISTERS_BANKED_START; i < REGISTERS_BANKED_END; i++) {
            printf("%14s=", reg_changed[i] ? reg_names[i] : 
                   reg_names_lower[i]);
            
            if( reg_valid[i])
                printf("%08lx", reg_values[i]);
            else
                printf("%-8s",  "???");
            
            printf("%c", (i % 3) == 2 ? '\n' : ' ');                          
        }        
        printf("\n");
        
        print_status_register(reg_changed[REGISTER_INDEX_SPSR] ? 
                              reg_names[REGISTER_INDEX_SPSR] : reg_names_lower[REGISTER_INDEX_SPSR], 
                              reg_values[REGISTER_INDEX_SPSR]);    
    }
    printf("\n");
    
}

void doSetWatchpoint(icmProcessorP processor)
{
    int start, end;
    int write_access = 1;
    int physical = 0;
    
    if(!params_has(1)) {
        listAllWatchpoints();
        return;
    }
    
    if(!getNumber(processor, params_next(0), &start)) {        
        printf("Error: bad watch address\n" );
        return;
    }
    
    end = start + 3;
    
    if(params_has_flag('p')) physical = 1;
    if(params_has_flag('w')) write_access = 1;
    
    
    if(params_has(1)) {        
        if(!getNumber(processor, params_next(0), &end)) {        
            printf("Error: bad watch end address\n");
            return;
        }    
    }
        
    
    setWatchpoint(processor, start, end, write_access, physical);
}

void doSetBreakPoint(icmProcessorP processor)
{
    int adr;
    
    if(!getNumber(processor, params_next(0), &adr)) {        
        listAllBreakpoints();
        return;
    }
    
    /* XXX: this wont work on thumb */
    adr &= ~3;
    
    if(isBreakpoint(adr)) {
        removeBreakpoint(processor, adr);
        printf("REMOVED breakpoint from address 0x%08lx\n", adr);
    } else {
        setBreakpoint(processor, adr);
        printf("ADDED breakpoint from address 0x%08lx\n", adr);        
    }
    
    do_list_program(processor, adr-8, 5, adr);    
}

int doNextInstruction(icmProcessorP processor, int show_changes)
{
    int done = 0, reason;
    int i, count;
    Uns32 pc_last, pc, pc_target, lr, r0, r1, r2, r3;
    Uns32 pc_min = 0, pc_max = -1;
    int execute_function = 0;
    int execute_until = 0;
    char *from = 0, *to = 0;
    
    
    if(params_has_flag('r')) {
        icmReadReg(processor, "LR", & pc_target);
        execute_function = 1;
        execute_until = 1;
        count = 0x1000000;        
    } else if(params_has_flag('f')) {
        pc = (Uns32)icmGetPC(processor); 
        getClosestSymbolBefore(pc, &pc_min, &from);
        getClosestSymbolAfter(pc, &pc_max, 0);        
        if(pc_min > pc) pc_min = 0;
        if(pc_max < pc) pc_max = -1;        
        pc_max--; /* dont include this one  */
        execute_function = 1;
        count = 0x1000000;
        printf("INFO: Executing within %08lx-%08lx...\n", pc_min, pc_max);
    } else { 
        count = 1;
    }
    
    if(params_has(1)) {
        if(!getNumberFromString(params_next(0), &count)) {
            printf("ERROR: Invalid number");
            return;
        }
    }
    
    if(show_changes) {
        work_registers_get_state(processor);
    }
    
    for(i = 0; !done & i < count; i++) {
        pc_last = (Uns32)icmGetPC(processor);         
        reason = run_one_cycle(processor, i == 0);                
        done = reason_to_break(reason);
        
        if(reason == ICM_SR_WATCHPOINT) count = 0; // break
        
        if(execute_function) {
            pc = (Uns32)icmGetPC(processor); 
            if(execute_until) {
                if(pc == pc_target) count = i;
            } else {
                if(pc < pc_min || pc > pc_max) count = i;
            }
        }
        
        /* print register changes */        
        if(show_changes) {
            work_registers_get_state(processor);
            work_registers_print_changed(processor);
        }
    }
        
    if(done) {
        done = reason_to_stop(reason);
    }

    if(execute_function) {
        getClosestSymbolBefore(pc, 0, &to);            
        if(from != 0 && to != 0) {            
            printf("\n");         
            
            /* if PC = LR, we assume this was a return from function */
            icmReadReg(processor, "LR", & lr);
            if(pc == lr) {
                icmReadReg(processor, "R0", & r0);                
                printf("INFO: %s returned 0x%08lX to %s...\n", from, r0, to);
            } else if(lr == pc_last + 4) {
                icmReadReg(processor, "R0", & r0);                
                icmReadReg(processor, "R1", & r1);
                icmReadReg(processor, "R2", & r2);                                
                icmReadReg(processor, "R3", & r3);                                
                printf("INFO: Called %s(0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, ...) from %s...\n", 
                       to, r0, r1, r2, r3, from);   
            } else {
                printf("INFO: Leaving %s and entering %s...\n", from, to);
            }
            return done;
        }
    }
        
    /* show the listing */    
    if(count > 1 || execute_function) {
        printf("\n\n");        
        do_list_here(processor);
    }    
    
    return done;
}

int doRunUntilModeChange(icmProcessorP processor)
{
    int done = 0, reason, i = 0;
    int tmp, mode;
    
    icmReadReg(processor, "CPSR", & tmp);
    mode = tmp & 31;        
    
    do {
        reason = run_one_cycle(processor, i++ == 0);                
        done = reason_to_break(reason);            
        
        icmReadReg(processor, "CPSR", & tmp);
        tmp &= 31;
    } while(!done && tmp == mode);
    
    if(tmp != mode) printf("Mode switch: %02x -> %02x\n", mode, tmp);
    
    return done;
}
   
int doRun(icmProcessorP processor)
{
    int steps, count, reason, done;    
    Uns32 start_pc = (Uns32)icmGetPC(processor);
  
    if(! getNumberFromString(params_next(0), &steps))
        steps = MAX_CYCLES;
    
    icmSetICountBreakpoint(processor, (Uns64) steps);        
    
    for(count = 0, done = 0; !done; count++) {
        Uns32 end_pc;
        
        icmSimulatePlatform();
        reason = icmGetStopReason(processor);
        end_pc = (Uns32)icmGetPC(processor);
                
        if(reason == ICM_SR_BP_ADDRESS) {
            if(end_pc == start_pc && count == 0) {
                // first instruction was on a breakpoint :(
                reason = run_one_cycle(processor,1);                
            }
        }
        
        done = reason_to_break(reason);
    }
    
    icmClearICountBreakpoint(processor);    
    
    do_list_here(processor);
    
    if(done) {
        if(reason == ICM_SR_BP_ICOUNT) {
            printf("NOTE: stopped simulation after %d cycles\n", steps);
        } else if(reason == ICM_SR_BP_ADDRESS) {            
            Uns32 pc = (Uns32)icmGetPC(processor); 
            char *sym = getSymbolFromAddress(pc);
            
            if(sym) {
                int r0, r1, r2, r3, lr;
                icmReadReg(processor, "R0", & r0);                
                icmReadReg(processor, "R1", & r1);
                icmReadReg(processor, "R2", & r2);                                
                icmReadReg(processor, "R3", & r3);                                
                printf("Stopped at breakpoint %s(0x%08lX, 0x%08lX, 0x%08lX, 0x%08lX, ...)", 
                       sym, r0, r1, r2, r3);   
                
                icmReadReg(processor, "LR", & lr);                
                sym = getSymbolFromAddress(lr);
                if(sym) {
                    printf(" (from %sn", sym);
                }
                printf("\n");
                
            } else {
                printf("Reached a breakpoint...\n");            
            }
        }
    }    
    
    return done;
}

static int need_start_binary = True;
int doLoadBinary(icmProcessorP processor, char *filename )
{
    if(filename) {
        char *type = strrchr(filename , '.');
        char *at;
        int adr = 0;
        
        
        if(type == 0) {
            fprintf(stderr, "Unknown file type: '%s'\n", filename);
            return 0;
        }
        
        /* see if a load address is given */
        at = strchr(type, ':');        
        if(at) {
            *at++ = '\0';
            if(*at != '\0') {         
                if(!getNumberFromString( at, & adr)) {
                    adr = 0;
                    printf("ERROR: invalid load address: %s\n", at);
                } 
            }
        }
        
        /* load based on extension */
        if(!strcasecmp(type, ".fw") || !strcasecmp(type, ".bin")) { 
            int c;
            FILE *fp = fopen(filename, "rb");
            if(!fp) {
                fprintf(stderr, "Could not open '%s' for reading\n", filename);
                return 0;
            }
            
            printf("INFO: loading %s to 0x%08lx\n", filename, adr);
            
            if(need_start_binary) {
                need_start_binary = False;                
                icmWriteReg(processor, "R15", (Addr) (0xFFFFFFFF & adr));   
                printf("INFO: focring cpu to start at 0x%08lx\n", adr);
            }
            
            
            while(!feof(fp)) {
                c = 0;
                c |= (0xFF & fgetc(fp)) << 0;
                c |= (0xFF & fgetc(fp)) << 8;
                c |= (0xFF & fgetc(fp)) << 16;
                c |= (0xFF & fgetc(fp)) << 24;                
                if(!icmDebugWriteProcessorMemory( processor, (Addr) (0xFFFFFFFF & adr), &c, 4, 1, ICM_HOSTENDIAN_HOST)) {    
                    fprintf(stderr, "Could to write '%s' to address %08lx\n", filename, adr);
                    return 0;
                }
                adr += 4;
            }
            fclose(fp);
            
            return 1;
        } else if(!strcasecmp(type, ".elf")) {
            //processor, ELF file, virtual address, enable debug, start execution spec in object file
            int ret = icmLoadProcessorMemory(processor, filename, False, True, need_start_binary);
            if(!ret) return 0;
            need_start_binary = False;                
            doLoadSymbols(filename);        
            return 1;            
        } else {
            fprintf(stderr, "Unsupported file type: '%s'\n", filename);
            return 0;
        }
    } else {
        printf("No file given!\n");
    }
    
    return 0; /* failed */
}

void doDataOperation(icmProcessorP proc)
{
    char *op, *name;
    unsigned long adr, size;
    int has_adr;
    
    if(!params_has(2)) {
        fprintf(stderr, "Data operation requires op and name\n");
        return;
    }
    
    op = params_next(0);
    name = params_next(0);
    
    if(!strcmp(op, "delete")) {
        data_delete(name);
    } else if(!strcmp(op, "show")) {
        data_show(name);        
    } else if(!strcmp(op, "save")) {
        if(params_has(1)) {            
            if(getNumber(proc, params_next(0), & adr))  {
                size = 64;
                if(params_has(1)) {
                    getNumber(proc, params_next(0), & size);
                 } 

                data_save(proc, name, adr, size);
                return;
            }                        
        }
        // FAILED:
        fprintf(stderr, "Usage: d save <filename> <start> <size>\n");
    } else if(!strcmp(op, "load")) {
        has_adr = 0;
        if(params_has(1)) {
            if(getNumber(proc, params_next(0), & adr))  {
                has_adr = 1;
            }
        }
        
        data_load(proc, name, adr, has_adr);

    } else if(!strcmp(op, "compare")) {
        has_adr = 0;
        if(params_has(1)) {
            if(getNumber(proc, params_next(0), & adr))  {
                has_adr = 1;
            }
        }
        
        data_compare(proc, name, adr, has_adr);
    } else {
        fprintf(stderr, "Unknown data operation: '%s'\n", op);
    }
}

void doShowHelp()
{
    printf("Valid commands are\n"
           "  n [c | -f | -r]          Next c instructions, out of function, until return\n"
           "  c                        Next instruction until an ARM mode change is detected\n"
           "  g [c]                    Go\n"           
           "  b adr                    Set execution breakpoint\n"
           "  B adr [adr] [-r | -p]    Set memory breakpoint\n"
           "  r                        Show registers\n"           
           "  W [adr]                  Where am I in the code?\n"
           "  m adr [len] [ -s | -a]   Dump memory contecnts. Example: 'm 0xFF00'\n"
           "  d <op> [op params...]    Data operation\n"
           "  w adr                    Write data to memory starting at address adr.\n"
           "  s [-f] [adr] [len]       Show code, -f tries to show the whole function.\n"
           "  L filename               Load file to memory\n"           
           "  ! [number]               History\n"           
           "  < [filename]             Save history\n"           
           "  > [filename]             Load and replay history\n"           
           "  = [var [value]]          List/get/set variables\n"
           "  q                        Quit\n"
           );
    
}


void doTerminate()
{    
    printf ("\n**** End time: %Lf  ****\n\n", icmGetCurrentTime ());
    icmTerminate();
    exit(0);
}

/***********************************************************
 * signal handlers
 ***********************************************************/

void int_handler(int sig)
{
    printf("Ctrl-C detected, existing...\n");
    fflush(stdout);
    
    doTerminate();
}


/***********************************************************
 * execute one command
 ***********************************************************/

int executeCommand(icmProcessorP processor, char *buffer)
{
    
    int i, count;
    char *tmp, *cmd, *arg1, *start;
    Uns32 currentPC;
    
    
    /* multiple commands on the same line? */    
    tmp = strchr(buffer, ';');
    if(tmp) {
        *tmp++ = '\0';
        if(executeCommand(processor, buffer)) return True;
        return executeCommand(processor, tmp);
    }
    
    
    start = buffer;    
    cmd = trimString(start);
    if(cmd == 0) return False;
    
      
    
    /* history buffer is handled separatly before any parsing */
    switch(cmd[0]) {
    case '!':
        arg1 = trimString(cmd+1);        
        executeHistory(processor, arg1);
        return 0;
    case '<':
        arg1 = trimString(cmd+1);        
        historySave(arg1);
        return 0;        
    case '>':
        arg1 = trimString(cmd+1);                
        historyReplay(processor, arg1);
        return 0;
    default:
        addHistory(cmd);
    }
    
    
    /* parse the line and execute the command */
    params_set_line(cmd);
    cmd = params_next("");
    
    switch(cmd[0]) {
    case 'n':
    case 'N':        
        return doNextInstruction(processor, 1);
        
    case 'c':
        return doRunUntilModeChange(processor);
        
    case 'b':
        doSetBreakPoint(processor);
        break;
        
    case 'B':
        doSetWatchpoint(processor);
        break;
    case 'd':
        doDataOperation(processor);
        break;
        
    case 'l':
        doLoadSymbols( params_next("a.out") );
        break;
    case 's':
        doListProgram(processor);            
        break;
        
    case 'm':
        doShowMemory(processor);
        break;
        
    case 'W':
        doShowWhere(processor);
        break;
        
    case 'w':
        doWriteMemory(processor);
        break;
        
    case 'r':
        doShowRegisters(processor);
        break;     
                
    case 'g':
        doRun(processor);
        break;
        
    case 'L':
        doLoadBinary(processor, params_next(0) );
        break;
                
    case 'q':
        return True;
        
    case '=':
        doDefineVariable(processor);
        break;
    default:
        doShowHelp();
    }
    
    return False;
}

#if 0

/* stop excution if we see too many exceptions */
void notify_exception()
{
#define EXCEPTION_STOP_COUNT 5
    static int cnt = 0;
    
    if(cnt ++ > EXCEPTION_STOP_COUNT) {
        cnt = 0;
        printf("DEBUGGER: automatic stop after %d exceptions...\n", EXCEPTION_STOP_COUNT);
        icmInterrupt();  
//        icmYield(processor);
    }
}
#endif

/***********************************************************
 * main
 ***********************************************************/

int main(int argc, const char **argv) 
{    
    int data, adr, i, debug = 0, verbose = 0;
    
    char *binaries[32];
    int binaries_count = 0;
    int normalrun = 0;
    
#define LINE_SIZE (1024 * 2)    
    char buffer[LINE_SIZE];
    char buffer_last[LINE_SIZE];
    
    /* parse args */
    for(i = 1; i < argc; i++) {
        const char *arg = argv[i];
        
        if(*arg == '-') {
            if(!strcmp(arg, "-debug") || !strcmp(arg, "--debug") || !strcmp(arg, "-d"))
                debug = 1;
            else if(!strcmp(arg, "-verbose") || !strcmp(arg, "--verbose") || !strcmp(arg, "-V"))
                verbose = 1;
            else if(!strcmp(arg, "-normalrun") || !strcmp(arg, "-n") )
            	normalrun = 1;
            else {
                fprintf(stderr, "Unknown option: %s\n", arg);
                return 3;
            }
        } else {
            if(binaries_count < 31)
                binaries[binaries_count++] = arg;
        }
    }
    
    if(binaries_count == 0) {
        fprintf(stderr, "Usage: %s [-d] [-V] <ELF file> [<additional elf files>]\n", argv[0]);
        return 3;        
    }
    
    icmBusP bus;
    icmProcessorP processor;
    
    /* load the platform with the first binary */
    createPlatform(debug, verbose, &processor, &bus);
    
    /* load the remaining binaries */
    for(i = 0; i < binaries_count; i++) {
        if(!doLoadBinary(processor, binaries[i])) {
            fprintf(stderr, "ERROR: unable to load the binary '%s'\n", binaries[i]);
            return 20;
        }
    }
    
    icmSimulationStarting();
    
    
    if (False == icmSetSimulationTimePrecision (0.00001)) {
        printf ("Couldn't set precision\n");
        return 0;
    }
    
    // dealing with warnings
    //    icmIgnoreMessage("ARM_MORPH_UCA");
    
    
    if(debug || normalrun) {
        icmSimulatePlatform();    
        
    } else {
        Bool done = False;
        
        printf("\n"
               "***********************************************\n"
               " Welcome to the SICS ARM debugger, where we\n"
               " keep things _really_ simple...\n"
               "\n"
               " Binary = %s, Debug = %d, Verbose = %d\n"
               "***********************************************\n"
               "\n",
               binaries[0], debug, verbose
               );
        
        /*
         * set this as late as possible so OVP can't override
         */
        signal(SIGINT, int_handler);
        
#if 0        
        /* exception callback */
        icmSetExceptionWatchPoint(processor, processor, notify_exception);
#endif
        
        /* load startup file */
        historyReplay(processor, "debugger.startup");
        
        
        /*
         * the command loop
         */
        
        strcpy(buffer_last, "");        
        while(!done) { 
                        
            printf("Enter debugger command> ");                
            fflush(stdout);
            

            if(!readLine(buffer, LINE_SIZE)) {
                if(strlen(buffer_last) > 0) {
                    /* repeat last command */
                    strcpy(buffer, buffer_last);
                } else {
                    continue;
                }
            } else {
                strcpy(buffer_last, buffer);
            }
                        
            done = executeCommand(processor, buffer);                
            
                                       
        }
    }
    
    doTerminate();
    return 0;
}

