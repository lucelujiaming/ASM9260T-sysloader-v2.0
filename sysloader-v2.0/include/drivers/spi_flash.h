/******************************************************
* AlphaScale ASAP18xx BootCode
*  
* spi_flash.h 
*  
* SPI-Flash boot support
* 
* Copyright (C) 2009 AlphaScale Inc.
* He Yong <hoffer@sjtu.org>
* 
* ------------------- Modifications  ------------------
* Create File, Support 4 types of SPI Flash
*  He Yong 2009-05-16
* 
******************************************************/

#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

typedef volatile unsigned char  AS_REG8;
typedef volatile unsigned short AS_REG16;
typedef volatile unsigned long  AS_REG32;

#define SPI_FLASH_CMD_READ_DUMMY    0x0B
#define SPI_FLASH_CMD_READ          0x03
#define SPI_FLASH_CMD_PROTECT_SECTOR  0x36
#define SPI_FLASH_CMD_UNPROTECT_SECTOR  0x39
#define SPI_FLASH_CMD_WRITE         0x02
#define SPI_FLASH_CMD_WRITE_ENABLE  0x06
#define SPI_FLASH_CMD_WRITE_DISABLE 0x04
#define SPI_FLASH_CMD_ERASE_4K      0x20
#define SPI_FLASH_CMD_ERASE_32K     0x52
#define SPI_FLASH_CMD_ERASE_64K     0xD8
#define SPI_FLASH_CMD_ERASE_ALL     0x60 // 0xC7
#define SPI_FLASH_CMD_READ_STATUS   0x05
#define SPI_FLASH_CMD_WRITE_STATUS  0x01
#define SPI_FLASH_CMD_READ_ID       0x9F

/*  ========================================= */
/*  =============  IO Interface ============= */
/*  ========================================= */

typedef struct _spi_reg{
      AS_REG32 ctrl0[4];            //  0x00
      AS_REG32 ctrl1[4];            //  0x10
      AS_REG32 timing[4];           //  0x20
      AS_REG32 data[4];             //  0x30
      AS_REG32 status[4];           //  0x40
      AS_REG32 debug[4];            //  0x50
}ASAP18XX_SPI;

#define ASAP18XX_SPI_BASE         0x8002c000

#define SSP_CLKCTRL             0x80040080

#define MODE_NORMAL     0
#define MODE_BACKUP_1   1 // NAND D[7-4]
#define SPI_CLK_DIV_MODE_NORMAL 6

int spi_flash_init(int io_mode, int spi_clk_div, int internal_clk_div);


#define SPI_FLASH_PAGESHIFT  8 // 256 Byte

int spi_flash_read(u8 * buf, u32 faddr, int count);
#define IS_DMA_APBH_CH5_COMPLETE() ((inl(HW_APBH_CH5_SEMA)&0x00ff0000)==0)

#endif //__SPI_FLASH_H__

