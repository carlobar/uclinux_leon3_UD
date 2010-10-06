/*
 *  linux/fs/proc/root.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  proc root directory handling functions
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/config.h>
#include <asm/bitops.h>

/*
 * Offset of the first process in the /proc root directory..
 */
#define FIRST_PROCESS_ENTRY 256

static int proc_root_readdir(struct inode *, struct file *, void *, filldir_t);
static int proc_root_lookup(struct inode *,const char *,int,struct inode **);

static unsigned char proc_alloc_map[PROC_NDYNAMIC / 8] = {0};

/*
 * These are the generic /proc directory operations. They
 * use the in-memory "struct proc_dir_entry" tree to parse
 * the /proc directory.
 *
 * NOTE! The /proc/scsi directory currently does not correctly
 * build up the proc_dir_entry tree, and will show up empty.
 */
static struct file_operations proc_dir_operations = {
	NULL,			/* lseek - default */
	NULL,			/* read - bad */
	NULL,			/* write - bad */
	proc_readdir,		/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL,			/* no special release code */
	NULL			/* can't fsync */
};

/*
 * proc directories can do almost nothing..
 */
struct inode_operations proc_dir_inode_operations = {
	&proc_dir_operations,	/* default net directory file-ops */
	NULL,			/* create */
	proc_lookup,		/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* readpage */
	NULL,			/* writepage */
	NULL,			/* bmap */
	NULL,			/* truncate */
	NULL			/* permission */
};

/*
 * The root /proc directory is special, as it has the
 * <pid> directories. Thus we don't use the generic
 * directory handling functions for that..
 */
static struct file_operations proc_root_operations = {
	NULL,			/* lseek - default */
	NULL,			/* read - bad */
	NULL,			/* write - bad */
	proc_root_readdir,	/* readdir */
	NULL,			/* select - default */
	NULL,			/* ioctl - default */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL,			/* no special release code */
	NULL			/* no fsync */
};

/*
 * proc root can do almost nothing..
 */
static struct inode_operations proc_root_inode_operations = {
	&proc_root_operations,	/* default base directory file-ops */
	NULL,			/* create */
	proc_root_lookup,	/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	NULL,			/* readlink */
	NULL,			/* follow_link */
	NULL,			/* readpage */
	NULL,			/* writepage */
	NULL,			/* bmap */
	NULL,			/* truncate */
	NULL			/* permission */
};

/*
 * This is the root "inode" in the /proc tree..
 */
struct proc_dir_entry proc_root = {
	PROC_ROOT_INO, 5, "/proc",
	S_IFDIR | S_IRUGO | S_IXUGO, 2, 0, 0,
	0, &proc_root_inode_operations,
	NULL, NULL,
	NULL,
	&proc_root, NULL
};

struct proc_dir_entry proc_net = {
	PROC_NET, 3, "net",
	S_IFDIR | S_IRUGO | S_IXUGO, 2, 0, 0,
	0, &proc_dir_inode_operations,
	NULL, NULL,
	NULL,
	NULL, NULL	
};

struct proc_dir_entry proc_scsi = {
	PROC_SCSI, 4, "scsi",
	S_IFDIR | S_IRUGO | S_IXUGO, 2, 0, 0,
	0, &proc_dir_inode_operations,
	NULL, NULL,
	NULL, &proc_root, NULL
};

struct proc_dir_entry proc_sys_root = {
	PROC_SYS, 3, "sys",			/* inode, name */
	S_IFDIR | S_IRUGO | S_IXUGO, 2, 0, 0,	/* mode, nlink, uid, gid */
	0, &proc_dir_inode_operations,		/* size, ops */
	NULL, NULL,				/* get_info, fill_inode */
	NULL,					/* next */
	NULL, NULL				/* parent, subdir */
};

int proc_register(struct proc_dir_entry * dir, struct proc_dir_entry * dp)
{

	dp->next = dir->subdir;
	dp->parent = dir;
	dir->subdir = dp;
	if (S_ISDIR(dp->mode))
		dir->nlink++;
	return 0;
}

