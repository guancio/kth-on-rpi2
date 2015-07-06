
#ifndef _SOC_TIMER_H_
#define _SOC_TIMER_H_

#define TIMER_BASE 0x3F003000
#define TIMER_CHANNEL_COUNT 4

//TODO: Correct entries
struct timer_channel {
    uint32_t ccr;
    uint32_t cmr;
    uint32_t unused0[2];
    uint32_t cv;
    uint32_t ra;
    uint32_t rb;
    uint32_t rc;
    uint32_t sr;
    uint32_t ier;
    uint32_t idr;
    uint32_t imr;
    uint32_t unused1[4];
};


//TODO: Correct entries
typedef struct {
    struct timer_channel channels[TIMER_CHANNEL_COUNT];
    uint32_t bcr;
    uint32_t bmr;
} volatile timer_registers;

extern void soc_timer_init();

#endif /* _SOC_TIMER_H_ */
