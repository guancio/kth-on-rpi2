
MEMORY
{
	ram : ORIGIN = 0x1000000, LENGTH = 0x1000
}

SECTIONS
{
	. = 0x1000000;
	.startup : { *(.text.startup*) } > ram
	.bss : { *(.bss*) } > ram
}

