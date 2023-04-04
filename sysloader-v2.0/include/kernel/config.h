
/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310d SRAM APP Header file
 
Che log: 

------------------- Version 1.0  ----------------------
switch to Ironpalms OS Loader
 He Yong 2007-7-02
 
------------------- Version 1.3.2  ----------------------
Move main() to main,c
 He Yong 2007-3-18
 
------------------- Version 1.3.1  ----------------------
Support malloc printf mmu, etc..
He Yong 2007-03-18

------------------- Version 1.3  ----------------------
Support NAND NOR UART ROM boot
He Yong 2006-12-04

------------------- Version 1.2  ----------------------
Switch to AS3310C, build in Boot Rom
He Yong 2006-11-06

------------------- Version 1.1  ----------------------
Define all registers IO prots .. etc
He Yong 2006-11-03

------------------- Version 1.0  ----------------------
He Yong 2006-05-26
Create File.
*/


#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Global Var defs*/
extern long __TEXT_START;
extern long __TEXT_END;
extern long __CMD_START;
extern long __CMD_END;
extern long __INT_ENTRY_START;
extern long __RODATA_END;
extern long __NAND_TABLE_START;
extern long __BOOT_INFO_START;
extern long __BSS_START;
extern long __BSS_END;
extern long __STACK_START;
extern long __NONCACHE_MALLOC_START;
extern long __GLUE_7_START;

/* ==================  Configuation =========================*/

#define BANNER_LOGO  "\n\t\t========  Alpha Scale SRAM Loader  ========\n\
\t\t\tAlpscale all rights reserved 2008\n\n"

#define ABOUT_MSG "\n\tAlpha Scale Chip SRAM Loader\n"

#define CMD_LOGO "ASAP18xx @ "
#define CMD_PROMPT_LINUX "# "
#define CMD_PROMPT_WIN32 "> "


    /*      Booting Devices     */
#ifdef CONFIG_BOOT_HEADER
#define BOOT_FROM_UART      1
#else // normal mode
#define BOOT_FROM_UART      0
#endif // DEBUG_MODE


/*      NAND Flash Settings         */
#define CONFIG_USE_NAND_TABLE       0   /*  We don't use nand table in nand init */
#define CONFIG_NAND_CHIP_CEN        0   /*  Nand flash chip for boot */
#define INIT_NAND "setup nand 0"    /*  Nand init cmd */

    /*      SDRAM Config    */
    /* 
    64 MB SDRAM "13 10"
    32 MB SDRAM "13 9"
    16 MB SDRAM "12 9"  -- Default config
    8 MB  SDRAM "12 8"
    */

#define CONFIG_USE_LCD 0
    #define TEXT_LIB 0
    //#define EN_IRQ_IN_PRO
    #define LCD_DEBUG 0
    #define LCD_MORNITOR 0

    /*      CPU CLK Config    */
#define CPU_CLK     180
#define CPU_DIV     1
#define HCLK_DIV    2
#define EMI_DIV     2
#define GMPI_CLK_DIV 3  /*  GPMI CLK DIV by PLL */
#define __PLL_24MHz_ 0


#define __EMI_DEBUG__ 0

#define _DEBUG_CMD 0

#define CONFIG_GPIO_CMD     0
#define CONFIG_I2C          0


#define putc serial_putc



#define ROM_BASE 0x00000000

#define __ROM_RELOCATE
//#define __UART_RELOCATE

#define CODE_SIZE_BOOT_INFO 0x10000 
#define CODE_SIZE ((ulong)&__RODATA_END - (ulong)&__TEXT_START)
#define CODE_START ((ulong)&__TEXT_START)



/*  Device Discriptor   */

#define LOCATION_NAND   0
#define LOCATION_I2C    1
#define LOCATION_NOR    2
#define LOCATION_ROM    3
#define LOCATION_USB    4
#define LOCATION_UART   5

#define LOCATION_SDRAM  9

#endif //__CONFIG_H__
      
