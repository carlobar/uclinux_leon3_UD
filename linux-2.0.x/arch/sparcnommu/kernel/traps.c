/* $Id: traps.c,v 1.43 1996/04/24 09:09:42 davem Exp $
 * arch/sparc/kernel/traps.c
 *
 * Copyright 1995 David S. Miller (davem@caip.rutgers.edu)
 */

/*
 * I hate traps on the sparc, grrr...
 */

#include <linux/sched.h>  /* for jiffies */
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/signal.h>

#include <asm/delay.h>
#include <asm/system.h>
#include <asm/ptrace.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/unistd.h>
#include <asm/traps.h>

#include "hex.h"
#include <asm/leon.h>

/* #define TRAP_DEBUG */

static void puts(unsigned char *s)
{
	struct lregs *regs = (struct lregs *) 0x80000000;

	while(*s) {
		while (!(regs->uartstatus1 & 0x4));
		regs->uartdata1 = *s++;
	}
	while (!(regs->uartstatus1 & 0x4));
	regs->uartdata1 = '\r';
	while (!(regs->uartstatus1 & 0x4));
	regs->uartdata1 = '\n';

}

void dumpit(unsigned long l1, unsigned long l2)
{
	struct lregs *regs = (struct lregs *) 0x80000000;
	outhex32(l1);
	puts("l1");
	outhex32(l2);
	puts("l2");
	while(1);
}

struct trap_trace_entry {
	unsigned long pc;
	unsigned long type;
};

int trap_curbuf = 0;
struct trap_trace_entry trapbuf[1024];

void syscall_trace_entry(struct pt_regs *regs)
{
	printk("%s[%d]: ", current->comm, current->pid);
	printk("scall<%d> (could be %d)\n", (int) regs->u_regs[UREG_G1],
	       (int) regs->u_regs[UREG_I0]);
}

void syscall_trace_exit(struct pt_regs *regs)
{
}

void die_if_kernel(char *str, struct pt_regs *pregs)
{
	struct lregs *regs = (struct lregs *) 0x80000000;
	unsigned long pc;

	pc = pregs->pc;
	puts("");
	outhex32(pc);
	puts(" traped to die_if_kernel");
	show_regs(pregs);
	if(pregs->psr & PSR_PS)
		do_exit(SIGKILL);
	do_exit(SIGSEGV);
}

void do_hw_interrupt(unsigned long type, unsigned long psr, unsigned long pc)
{
	if(type < 0x80) {
		/* Sun OS's puke from bad traps, Linux survives! */
		printk("Unimplemented Sparc TRAP, type = %02lx\n", type);
		die_if_kernel("Whee... Hello Mr. Penguin", current->tss.kregs);
	}	
	if(type == SP_TRAP_SBPT) {
		send_sig(SIGTRAP, current, 1);
		return;
	}
	current->tss.sig_desc = SUBSIG_BADTRAP(type - 0x80);
	current->tss.sig_address = pc;
	send_sig(SIGILL, current, 1);
}

void do_illegal_instruction(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			    unsigned long psr)
{
	extern int do_user_muldiv (struct pt_regs *, unsigned long);
	
	if(psr & PSR_PS)
		die_if_kernel("Kernel illegal instruction", regs);
#ifdef TRAP_DEBUG
	printk("Ill instr. at pc=%08lx instruction is %08lx\n",
	       regs->pc, *(unsigned long *)regs->pc);
#endif
	//if (!do_user_muldiv (regs, pc))
	//  return;
	
	current->tss.sig_address = pc;
	current->tss.sig_desc = SUBSIG_ILLINST;
	send_sig(SIGILL, current, 1);
}


void do_priv_instruction(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			 unsigned long psr)
{
  extern int do_emul_y(struct pt_regs *regs, unsigned long pc);
  unsigned int insn;

  if(psr & PSR_PS)
    die_if_kernel("Penguin instruction from Penguin mode??!?!", regs);
  
  //if (!do_emul_y (regs, pc))
  // return;
	
  current->tss.sig_address = pc;
  current->tss.sig_desc = SUBSIG_PRIVINST;
  send_sig(SIGILL, current, 1);
}

/* XXX User may want to be allowed to do this. XXX */

void do_memaccess_unaligned(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			    unsigned long psr)
{
	if(regs->psr & PSR_PS) {
		printk("KERNEL MNA at pc %08lx npc %08lx called by %08lx\n", pc, npc,
		       regs->u_regs[UREG_RETPC]);
		die_if_kernel("BOGUS", regs);
		/* die_if_kernel("Kernel MNA access", regs); */
	}
	current->tss.sig_address = pc;
	current->tss.sig_desc = SUBSIG_PRIVINST;
	send_sig(SIGBUS, current, 1);
}

extern void fpsave(unsigned long *fpregs, unsigned long *fsr,
		   void *fpqueue, unsigned long *fpqdepth);
extern void fpload(unsigned long *fpregs, unsigned long *fsr);

static unsigned long init_fsr = 0x0UL;
static unsigned long init_fregs[32] __attribute__ ((aligned (8))) =
                { ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL,
		  ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL,
		  ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL,
		  ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL, ~0UL };

