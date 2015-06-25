#ifndef _HW_HAL_H_
#define _HW_HAL_H_

/*
 * HAL API: these are the functions that must be implemented by HAL
 */

/* CPU */

void cpu_init();

void cpu_break_to_debugger();
void cpu_get_type(cpu_type *type, cpu_model *model);

interrupt_state cpu_interrupt_set(interrupt_state new_);
interrupt_state cpu_interrupt_user_set(interrupt_state new_);

int cpu_irq_get_count();
void cpu_irq_set_enable(int number, BOOL enable);
void cpu_irq_set_handler(int number, cpu_callback handler);

void cpu_set_undef_handler(cpu_callback handler);
void cpu_set_swi_handler(cpu_callback handler);
void cpu_set_abort_handler(cpu_callback inst, cpu_callback data);

/* context */
extern void cpu_context_current_set(context *curr);
extern context *cpu_context_current_get();
extern void cpu_context_initial_set(context *curr);
extern int cpu_context_depth_get();

/* caches */
extern void mem_cache_set_enable(BOOL enable);
extern void mem_cache_invalidate(BOOL inst_inv, BOOL data_inv, BOOL dont_writeback);
extern void mem_cache_dcache_area(uint32_t reg0, uint32_t reg1, uint32_t flag);


/* memory management */
extern void mem_mmu_set_enable(BOOL enable);
extern void mem_mmu_set_domain(int domain);

extern void mem_mmu_set_translation_table(uint32_t *base);
extern void mem_mmu_tlb_invalidate_all(BOOL inst, BOOL data);
extern void mem_mmu_tlb_invalidate_one(BOOL inst, BOOL data, uint32_t virtual_addr);

/* fast context switching */
extern void mem_mmu_pid_set(int pid);
extern int mem_mmu_pid_get();

/* stdio */
extern void stdio_write_char(int c);
extern int stdio_read_char();
extern int stdio_can_write();
extern int stdio_can_read();

/* timer */
extern void timer_tick_start(cpu_callback handler);
extern void timer_tick_stop();

/* board stuff */
memory_layout_entry *mem_padr_lookup(uint32_t padr);


/* HW REG */
extern uint32_t hwreg_read(uint32_t adr);
extern void hwreg_write(uint32_t adr, uint32_t value);
extern uint32_t hwreg_update(uint32_t adr, uint32_t mask, uint32_t value);
extern void hwreg_wait(uint32_t adr, uint32_t mask, uint32_t value);

#endif /*_HW_HAL_H_*/
