/*
 * Automatically generated C config: don't edit
 */
#define AUTOCONF_INCLUDED
#define CONFIG_UCLINUX 1
/*
 * Code maturity level options
 */
#define CONFIG_EXPERIMENTAL 1
/*
 * Loadable module support
 */
#undef  CONFIG_MODULES
/*
 * Platform dependant setup
 */
#undef  CONFIG_LEON_2
#define CONFIG_LEON_3 1
/*
 * Platform
 */
#define CONFIG_LEON4TSIM 1
#undef  CONFIG_RAMKERNEL
#define CONFIG_ROMKERNEL 1
/*
 * General setup
 */
#undef  CONFIG_PCI
#undef  CONFIG_USE_ARCHMEMCPY
#undef  CONFIG_NET
#undef  CONFIG_SYSVIPC
#define CONFIG_REDUCED_MEMORY 1
#define CONFIG_BINFMT_FLAT 1
#undef  CONFIG_BINFMT_ZFLAT
#undef  CONFIG_CONSOLE
#define CONFIG_SPARCNOMMU_SET_DEF_KERNCMD 1
#define CONFIG_SPARCNOMMU_DEF_KERNCMD "console=ttyS0,38400"
/*
 * Floppy, IDE, and other block devices
 */
#define CONFIG_BLK_DEV_BLKMEM 1
#undef  CONFIG_BLK_DEV_IDE
/*
 * Additional Block/FLASH Devices
 */
#undef  CONFIG_BLK_DEV_LOOP
#undef  CONFIG_BLK_DEV_MD
#define CONFIG_BLK_DEV_RAM 1
#undef  CONFIG_RD_RELEASE_BLOCKS
#undef  CONFIG_DEV_FLASH
#undef  CONFIG_BLK_DEV_NFA
/*
 * Leon3 Amba configuration
 */
/*
 * Vendor Gaisler
 */
#define CONFIG_GRLIB_GAISLER_APBUART 1
#define CONFIG_GRLIB_GAISLER_APBUART_FIXED 1
#undef  CONFIG_GRLIB_GAISLER_GRETH
/*
 * PS/2 drivers
 */
#undef  CONFIG_GRLIB_GAISLER_PS2
/*
 * Vendor Opencores
 */
#undef  CONFIG_GRLIB_OPENCORES_ETHERNET
/*
 * Controller Area Network Cards/Chips
 */
#undef  CONFIG_CAN4LINUX
/*
 * Filesystems
 */
#undef  CONFIG_QUOTA
#undef  CONFIG_MINIX_FS
#undef  CONFIG_EXT_FS
#define CONFIG_EXT2_FS 1
#undef  CONFIG_XIA_FS
#undef  CONFIG_NLS
#define CONFIG_PROC_FS 1
#undef  CONFIG_NCP_FS
#undef  CONFIG_HPFS_FS
#undef  CONFIG_SYSV_FS
#undef  CONFIG_AUTOFS_FS
#undef  CONFIG_AFFS_FS
#define CONFIG_ROMFS_FS 1
#undef  CONFIG_JFFS_FS
#undef  CONFIG_UFS_FS
/*
 * Character devices
 */
#undef  CONFIG_LEON_SERIAL
/*
 * Kernel hacking
 */
#undef  CONFIG_FULLDEBUG
#undef  CONFIG_ALLOC2
#undef  CONFIG_PROFILE