int proc_unregister(struct proc_dir_entry * dir, int ino)
{
	struct proc_dir_entry **p = &dir->subdir, *dp;

	while ((dp = *p) != NULL) {
		if (dp->low_ino == ino) {
			*p = dp->next;
			dp->next = NULL;
			if (S_ISDIR(dp->mode))
				dir->nlink--;
			if (ino >= PROC_DYNAMIC_FIRST &&
			    ino < PROC_DYNAMIC_FIRST+PROC_NDYNAMIC)
				clear_bit(ino-PROC_DYNAMIC_FIRST, 
					  (void *) proc_alloc_map);
			return 0;
		}
		p = &dp->next;
	}
	return -EINVAL;
}	

static int make_inode_number(void)
{
	int i = find_first_zero_bit((void *) proc_alloc_map, PROC_NDYNAMIC);
	if (i<0 || i>=PROC_NDYNAMIC) 
		return -1;
	set_bit(i, (void *) proc_alloc_map);
	return PROC_DYNAMIC_FIRST + i;
}

int proc_register_dynamic(struct proc_dir_entry * dir,
			  struct proc_dir_entry * dp)
{

	int i = make_inode_number();
	if (i < 0)
		return -EAGAIN;
	dp->low_ino = i;
	dp->next = dir->subdir;
	dp->parent = dir;
	dir->subdir = dp;
	if (S_ISDIR(dp->mode))
		dir->nlink++;
	return 0;
}

/*
 * /proc/self:
 */
static int proc_self_followlink(struct inode * dir, struct inode * inode,
			int flag, int mode, struct inode ** res_inode)
{

	iput(dir);
	*res_inode = proc_get_inode(inode->i_sb, (current->pid << 16) + PROC_PID_INO, &proc_pid);
	iput(inode);
	if (!*res_inode)
		return -ENOENT;
	return 0;
}

static int proc_self_readlink(struct inode * inode, char * buffer, int buflen)
{
	int len;
	char tmp[30];


	iput(inode);
	len = 1 + sprintf(tmp, "%d", current->pid);
	if (buflen < len)
		len = buflen;
	memcpy_tofs(buffer, tmp, len);
	return len;
}

static struct inode_operations proc_self_inode_operations = {
	NULL,			/* no file-ops */
	NULL,			/* create */
	NULL,			/* lookup */
	NULL,			/* link */
	NULL,			/* unlink */
	NULL,			/* symlink */
	NULL,			/* mkdir */
	NULL,			/* rmdir */
	NULL,			/* mknod */
	NULL,			/* rename */
	proc_self_readlink,	/* readlink */
	proc_self_followlink,	/* follow_link */
	NULL,			/* readpage */
	NULL,			/* writepage */
	NULL,			/* bmap */
	NULL,			/* truncate */
	NULL			/* permission */
};

static struct proc_dir_entry base_stuff[] = {
{ PROC_LOADAVG, 7, "loadavg",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_UPTIME, 6, "uptime",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_MEMINFO, 7, "meminfo",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_SERIAL, 6, "serial", S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_KMSG, 4, "kmsg", S_IFREG | S_IRUSR, 1, 0, 0,},
{ PROC_VERSION, 7, "version", S_IFREG | S_IRUGO, 1, 0, 0,},
#ifdef CONFIG_PCI
{ PROC_PCI, 3, "pci",S_IFREG | S_IRUGO, 1, 0, 0,},
#endif
{ PROC_CPUINFO, 7, "cpuinfo",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_SELF, 4, "self", S_IFLNK | S_IRUGO | S_IWUGO | S_IXUGO, 1, 0, 0,64, &proc_self_inode_operations,},
#ifdef CONFIG_DEBUG_MALLOC
{ PROC_MALLOC, 6, "malloc", S_IFREG | S_IRUGO, 1, 0, 0,},
#endif
{ PROC_KCORE, 5, "kcore",S_IFREG | S_IRUSR, 1, 0, 0,},
#ifdef CONFIG_MODULES
{ PROC_MODULES, 7, "modules",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_KSYMS, 5, "ksyms",S_IFREG | S_IRUGO, 1, 0, 0,},
#endif
{ PROC_STAT, 4, "stat", S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_DEVICES, 7, "devices", S_IFREG | S_IRUGO, 1, 0, 0, },
{ PROC_INTERRUPTS, 10,"interrupts",S_IFREG | S_IRUGO, 1, 0, 0,},
#ifdef __SMP_PROF__
{ PROC_SMP_PROF, 3,"smp", S_IFREG | S_IRUGO, 1, 0, 0,},
#endif 
{ PROC_FILESYSTEMS, 11,"filesystems",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_DMA, 3, "dma", S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_IOPORTS, 7, "ioports",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_CMDLINE, 7, "cmdline",S_IFREG | S_IRUGO, 1, 0, 0,},
#ifdef CONFIG_RTC
{ PROC_RTC, 3, "rtc",S_IFREG | S_IRUGO, 1, 0, 0,},
#endif
{ PROC_LOCKS, 5, "locks",S_IFREG | S_IRUGO, 1, 0, 0,},
{ PROC_MTAB, 6, "mounts", S_IFREG | S_IRUGO, 1, 0, 0, },
{ PROC_PROFILE, 7, "profile",S_IFREG | S_IRUGO | S_IWUSR, 1, 0, 0,},

{ 0, 0, 0, 0, 0, 0,}
};

