/*
 * linux/drivers/char/68332keyboard.c
 * quite ugly but no time at the moment...
 *
 * Copyright Gerold Boehler <gboehler@mail.austria.at>
 */

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <../drivers/char/kbd_kern.h>
#include <asm/leon.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/system.h>

#define CLOCK	(PORTF & 0x40)
#define DATA	(PORTF & 0x20)
#define BUFLEN	32
#define KBD_DEFMODE ((1 << VC_REPEAT) | (1 << VC_META))

extern struct tty_struct **ttytab;
extern unsigned char kbd_read_mask;	/* modified by psaux.c */
extern void kbd_bh(void);

static void kbd_int(int, void*, struct pt_regs*);
int kbd_init(void);

static void kbd_int(int irq, void *dummy, struct pt_regs *regs) {
	unsigned char status,scancode;

	//disable_keyboard();

	while((status=leon_apbps2->status) & LEON_REG_PS2_STATUS_DR){

		/* mouse data? */
		if (status & kbd_read_mask & 0x20)
			break;

		scancode = leon_apbps2->data;

		handle_scancode(scancode);
	}

	mark_bh(KEYBOARD_BH);
	//enable_keyboard();



/*         cli();     */
/* 	save_flags(flags); */
/* 	if(leon_apbps2->status & 0x1){ */
/* 	 	  scancode = leon_apbps2->data; */

/* 		  //	scancode = read_code(); */

/* 	//	keyboard_stop(); */

/* 	if (scancode == 0xf0 || scancode == 0xe0) { */
/* 		scancode = read_code(); */



/* 	/\* only capslock style at the moment *\/ */


/* 	} else if ( scancode == 0x12 || scancode == 0x59) { */
/* 		upper = upper ? 0 : 1; */
/* 	} */
/* 	if(get_ascii(scancode) &&  !is_full()) { */
/* 		buf[in] = upper ? get_ASCII(scancode) : get_ascii(scancode); */
/* 		in = (in+1) % BUFLEN; */
/* 		wake_up(&keypress_wait); */
/* 	} */

/* 	//	keyboard_start(); */
/* 	restore_flags(flags); */
}


//volatile LEON3_APBPS2_REGS_Map *leon_apbps2 = 0;

/*
 * External int 29 is used for startbit, portf=0x40 is used for clock
        */
        
int kbd_init(void) {
        
  int j;
  amba_apb_device dev[1];
  printk("Attaching grlib PS/2 keyboard drivers:\n");
  j = amba_get_free_apbslv_devices (VENDOR_GAISLER, GAISLER_KBD, dev, 1); 
  if (j >= 1) {
    leon_apbps2 = (LEON3_APBPS2_REGS_Map *)dev[0].start;
	}
  else{
    return -1;
	}

    printk("Init");

    //  while(!(leon_apbps2->ctrl & 0x08)){
    //      printk("\nwaiting for interrupt\n");
    //    }

  if (request_irq(dev[0].irq, kbd_int, IRQ_FLG_STD, "keyboard", NULL)) { 
    panic("Unable to attach keyboard interrupt!\n"); 
    return -1; 
  } 

  //leon_apbps2->ctrl = LEON_REG_PS2_CTRL_RE ;
    leon_apbps2->ctrl = LEON_REG_PS2_CTRL_RE | LEON_REG_PS2_CTRL_RI | LEON_REG_PS2_CTRL_TE;
    leon_apbps2->data = 0xFF;

  {

    int i;
    struct kbd_struct kbd0;
    extern struct tty_driver console_driver;

    kbd0.ledflagstate = kbd0.default_ledflagstate = 0;//KBD_DEFLEDS;
    kbd0.ledmode = LED_SHOW_FLAGS;
    kbd0.lockstate = 0;//KBD_DEFLOCK;
    kbd0.slockstate = 0;
    kbd0.modeflags = KBD_DEFMODE;
    kbd0.kbdmode = VC_XLATE;
    
    for (i = 0 ; i < MAX_NR_CONSOLES ; i++)
      kbd_table[i] = kbd0;
    
    ttytab = console_driver.table;
    
    init_bh(KEYBOARD_BH, kbd_bh);
    mark_bh(KEYBOARD_BH);
  }
  {
    unsigned long scancode, status;
    do {
      scancode = leon_apbps2->data;
      status = leon_apbps2->status;
    } while (status & 0x01);
  }
	return 0;
}


