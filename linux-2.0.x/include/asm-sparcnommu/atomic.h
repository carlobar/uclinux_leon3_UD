/* atomic.h: These really suck for now.
 *
 * Copyright (C) 1996 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __ARCH_SPARC_ATOMIC__
#define __ARCH_SPARC_ATOMIC__

typedef int atomic_t;

#ifdef __KERNEL__
#include <asm/system.h>
#include <asm/psr.h>

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
#define __atomic_fool_gcc(x) ((struct { int a[100]; } *)x)

static __inline__ void atomic_add(atomic_t i, atomic_t *v)
{
	__asm__ __volatile__(
	"rd	%%psr, %%g2\n\t"
	"andcc	%%g2, %2, %%g0\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, %2, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1: ld	[%0], %%g3\n\t"
	"add	%%g3, %1, %%g3\n\t"
	"andcc	%%g2, %2, %%g0\n\t"
	"st	%%g3, [%0]\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, 0x0, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1:     nop\n\t"
	: /* no outputs */
	: "r" (__atomic_fool_gcc(v)), "r" (i), "i" (PSR_PIL)
	: "g2", "g3", "cc");
}

static __inline__ void atomic_sub(atomic_t i, atomic_t *v)
{
	__asm__ __volatile__(
	"rd	%%psr, %%g2\n\t"
	"andcc	%%g2, %2, %%g0\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, %2, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1:ld	[%0], %%g3\n\t"
	"sub	%%g3, %1, %%g3\n\t"
	"andcc	%%g2, %2, %%g0\n\t"
	"st	%%g3, [%0]\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, 0x0, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
	"1: nop \n\t"
	: /* no outputs */
	: "r" (__atomic_fool_gcc(v)), "r" (i), "i" (PSR_PIL)
	: "g2", "g3", "cc");
}

static __inline__ int atomic_add_return(atomic_t i, atomic_t *v)
{
	__asm__ __volatile__(
	"rd	%%psr, %%g2\n\t"
	"andcc	%%g2, %3, %%g0\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, %3, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1:ld	[%1], %%g3\n\t"
	"add	%%g3, %2, %0\n\t"
	"andcc	%%g2, %3, %%g0\n\t"
	"st	%0, [%1]\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, 0x0, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1:  nop\n\t"
	: "=&r" (i)
	: "r" (__atomic_fool_gcc(v)), "0" (i), "i" (PSR_PIL)
	: "g2", "g3", "cc");

	return i;
}

static __inline__ int atomic_sub_return(atomic_t i, atomic_t *v)
{
	__asm__ __volatile__(
	"rd	%%psr, %%g2\n\t"
	"andcc	%%g2, %3, %%g0\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, %3, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1:ld	[%1], %%g3\n\t"
	"sub	%%g3, %2, %0\n\t"
	"andcc	%%g2, %3, %%g0\n\t"
	"st	%0, [%1]\n\t"
	"bne	1f\n\t"
	" nop\n\t"
	"wr	%%g2, 0x0, %%psr\n\t"
	"nop\n\t"
	"nop\n\t"
	"nop\n\t"
"1: nop\n\t"
	: "=&r" (i)
	: "r" (__atomic_fool_gcc(v)), "0" (i), "i" (PSR_PIL)
	: "g2", "g3", "cc");

	return i;
}

#define atomic_dec_return(v) atomic_sub_return(1,(v))
#define atomic_inc_return(v) atomic_add_return(1,(v))

#define atomic_sub_and_test(i, v) (atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)

#define atomic_inc(v) atomic_add(1,(v))
#define atomic_dec(v) atomic_sub(1,(v))

#endif /* !(__KERNEL__) */

#endif /* !(__ARCH_SPARC_ATOMIC__) */
