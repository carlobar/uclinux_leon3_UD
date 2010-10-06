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
#define CONFIG_LEON_2 1
#undef  CONFIG_LEON_3

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
#define CONFIG_NET 1
#undef  CONFIG_SYSVIPC
#define CONFIG_REDUCED_MEMORY 1
#define CONFIG_BINFMT_FLAT 1
#undef  CONFIG_BINFMT_ZFLAT
#undef  CONFIG_CONSOLE

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
#undef  CONFIG_BLK_DEV_RAM
#undef  CONFIG_DEV_FLASH
#undef  CONFIG_BLK_DEV_NFA

/*
 * Controller Area Network Cards/Chips
 */
#undef  CONFIG_CAN4LINUX

/*
 * Networking options
 */
#undef  CONFIG_FIREWALL
#undef  CONFIG_NET_ALIAS
#define CONFIG_INET 1
#undef  CONFIG_IP_FORWARD
#undef  CONFIG_IP_MULTICAST
#undef  CONFIG_SYN_COOKIES
#undef  CONFIG_IP_ACCT
#undef  CONFIG_IP_ROUTER
#undef  CONFIG_NET_IPIP

/*
 * (it is safe to leave these untouched)
 */
#undef  CONFIG_INET_PCTCP
#undef  CONFIG_INET_RARP
#undef  CONFIG_NO_PATH_MTU_DISCOVERY
#undef  CONFIG_IP_NOSR
#undef  CONFIG_SKB_LARGE

/*
 *  
 */
#undef  CONFIG_IPX
#undef  CONFIG_ATALK
#undef  CONFIG_AX25
#undef  CONFIG_BRIDGE
#undef  CONFIG_NETLINK
#undef  CONFIG_IPSEC

/*
 * Network device support
 */
#define CONFIG_NETDEVICES 1
#undef  CONFIG_DUMMY
#undef  CONFIG_SLIP
#undef  CONFIG_PPP
#undef  CONFIG_EQUALIZER
#undef  CONFIG_UCCS8900
#undef  CONFIG_SMC9194
#undef  CONFIG_SMC91111
#undef  CONFIG_NE2000
#undef  CONFIG_FEC
#undef  CONFIG_OPEN_ETH

/*
 * Filesystems
 */
#undef  CONFIG_QUOTA
#undef  CONFIG_MINIX_FS
#undef  CONFIG_EXT_FS
#undef  CONFIG_EXT2_FS
#undef  CONFIG_XIA_FS
#undef  CONFIG_NLS
#define CONFIG_PROC_FS 1
#define CONFIG_NFS_FS 1
#undef  CONFIG_ROOT_NFS
#undef  CONFIG_SMB_FS
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
#define CONFIG_LEON_SERIAL 1

/*
 * Kernel hacking
 */
#undef  CONFIG_FULLDEBUG
#undef  CONFIG_ALLOC2
#undef  CONFIG_PROFILE
