
#include <hw.h>
#include "soc_defs.h"


#define GPIO_BANK_COUNT 6

typedef struct {
    uint32_t revision;
    uint32_t unused[3];
    uint32_t sysconfig;
    uint32_t sysstatus;
    uint32_t irqstatus1;
    uint32_t irqenable1;
    uint32_t wakeupenable;
    uint32_t unused2;
    uint32_t irqstatus2;
    uint32_t irqenable2;
    uint32_t ctrl;
    uint32_t oe;
    uint32_t datain;
    uint32_t dataout;
} volatile gpio_registers;


/* ------------------------------------------------------- */


static const uint32_t gpio_bank_adr[GPIO_BANK_COUNT] = {
    0x48310000,
    0x49050000,
    0x49052000,
    0x49054000,
    0x49056000,
    0x49058000
};

/* ------------------------------------------------------- */

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
    return (gpio_registers *) IO_VA_ADDRESS(gpio_bank_adr[bank]);
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

// --------------------------------------------------------


void soc_gpio_init()
{
    int i;
    for(i = 0; i < GPIO_BANK_COUNT; i++) {
        gpio_registers *r = gpio_get_bank(i);
        
        /* disable all interrupts for now */
        r->irqenable1 = r->irqenable2 = 0;
    }
    
}

// --------------------------------------------------------

void gpio_set_pad_configuration(int item, int mode, BOOL is_input,
                   BOOL pull_enable, BOOL pull_up)
{
    
    BASE_REG *pad_config;
    uint32_t tmp, val;
    
    
    /* set system controller stuff */
    pad_config = (BASE_REG *) IO_VA_ADDRESS((SMC_CONTROL_PAD_CONFIG_BASE + (item / 2) * 4));
// Flyswatter gets stuck in uart_write_char when using the printf below!
// printf("DEBUF: pad_config = %x, base =%x, item = %d\n", pad_config, SMC_CONTROL_PAD_CONFIG_BASE, item); // DEBUG
    
    /*
    printf("DEBUG: item %d, mode %d. pad_config %x -> %x\n",
           item, mode, pad_config, *pad_config); // DEBUG
     */
    
    /* get pad configuration value */
    val = mode;    
    if(is_input) val |= SMC_CONTROL_PAD_CONFIG_INPUTENABLE;
    if(pull_enable) val |= SMC_CONTROL_PAD_CONFIG_PULLENABLE;
    if(pull_up) val |= SMC_CONTROL_PAD_CONFIG_PULLUP;
    
    /* set pad configuration to correct 16 bits */        
    tmp = (uint32_t)*pad_config;
   
    if( item & 1) 
        *pad_config = (tmp & 0x0000FFFF) | (val << 16);
    else
        *pad_config = (tmp & 0xFFFF0000) | val;    
    
    /*
    printf("DEBUG: AFTER: val %x ==> %x\n", val, *pad_config);// DEBUG
     */    
}

