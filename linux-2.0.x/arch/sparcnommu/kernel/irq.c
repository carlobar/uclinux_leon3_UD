/*
 * linux/arch/$(ARCH)/platform/$(PLATFORM)/traps.c -- general exception handling code
 *
 * Cloned from Linux/m68k.
 *
 * No original Copyright holder listed,
 * Probabily original (C) Roman Zippel (assigned DJD, 1999)
 *
 * Copyright 1999-2000 D. Jeff Dionne, <jeff@rt-control.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/errno.h>

#include <asm/system.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/page.h>
#include <asm/leon.h>

/* table for system interrupt handlers */
static irq_handler_t irq_list[SYS_IRQS];

static const char *default_names[SYS_IRQS] = {
	"huh", "EDAC", "UART 1", "UART 2",
	"pio0 handler", "pio1 handler", "pio2 handler", "pio3 handler",
	"Timer 1", "Timer 2",
};

/* The number of spurious interrupts */
volatile unsigned int num_spurious;

#define NUM_IRQ_NODES 16
static irq_node_t nodes[NUM_IRQ_NODES];

/*
 * void init_IRQ(void)
 *
 * Parameters:	None
 *
 * Returns:	Nothing
 *
 * This function should be called during kernel startup to initialize
 * the IRQ handling routines.
 */

#ifdef CONFIG_LEON_3
extern volatile LEON3_IrqCtrl_Regs_Map *LEON3_IrqCtrl_Regs; /* drivers/amba/amba.c:amba_init() */
extern volatile LEON3_GpTimer_Regs_Map *LEON3_GpTimer_Regs;
#endif

void init_IRQ(void)
{
#ifndef CONFIG_LEON_3
        struct lregs *regs = (struct lregs *) 0x80000000;
#endif        
	int i;

#ifdef CONFIG_LEON_3
        if (LEON3_GpTimer_Regs && LEON3_IrqCtrl_Regs) {
          //regs -> timerctrl1 = 0;
          //regs -> timercnt1 = 0;
          //regs -> timerload1 = (((1000000/100) - 1));
          LEON3_GpTimer_Regs ->e[0].val = 0;
          LEON3_GpTimer_Regs ->e[0].rld = (((1000000/100) - 1));
          LEON3_GpTimer_Regs ->e[0].ctrl = 0;
        } else {
          while(1) printk("No Timer/irqctrl found\n");
        }
#else
	regs -> timerctrl1 = 0;
	regs -> timercnt1 = 0;
	regs -> timerload1 = (((1000000/100) - 1));
#endif        
	
        //regs->scalercnt  = 33;
	//regs->scalerload = 33;
	//regs->timercnt1  = 1999;
	//regs->timerload1 = 1999; /* HZ = 100 */

	for (i = 0; i < SYS_IRQS; i++) {
		irq_list[i].handler = NULL;
		irq_list[i].flags   = IRQ_FLG_STD;
		irq_list[i].dev_id  = NULL;
		irq_list[i].devname = default_names[i];
	}

	for (i = 0; i < NUM_IRQ_NODES; i++)
		nodes[i].handler = NULL;
        
	/* all ints off, prioritized */
#ifdef CONFIG_LEON_3
	LEON3_IrqCtrl_Regs->mask[0] = 0;
        LEON3_IrqCtrl_Regs->ilevel = 0xfffe;

        LEON3_GpTimer_Regs ->e[0].ctrl =
          LEON3_GPTIMER_EN |
          LEON3_GPTIMER_RL |
          LEON3_GPTIMER_LD |
          LEON3_GPTIMER_IRQEN;
#else
	regs->irqmask = (unsigned long)0xfffe0000;

	regs -> timerctrl1 = 0x7;
#endif        

        //regs -> timerctrl1 = 0x7;
        
}

irq_node_t *new_irq_node(void)
{
	irq_node_t *node;
	short i;

	for (node = nodes, i = NUM_IRQ_NODES-1; i >= 0; node++, i--)
		if (!node->handler)
			return node;

	printk ("new_irq_node: out of nodes\n");
	return NULL;
}

