
/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Stand-alone Header file

------------------- Version 1.0  ----------------------
 He Yong 2006-05-26

*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/types.h>	/* for size_t */
#include <linux/stddef.h>	/* for NULL */
//#include <linux/ctype.h>

#include <autoconf.h>

#include <kernel/config.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/string.h>
#include <kernel/cmd.h>
#include <kernel/device.h>
#include <kernel/dma.h>
#include <kernel/bcb.h>
#include <kernel/mem.h>
#include <kernel/cache-v5te.h>
#include <kernel/vsprintf.h>
#include <kernel/printf.h>
#include <kernel/asmalloc.h>
#include <kernel/pincontrol.h>
#include <kernel/task.h>
#include <kernel/irq.h>
#include <kernel/time.h>
#include <kernel/errno.h>

#ifdef CONFIG_VFS
#include <kernel/vfs.h>
#include <kernel/fat.h>
#endif// CONFIG_VFS

#endif //__COMMON_H__