void do_fpd_trap(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		 unsigned long psr)
{
	/* Sanity check... */
	if(psr & PSR_PS)
		die_if_kernel("Kernel gets FloatingPenguinUnit disabled trap", regs);

	put_psr(get_psr() | PSR_EF);    /* Allow FPU ops. */
	regs->psr |= PSR_EF;
#ifndef __SMP__
	if(last_task_used_math == current)
		return;
	if(last_task_used_math) {
		/* Other processes fpu state, save away */
		struct task_struct *fptask = last_task_used_math;
		fpsave(&fptask->tss.float_regs[0], &fptask->tss.fsr,
		       &fptask->tss.fpqueue[0], &fptask->tss.fpqdepth);
	}
	last_task_used_math = current;
	if(current->used_math) {
		fpload(&current->tss.float_regs[0], &current->tss.fsr);
	} else {
		/* Set initial sane state. */
		fpload(&init_fregs[0], &init_fsr);
		current->used_math = 1;
	}
#else
	if(!current->used_math) {
		fpload(&init_fregs[0], &init_fsr);
		current->used_math = 1;
	} else {
		fpload(&current->tss.float_regs[0], &current->tss.fsr);
	}
	current->flags |= PF_USEDFPU;
#endif
}

static unsigned long fake_regs[32] __attribute__ ((aligned (8)));
static unsigned long fake_fsr;
static unsigned long fake_queue[32] __attribute__ ((aligned (8)));
static unsigned long fake_depth;

void do_fpe_trap(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		 unsigned long psr)
{
	static int calls = 0;
#ifndef __SMP__
	struct task_struct *fpt = last_task_used_math;
#else
	struct task_struct *fpt = current;
#endif

	put_psr(get_psr() | PSR_EF);
	/* If nobody owns the fpu right now, just clear the
	 * error into our fake static buffer and hope it don't
	 * happen again.  Thank you crashme...
	 */
#ifndef __SMP__
	if(!fpt) {
#else
        if(!(fpt->flags & PF_USEDFPU)) {
#endif
		fpsave(&fake_regs[0], &fake_fsr, &fake_queue[0], &fake_depth);
		regs->psr &= ~PSR_EF;
		return;
	}
	fpsave(&fpt->tss.float_regs[0], &fpt->tss.fsr,
	       &fpt->tss.fpqueue[0], &fpt->tss.fpqdepth);
	fpt->tss.sig_address = pc;
	fpt->tss.sig_desc = SUBSIG_FPERROR; /* as good as any */
#ifdef __SMP__
	fpt->flags &= ~PF_USEDFPU;
#endif
	if(psr & PSR_PS) {
		/* The first fsr store/load we tried trapped,
		 * the second one will not (we hope).
		 */
		printk("WARNING: FPU exception from kernel mode. at pc=%08lx\n",
		       regs->pc);
		regs->pc = regs->npc;
		regs->npc += 4;
		calls++;
		if(calls > 2)
			die_if_kernel("Too many Penguin-FPU traps from kernel mode",
				      regs);
		return;
	}
	send_sig(SIGFPE, fpt, 1);
#ifndef __SMP__
	last_task_used_math = NULL;
#endif
	regs->psr &= ~PSR_EF;
	if(calls > 0)
		calls=0;
}

void handle_tag_overflow(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			 unsigned long psr)
{
	if(psr & PSR_PS)
		die_if_kernel("Penguin overflow trap from kernel mode", regs);
	current->tss.sig_address = pc;
	current->tss.sig_desc = SUBSIG_TAG; /* as good as any */
	send_sig(SIGEMT, current, 1);
}

void handle_watchpoint(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		       unsigned long psr)
{
#ifdef TRAP_DEBUG
	printk("Watchpoint detected at PC %08lx NPC %08lx PSR %08lx\n",
	       pc, npc, psr);
#endif
	if(psr & PSR_PS)
		panic("Tell me what a watchpoint trap is, and I'll then deal "
		      "with such a beast...");
}

void handle_reg_access(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		       unsigned long psr)
{
#ifdef TRAP_DEBUG
	printk("Register Access Exception at PC %08lx NPC %08lx PSR %08lx\n",
	       pc, npc, psr);
#endif
	send_sig(SIGILL, current, 1);
}

void handle_cp_disabled(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			unsigned long psr)
{
	send_sig(SIGILL, current, 1);
}

void handle_bad_flush(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		      unsigned long psr)
{
#ifdef TRAP_DEBUG
	printk("Unimplemented FLUSH Exception at PC %08lx NPC %08lx PSR %08lx\n",
	       pc, npc, psr);
#endif
	printk("INSTRUCTION=%08lx\n", *((unsigned long *) regs->pc));
	send_sig(SIGILL, current, 1);
}

void handle_cp_exception(struct pt_regs *regs, unsigned long pc, unsigned long npc,
			 unsigned long psr)
{
#ifdef TRAP_DEBUG
	printk("Co-Processor Exception at PC %08lx NPC %08lx PSR %08lx\n",
	       pc, npc, psr);
#endif
	send_sig(SIGILL, current, 1);
}

void handle_hw_divzero(struct pt_regs *regs, unsigned long pc, unsigned long npc,
		       unsigned long psr)
{
	send_sig(SIGILL, current, 1);
}

/* Since we have our mappings set up, on multiprocessors we can spin them
 * up here so that timer interrupts work during initialization.
 */

extern void sparc_cpu_startup(void);

extern int linux_num_cpus;
extern ctxd_t *srmmu_ctx_table_phys;

int linux_smp_still_initting;
unsigned int thiscpus_tbr;
int thiscpus_mid;

void trap_init(void)
{
}
