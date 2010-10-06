/* Can_debug
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*------------------------------------------------------------------
* $Header: $
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* $Log: $
*
*/
#include <can_defs.h>


/* default debugging level */

#if DEBUG
# ifndef DEFAULT_DEBUG
  unsigned int   dbgMask  = \
    (DBG_ENTRY | DBG_EXIT | DBG_BRANCH | DBG_DATA | DBG_INTR | DBG_1PPL)
    & ~DBG_ALL;
# else
unsigned int   dbgMask  = 0;
# endif
#else
unsigned int   dbgMask  = 0;
#endif


