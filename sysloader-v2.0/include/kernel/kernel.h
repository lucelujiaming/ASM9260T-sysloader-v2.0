/*

Alpha Scale AS3310X alpos
zhang bo, AlpScale Software Engineering,


------------------- Version 1.0  ----------------------
Create File
 heyong 2007-12-6

*/

#ifndef  __KERNEL_H__
#define  __KERNEL_H__

#include <common.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned char uchar;
//typedef unsigned short ushort;
//typedef unsigned long ulong;

typedef volatile u8		AS3310_REG8;
typedef volatile u16	AS3310_REG16;
typedef volatile u32	AS3310_REG32;

/* Macros to access registers */
#define outb(v,p) *(volatile u8 *) (p) = v
#define outw(v,p) *(volatile u16 *) (p) = v
#define outl(v,p) *(volatile u32 *) (p) = v

#define inb(p)	*(volatile u8 *) (p)
#define inw(p)	*(volatile u16 *) (p)
#define inl(p)	*(volatile u32 *) (p)


/* access regs types already known */
#define outx(v,p) *(p) = v

#define inx(p)  *(p)


#define max(a,b)	((a) > (b) ? (a) : (b))
#define min(a,b)	((a) < (b) ? (a) : (b))
#define	abs(a)		((a) < 0 ? -(a) : (a))


#define EOF (-1)
/*
Control Display Macros
 */
#define KEY_LEFT_1 0x1b
#define KEY_LEFT_2 0x5b
#define KEY_LEFT_3 0x44

#define KEY_BACKSPACE_LINUX 0x7f
#define KEY_BACKSPACE_WIN32 0x8

/* macro to get at IO space when running virtually */
#define IO_ADDRESS(x) ((x))


#define dbg_puts(fmt) { if(0) puts(fmt); }
#define dbg_putb(fmt) { if(0) putb(fmt); }
#define dbg_puth(fmt) { if(0) puth(fmt); }

#define as_putc(fmt) { if(1) putc(fmt); }
#define as_puts(fmt) { if(1) puts(fmt); }
#define as_putb(fmt) { if(1) putb(fmt); }
#define as_puth(fmt) { if(1) puth(fmt); }

#define IS_DMA_LCD_FRAME_FLUSH_COMPLETE IS_DMA_APBH_CH1_COMPLETE

int getc(uchar * data);
void serial_putc(uchar data);
void puth(unsigned long x);
#define puts as3310_puts
void as3310_puts(uchar * buffer);

//int strcmp(const char * cs,const char * ct);
int MemDisp_TRL(uchar *buf,int total_length,int size);
long TextToLong_TRL(char *S);
void Pin_assign_dev(int dev);
void Command_loop(void);

void enable_interrupts (void);
int disable_interrupts (void);
int interrupt_init (void);
void Ecc_Init(void);
int do_cmd(char * buffer);
int Get_argv_TRL(uchar * buffer);
void Byte2HEX(uchar *tempstring,uchar data);
void Long2HEX(uchar *tempstring,ulong data);
void putb(uchar x);
//void delay(ulong volatile time);
int argv_detect(void);

void IRQ_print(void);
void dbg_putc(uchar data);
void EMI_init(void);
void clk_init(void);
void base_init();
void test_init(void);
void alp_init(void);
void dbg_hexdump(const void *ptr, int size);

int detect_sdram_config (void);
void base_init(void);

#define PLAT_WIN32 1
#define PLAT_LINUX 2

#define __LEFT(platform)\
do{\
    if (platform == PLAT_WIN32 ) {\
    putc(8);\
    }\
    else if (platform == PLAT_LINUX) {\
       putc(KEY_LEFT_1);putc(KEY_LEFT_2);putc(KEY_LEFT_3);\
    }\
}while(0)


#define __BACKSPACE(platform)\
do{\
    if (platform == PLAT_WIN32 ) {\
    putc(8);putc(' ');putc(8);\
    }\
    else if (platform == PLAT_LINUX) {\
       putc(KEY_LEFT_1);putc(KEY_LEFT_2);putc(KEY_LEFT_3);\
       putc(' ');\
       putc(KEY_LEFT_1);putc(KEY_LEFT_2);putc(KEY_LEFT_3);\
    }\
}while(0)


extern volatile char current_platform;

/* Nand configure */
extern uchar nand_cycles;
extern uchar nand_row_cycles;
extern uchar nand_col_cycles;
extern uchar nand_page_shift;
extern uchar nand_block_shift;
extern uchar nand_current_cen;

extern int watch_dog;

#define get_ms()   (inl(HW_RTC_BASE+HW_RTC_MILLISECONDS))
#define get_us()   (inl(HW_DIGCTL_MICROSECONDS))

#define ENTER_INPUT 0x0d
#define NON_ENTER_INPUT 0xd0

#define MAX_ARG_LENGTH 256
#define MAX_AGRC 10


/*
 * Used for initialization calls..
 */
typedef int (*initcall_t)(void);

extern initcall_t __initcall_start[], __initcall_end[];
extern long __init_struct_start, __init_struct_end;

#define __define_init_task(fn) \
	static t_ops * init_struct_##fn \
	__attribute__((__section__(".init_struct"))) = &fn

#define __define_initcall(level,fn,id) \
	static initcall_t __initcall_##fn##id \
	__attribute__((__section__(".initcall" level ".init"))) = fn

#define __init_device(fn)		__define_initcall("1",fn,1)
#define __init_task(fn)		    __define_initcall("3",fn,3)

void do_initcalls(void);

#define ALIGN4 __attribute__((aligned (4)))
#define ALIGN32 __attribute__((aligned (32)))

#endif




