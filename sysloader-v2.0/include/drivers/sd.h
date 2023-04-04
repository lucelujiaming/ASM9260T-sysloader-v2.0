

#include "common.h"

#ifndef __SD_H__
#define __SD_H__

#define SSP_CTRL0        	    0x80060000
#define SSP_CMD0                0x80060010
#define SSP_CMD1                0x80060020
#define SSP_COMPREF             0x80060030
#define SSP_COMPMASK            0x80060040
#define SSP_TIMING              0x80060050
#define SSP_CTRL1               0x80060060
#define SSP_DATA                0x80060070
#define SSP_SDRESP0             0x80060080
#define SSP_SDRESP1             0x80060090
#define SSP_SDRESP2             0x800600A0
#define SSP_SDRESP3             0x800600B0
#define SSP_STATUS              0x800600C0
#define SSP_DEBUG               0x80060100

#define SSP_CLKCTRL             0x80040080
#define PIN_CTRL                0x80018000
#define SSP_PIN                 0x80018020
#define SSP_8MA                 0x80018030

// DMA register
#define DMA_CTRL0    0x80024000
#define DMA_CTRL1    0x80024010
#define CH4_CURCMDAR 0x800241f0
#define CH4_NXTCMDAR 0x80024200
#define CH4_CMD      0x80024210
#define CH4_BAR      0x80024220
#define CH4_SEMA     0x80024230


// DMA package address
#define DMA_CMD_PKG_ADDR1	0x40009000
#define DMA_CMD_PKG_ADDR2	0x40009040
#define DMA_CMD_PKG_ADDR3	0x40008100
#define DMA_CMD_PKG_ADDR4	0x40009140
#define TX_BASE_ADDR	        0xffff0000     
#define RX_BASE_ADDR	        0x40009200 

#define _1BIT_ 0
#define _4BIT_ 1


typedef struct _ssp_reg{
      AS3310_REG32 ctrl0[4];
      AS3310_REG32 cmd0[4];
      AS3310_REG32 cmd1[4];
      AS3310_REG32 compref[4];
      AS3310_REG32 compmask[4];
      AS3310_REG32 timing[4];
      AS3310_REG32 ctrl1[4];
      AS3310_REG32 data[4];
      AS3310_REG32 sdresp0[4];
      AS3310_REG32 sdresp1[4];
      AS3310_REG32 sdresp2[4];
      AS3310_REG32 sdresp3[4];
      AS3310_REG32 status[4];
      AS3310_REG32 debug[4];
}ssp_reg;

void send_acmd41();
void send_cmd(ulong cmd0,ulong cmd1);

#endif //__SD_H__
