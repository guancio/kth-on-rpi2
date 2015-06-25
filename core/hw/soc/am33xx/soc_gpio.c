
#include <hw.h>

#include "soc_defs.h"
#include "soc_ctrl.h"

#define GPIO_BANK_COUNT 4


typedef struct {
     uint32_t revision;
     uint32_t unused0[3];
     uint32_t sysconfig;
     uint32_t unused1[3];
     uint32_t eoi;
     uint32_t irqstatus_raw_0;
     uint32_t irqstatus_raw_1;
     uint32_t irqstatus_0;
     uint32_t irqstatus_1;
     uint32_t irqstatus_set_0;
     uint32_t irqstatus_set_1;
     uint32_t irqstatus_clr_0;
     uint32_t irqstatus_clr_1;
     uint32_t unused2[52];
     uint32_t sysstatus;
     uint32_t unused3[6];
     uint32_t ctrl;
     uint32_t oe;
     uint32_t datain;
     uint32_t dataout;
     uint32_t leveldetect0;
     uint32_t leveldetect1;
     uint32_t risingdetect;
     uint32_t fallingdetect;
     uint32_t debouncenable;
     uint32_t debouncingtime;
     uint32_t unused4[14];
     uint32_t cleardataout;
    uint32_t setdataout;    
} volatile gpio_registers;


/* ------------------------------------------------------------------ */

static const uint32_t gpio_bank_adr[GPIO_BANK_COUNT] = {
    0x44E07000,
    0x4804C000,
    0x481AC000,
    0x481AE000
};

/* ------------------------------------------------------------------ */

int gpio_get_bank_from_pin(int pin)
{
    return pin / 32;
}
int gpio_get_position_from_pin(int pin)
{
    return pin % 32;
}

gpio_registers *gpio_get_bank(int bank)
{
    if(bank < 0 || bank >= GPIO_BANK_COUNT) return 0;    
    return (gpio_registers *) gpio_bank_adr[bank];
}

void gpio_set_data(int pin, BOOL data)
{
    int bank;
    gpio_registers *r;
    
    bank = gpio_get_bank_from_pin(pin);
    pin  = gpio_get_position_from_pin(pin);
    
    r = gpio_get_bank(bank);    
    if(!r) return;
    
    if(data)
        r->dataout |= 1 << pin;
    else
        r->dataout &= ~(1 << pin);        
}

void gpio_set_direction(int pin, BOOL is_input)
{    
    int bank, pos;
    gpio_registers *r;
    
    /* set GPIO OE */
    bank = gpio_get_bank_from_pin(pin);
    pin  = gpio_get_position_from_pin(pin);
    r = gpio_get_bank(bank);    
    if(!r) return;
    
    if(is_input)
        r->oe |=  (1 << pos);    
    else        
        r->oe &= ~(1 << pos);
}


BOOL gpio_get_data(int pin)
{
    int bank;
    gpio_registers *r;
    
    bank = gpio_get_bank_from_pin(pin);
    pin  = gpio_get_position_from_pin(pin);
    r = gpio_get_bank(bank);    
    if(!r) return FALSE;
    
    return r->datain & (1 << pin) ? TRUE : FALSE;
}

void gpio_set_pad_configuration(int item, int mode, BOOL is_input,
                   BOOL pull_enable, BOOL pull_up)
{
    /* Pad configuration is inside the control module */
    am355_ctrl *ctrl = (am355_ctrl *) CONTROL_MODULE_BASE;
    
    /* pad configuratiob starts here */
    BASE_REG *pad_config = (BASE_REG *) & ctrl->conf_gpmc_ad0;
    uint32_t val;
    
    /* get register value */
    val = mode & 0x7;    
    if(is_input) val |= CONTROL_MODULE_PADCONFG_INPUT_ENABLE;
    if(pull_enable) val |= CONTROL_MODULE_PADCONFG_PULL_ENABLE;   
    if(pull_up) val |= CONTROL_MODULE_PADCONFG_PULL_UP;
    
    /* set it! */
    pad_config[item] = val;    
}

void soc_gpio_init()
{
    int i;
    for(i = 0; i < GPIO_BANK_COUNT; i++) {
        gpio_registers *r = gpio_get_bank(i);
        
        /* disable all interrupts for now */
        r->irqstatus_clr_0 = r->irqstatus_clr_0 = -1;
    }
    
}