int request_irq(unsigned int irq, void (*handler)(int, void *, struct pt_regs *),
                unsigned long flags, const char *devname, void *dev_id)
{
#ifndef CONFIG_LEON_3
        struct lregs *regs = (struct lregs *) 0x80000000;
#endif        

	if (irq < IRQ1 || irq > IRQMAX) {
		printk("%s: Incorrect IRQ %d from %s\n", __FUNCTION__, irq, devname);
		return -ENXIO;
	}

	if (!(irq_list[irq].flags & IRQ_FLG_STD)) {
		if (irq_list[irq].flags & IRQ_FLG_LOCK) {
			printk("%s: IRQ %d from %s is not replaceable\n",
			       __FUNCTION__, irq, irq_list[irq].devname);
			return -EBUSY;
		}
		if (flags & IRQ_FLG_REPLACE) {
			printk("%s: %s can't replace IRQ %d from %s\n",
			       __FUNCTION__, devname, irq, irq_list[irq].devname);
			return -EBUSY;
		}
	}
	irq_list[irq].handler = handler;
	irq_list[irq].flags   = flags;
	irq_list[irq].dev_id  = dev_id;
	irq_list[irq].devname = devname;

	/* turn it on in the hardware */
#ifdef CONFIG_LEON_3
        LEON3_IrqCtrl_Regs ->mask[0] |= 1<<irq;
#else
	regs->irqmask |= 1<<irq;
#endif        
	return 0;
}

void free_irq(unsigned int irq, void *dev_id)
{
#ifndef CONFIG_LEON_3
        struct lregs *regs = (struct lregs *) 0x80000000;
#endif
        
	if (irq < IRQ1 || irq > IRQMAX) {
		printk("%s: Incorrect IRQ %d\n", __FUNCTION__, irq);
		return;
	}

	if (irq_list[irq].dev_id != dev_id)
		printk("%s: Removing probably wrong IRQ %d from %s\n",
		       __FUNCTION__, irq, irq_list[irq].devname);

#ifdef CONFIG_LEON_3
	LEON3_IrqCtrl_Regs ->mask[0] &= ~(1<<irq);
#else
	regs->irqmask &= ~(1<<irq);
#endif
        
	irq_list[irq].handler = NULL;
	irq_list[irq].flags   = IRQ_FLG_STD;
	irq_list[irq].dev_id  = NULL;
	irq_list[irq].devname = default_names[irq];
}

/* usually not useful in embedded systems */
unsigned long probe_irq_on (void)
{
	return 0;
}

int probe_irq_off (unsigned long irqs)
{
	return 0;
}

void enable_irq(unsigned int irq)
{
}

void disable_irq(unsigned int irq)
{
}

asmlinkage void process_int(unsigned long vec, struct pt_regs *fp)
{
#ifndef CONFIG_LEON_3
        struct lregs *regs = (struct lregs *) 0x80000000;
#endif
        
	/* give the machine specific code a crack at it first */

	kstat.interrupts[vec]++;
	if (irq_list[vec].handler) {
		irq_list[vec].handler(vec, irq_list[vec].dev_id, fp);
		/* clear the interrupt */
#ifdef CONFIG_LEON_3
                LEON3_IrqCtrl_Regs ->iclear  = 1<<vec;
#else
		regs->irqclear = 1<<vec;
#endif                
	} else
		panic("No interrupt handler for level %ld\n", vec);
}

int get_irq_list(char *buf)
{
	int i, len = 0;

	/* autovector interrupts */
	for (i = 0; i < SYS_IRQS; i++) {
		if (irq_list[i].handler) {
			len += sprintf(buf+len, "auto %2d: %10u ", i,
			               i ? kstat.interrupts[i] : num_spurious);
			if (irq_list[i].flags & IRQ_FLG_LOCK)
				len += sprintf(buf+len, "L ");
			else
				len += sprintf(buf+len, "  ");
			len += sprintf(buf+len, "%s\n", irq_list[i].devname);
		}
	}
	return len;
}
