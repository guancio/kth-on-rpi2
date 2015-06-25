#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

///////////////////////////////////////////
//LED-related address definitions here
#define GPIO_BASE	0x3F200000UL
#define LED_GPFSEL      GPIO_GPFSEL4
#define LED_GPFBIT      21
#define LED_GPSET       GPIO_GPSET1
#define LED_GPCLR       GPIO_GPCLR1
#define LED_GPIO_BIT    15

#define GPIO_GPFSEL0    0
#define GPIO_GPFSEL1    1
#define GPIO_GPFSEL2    2
#define GPIO_GPFSEL3    3
#define GPIO_GPFSEL4    4
#define GPIO_GPFSEL5    5

#define GPIO_GPSET0     7
#define GPIO_GPSET1     8

#define GPIO_GPCLR0     10
#define GPIO_GPCLR1     11

#define GPIO_GPLEV0     13
#define GPIO_GPLEV1     14

#define GPIO_GPEDS0     16
#define GPIO_GPEDS1     17

#define GPIO_GPREN0     19
#define GPIO_GPREN1     20

#define GPIO_GPFEN0     22
#define GPIO_GPFEN1     23

#define GPIO_GPHEN0     25
#define GPIO_GPHEN1     26

#define GPIO_GPLEN0     28
#define GPIO_GPLEN1     29

#define GPIO_GPAREN0    31
#define GPIO_GPAREN1    32

#define GPIO_GPAFEN0    34
#define GPIO_GPAFEN1    35

#define GPIO_GPPUD      37
#define GPIO_GPPUDCLK0  38
#define GPIO_GPPUDCLK1  39

//GPIO Register set
volatile unsigned int* gpio = (unsigned int*)GPIO_BASE;

//Simple loop variable
volatile unsigned int tim;

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags){
	/* Write 1 to the GPIO16 init nibble in the Function Select 1 GPIO
	peripheral register to enable GPIO16 as an output */
	gpio[LED_GPFSEL] |= (1 << LED_GPFBIT);

	while(1){
		//Wait...
		for(tim = 0; tim < 500000; tim++){
		    ;//Do nothing...
		}
		/* Set the LED GPIO pin low ( Turn OK LED on for original Pi, and off
		   for plus models )*/
		gpio[LED_GPCLR] = (1 << LED_GPIO_BIT);

		//Wait...
		for(tim = 0; tim < 500000; tim++){
		    ;//Do nothing...
		}
		/* Set the LED GPIO pin high ( Turn OK LED off for original Pi, and on
		   for plus models )*/
		gpio[LED_GPSET] = (1 << LED_GPIO_BIT);
	}
}
