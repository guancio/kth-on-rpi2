#ifndef _TYPES_H_
#define _TYPES_H_

#include <uclib.h>
#include <memlib.h>


/* Page table definition */
/* Page bits is defined in core/hw/cpu/arm/arm_common/mmu.h. */
#define ADDR_TO_PAGE(adr) ((adr) >> PAGE_BITS)
#define PAGE_TO_ADDR(page) ((page) << PAGE_BITS)


/* cpu type */
typedef enum cpu_type_ {
    CT_ARM
} cpu_type;

typedef enum cpu_model_ {
    CM_ARMv5 = 0,
    CM_ARMv6,
    CM_ARMv7a

} cpu_model;

/* memory layout */
typedef struct {
    uint32_t page_start;
    uint32_t page_count;
    uint32_t type;
    uint32_t flags;
} memory_layout_entry;

typedef enum memory_layout_type_ {
    MLT_NONE = 0,
    MLT_HYPER_RAM,
    MLT_HYPER_ROM,
    MLT_USER_RAM,
    MLT_USER_ROM,
    MLT_TRUSTED_RAM,
    MLT_IO_RW_REG,
    MLT_IO_RO_REG,
    MLT_IO_HYP_REG,
} memory_layout_type;

typedef enum memory_layout_flags_ {
    MLF_LAST = (1 << 0),
    MLF_READABLE = (1 << 2),
    MLF_WRITEABLE = (1 << 3)
} memory_layout_flags;




#endif /* _TYPES_H_ */