static struct proc_dir_entry *get_endstruct(int low_ino) {
  struct proc_dir_entry *p = base_stuff;
  while (p ->low_ino) {
    if (p->low_ino == low_ino) return p;
    p++;
  }
  printk ("proc_dir_entry for %d not found\n",low_ino);
  return 0;
}


void proc_root_init(void)
{
	static int done = 0;

	if (done)
		return;
	done = 1;
	proc_base_init();
	proc_register(&proc_root, get_endstruct(PROC_LOADAVG));
	proc_register(&proc_root, get_endstruct(PROC_UPTIME));
	proc_register(&proc_root, get_endstruct(PROC_MEMINFO));
	proc_register(&proc_root, get_endstruct(PROC_SERIAL));
	proc_register(&proc_root, get_endstruct(PROC_KMSG));
	proc_register(&proc_root, get_endstruct(PROC_VERSION));
#ifdef CONFIG_PCI
	proc_register(&proc_root, get_endstruct(PROC_PCI));
#endif
	proc_register(&proc_root, get_endstruct(PROC_CPUINFO));
	proc_register(&proc_root, get_endstruct(PROC_SELF));
	proc_register(&proc_root, &proc_net);
	proc_register(&proc_root, &proc_scsi);
	proc_register(&proc_root, &proc_sys_root);

#ifdef CONFIG_DEBUG_MALLOC
	proc_register(&proc_root, get_endstruct(PROC_MALLOC));
#endif
	proc_register(&proc_root, get_endstruct(PROC_KCORE));

#ifdef CONFIG_MODULES
	proc_register(&proc_root, get_endstruct(PROC_MODULES));
	proc_register(&proc_root, get_endstruct(PROC_KSYMS));
#endif
	proc_register(&proc_root, get_endstruct(PROC_STAT));
	proc_register(&proc_root, get_endstruct(PROC_DEVICES));
	proc_register(&proc_root, get_endstruct(PROC_INTERRUPTS));
#ifdef __SMP_PROF__
	proc_register(&proc_root, get_endstruct(PROC_SMP_PROF));
#endif 
	proc_register(&proc_root, get_endstruct(PROC_FILESYSTEMS));
	proc_register(&proc_root, get_endstruct(PROC_DMA));
	proc_register(&proc_root, get_endstruct(PROC_IOPORTS));
	proc_register(&proc_root, get_endstruct(PROC_CMDLINE));
#ifdef CONFIG_RTC
	proc_register(&proc_root, get_endstruct(PROC_RTC));
#endif
	proc_register(&proc_root, get_endstruct(PROC_LOCKS));

	proc_register( &proc_root, get_endstruct(PROC_MTAB));

	if (prof_shift) {
		proc_register(&proc_root, get_endstruct(PROC_PROFILE));
	}
}


int proc_match(int len,const char * name,struct proc_dir_entry * de)
{
	if (!de || !de->low_ino)
		return 0;
	/* "" means "." ---> so paths like "/usr/lib//libc.a" work */
	if (!len && (de->name[0]=='.') && (de->name[1]=='\0'))
		return 1;
	if (de->namelen != len)
		return 0;
	return !memcmp(name, de->name, len);
}

