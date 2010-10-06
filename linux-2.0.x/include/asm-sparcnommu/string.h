/*
 * string.h: External definitions for optimized assembly string
 *           routines for the Linux Kernel.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __SPARC_STRING_H__
#define __SPARC_STRING_H__

#ifndef AUTOCONF_INCLUDED
#error "Need to include autoconf.h"
#endif

#ifdef CONFIG_USE_ARCHMEMCPY
#define __HAVE_ARCH_BCOPY
#define __HAVE_ARCH_MEMMOVE
#define __HAVE_ARCH_MEMCPY
#define __HAVE_ARCH_MEMSET
#define __HAVE_ARCH_STRLEN
#endif

#endif /* !(__SPARC_STRING_H__) */
