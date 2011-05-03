! Code intended to configure and initialize the GPIO device

	.section	"start_gpio"
gpio_init:
	set	0x00000003, %o0		! Value of port direction register 
	set	0x00000001, %o1		! Value of port output register
	set	0x80000b00, %o2		! GPIO address
	st	%o0, [%o2 + 0x08]
	st	%o1, [%o2 + 0x04]