int proc_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	struct proc_dir_entry * de;
	int ino;


	*result = NULL;
	if (!dir || !S_ISDIR(dir->i_mode)) {
		iput(dir);
		return -ENOTDIR;
	}

	de = (struct proc_dir_entry *) dir->u.generic_ip;
	if (!de) {
		iput(dir);
		return -EINVAL;
	}

	/* Special case "." and "..": they aren't on the directory list */
	*result = dir;
	if (!len)
		return 0;
	if (name[0] == '.') {
		if (len == 1)
			return 0;
		if (name[1] == '.' && len == 2) {
			struct inode * inode;
			inode = proc_get_inode(dir->i_sb, de->parent->low_ino, de->parent);
			iput(dir);
			if (!inode)
				return -EINVAL;
			*result = inode;
			return 0;
		}
	}

	*result = NULL;
	for (de = de->subdir; de ; de = de->next) {
		if (proc_match(len, name, de))
			break;
	}
	if (!de) {
		iput(dir);
		return -ENOENT;
	}

	ino = de->low_ino | (dir->i_ino & ~(0xffff));

	if (!(*result = proc_get_inode(dir->i_sb, ino, de))) {
		iput(dir);
		return -EINVAL;
	}
	iput(dir);
	return 0;
}

static int proc_root_lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	unsigned int pid, c;
	int i, ino, retval;


	dir->i_count++;
	retval = proc_lookup(dir, name, len, result);
	if (retval != -ENOENT) {
		iput(dir);
		return retval;
	}
	
	pid = 0;
	while (len-- > 0) {
		c = *name - '0';
		name++;
		if (c > 9) {
			pid = 0;
			break;
		}
		pid *= 10;
		pid += c;
		if (!pid) break;
		if (pid & 0xffff0000) {
			pid = 0;
			break;
		}
	}
	for (i = 0 ; i < NR_TASKS ; i++)
		if (task[i] && task[i]->pid == pid)
			break;
	if (!pid || i >= NR_TASKS) {
		iput(dir);
		return -ENOENT;
	}
	ino = (pid << 16) + PROC_PID_INO;
	if (!(*result = proc_get_inode(dir->i_sb, ino, &proc_pid))) {
		iput(dir);
		return -EINVAL;
	}
	iput(dir);
	return 0;
}

/*
 * This returns non-zero if at EOF, so that the /proc
 * root directory can use this and check if it should
 * continue with the <pid> entries..
 *
 * Note that the VFS-layer doesn't care about the return
 * value of the readdir() call, as long as it's non-negative
 * for success..
 */
int proc_readdir(struct inode * inode, struct file * filp,
	void * dirent, filldir_t filldir)
{
	struct proc_dir_entry * de;
	unsigned int ino;
	int i;

	
	if (!inode || !S_ISDIR(inode->i_mode))
		return -ENOTDIR;
	ino = inode->i_ino;
	de = (struct proc_dir_entry *) inode->u.generic_ip;
	if (!de)
		return -EINVAL;
	i = filp->f_pos;
	switch (i) {
		case 0:
			if (filldir(dirent, ".", 1, i, ino) < 0)
				return 0;
			i++;
			filp->f_pos++;
			/* fall through */
		case 1:
			if (filldir(dirent, "..", 2, i, de->parent->low_ino) < 0)
				return 0;
			i++;
			filp->f_pos++;
			/* fall through */
		default:
			ino &= ~0xffff;
			de = de->subdir;
			i -= 2;
			for (;;) {
				if (!de)
					return 1;
				if (!i)
					break;
				de = de->next;
				i--;
			}

			do {
				if (filldir(dirent, de->name, de->namelen, filp->f_pos, ino | de->low_ino) < 0)
					return 0;
				filp->f_pos++;
				de = de->next;
			} while (de);
	}
	return 1;
}

#define NUMBUF 10

static int proc_root_readdir(struct inode * inode, struct file * filp,
	void * dirent, filldir_t filldir)
{
	char buf[NUMBUF];
	unsigned int nr,pid;
	unsigned long i,j;

	nr = filp->f_pos;
		
		
	if (nr < FIRST_PROCESS_ENTRY) {
		int error = proc_readdir(inode, filp, dirent, filldir);
		if (error <= 0)
			return error;
		filp->f_pos = nr = FIRST_PROCESS_ENTRY;
	}
	
	for (nr -= FIRST_PROCESS_ENTRY; nr < NR_TASKS; nr++, filp->f_pos++) {
		struct task_struct * p = task[nr];

		if (!p || !(pid = p->pid))
			continue;

		j = NUMBUF;
		i = pid;
		do {
			j--;
			buf[j] = '0' + (i % 10);
			i /= 10;
		} while (i);

		if (filldir(dirent, buf+j, NUMBUF-j, filp->f_pos, (pid << 16) + PROC_PID_INO) < 0)
			break;
	}
	return 0;
}
