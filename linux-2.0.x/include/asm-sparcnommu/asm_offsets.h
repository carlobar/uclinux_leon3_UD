#ifndef __ASM_OFFSETS_H__
#define __ASM_OFFSETS_H__
/*
 * DO NOT MODIFY.
 *
 * This file was generated by arch/sparcnommu/Makefile
 *
 */


#define TASK_STATE 0 /* offsetof(struct task_struct, state) */
#define TASK_PRIORITY 8 /* offsetof(struct task_struct, priority) */
#define TASK_SIGNAL 12 /* offsetof(struct task_struct, signal) */
#define TASK_BLOCKED 16 /* offsetof(struct task_struct, blocked) */
#define TASK_FLAGS 20 /* offsetof(struct task_struct, flags) */
#define TASK_SAVED_KSTACK 84 /* offsetof(struct task_struct, saved_kernel_stack) */
#define TASK_KSTACK_PG 88 /* offsetof(struct task_struct, kernel_stack_page) */

#define THREAD_UMASK 528 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,uwinmask) */
#define THREAD_SADDR 536 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,sig_address) */
#define THREAD_SDESC 540 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,sig_desc) */
#define THREAD_KSP 544 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,ksp) */
#define THREAD_KPC 548 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,kpc) */
#define THREAD_KPSR 552 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,kpsr) */
#define THREAD_KWIM 556 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,kwim) */
#define THREAD_FORK_KPSR 560 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,fork_kpsr) */
#define THREAD_FORK_KWIM 564 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,fork_kwim) */
#define THREAD_REG_WINDOW 568 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,reg_window) */
#define THREAD_STACK_PTRS 1080 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,rwbuf_stkptrs) */
#define THREAD_W_SAVED 1112 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,w_saved) */
#define THREAD_FLOAT_REGS 1120 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,float_regs) */
#define THREAD_FSR 1376 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,fsr) */
#define THREAD_SIGSTK 1512 /* offsetof(struct task_struct, tss) + offsetof(struct thread_struct,sstk_info) */

#endif