
/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader DMA Header file

------------------- Version 1.0  ----------------------
Create File
 He Yong 2007-03-11

*/
 

#ifndef __DMA_H__
#define __DMA_H__

#include <common.h>

#define DMA_CHANNEL_MIN 0
#define DMA_CHANNEL_MAX 7

/*DMA_APBH_LCD*/
#define HW_APBH_LCD_CTRL0 			0x80080000
#define HW_APBH_LCD_CTRL0_SET 		0x80080004
#define HW_APBH_LCD_CTRL0_CLR 		0x80080008
#define HW_APBH_LCD_CTRL0_TOG 		0x8008000C


#define HW_APBH_LCD_CH0_NXTCMDAR 0x80080040
#define HW_APBH_LCD_CH0_SEMA 0x80080070

#define HW_APBH_LCD_CH1_NXTCMDAR 0x800800B0
#define HW_APBH_LCD_CH1_SEMA 0x800800E0

#define HW_APBH_LCD_CH2_NXTCMDAR 0x80080120
#define HW_APBH_LCD_CH2_SEMA 0x80080150


/*DMA_APBH*/
#define HW_APBH_BASE 			0x80004000

#define HW_APBH_CTRL0 			0x80004000
#define HW_APBH_CTRL0_SET 		0x80004004
#define HW_APBH_CTRL0_CLR 		0x80004008
#define HW_APBH_CTRL0_TOG 		0x8000400C


#define HW_APBH_CTRL1 			0x80004010
#define HW_APBH_CTRL1_SET 		0x80004014
#define HW_APBH_CTRL1_CLR 		0x80004018
#define HW_APBH_CTRL1_TOG 		0x8000401C

/* Channel 0 - 7*/

//  #define HW_APBH_CH0_NXTCMDAR 0x80004040
//  #define HW_APBH_CH0_SEMA 0x80004070
//  
//  #define HW_APBH_CH1_NXTCMDAR 0x800040B0
//  #define HW_APBH_CH1_SEMA 0x800040E0
//  
//  #define HW_APBH_CH2_NXTCMDAR 	0x80004120
//  #define HW_APBH_CH2_SEMA 		0x80004150
//  
//  #define HW_APBH_CH3_NXTCMDAR 	0x80004190
//  #define HW_APBH_CH3_SEMA 		0x800041C0
//  
//  #define HW_APBH_CH4_NXTCMDAR 	0x80004200
//  #define HW_APBH_CH4_SEMA 		0x80004230

#define HW_APBH_CH0_NXTCMDAR  (HW_APBH_BASE+0x40)
#define HW_APBH_CH0_SEMA (HW_APBH_CH0_NXTCMDAR+0x30)

#define HW_APBH_CH1_NXTCMDAR   (HW_APBH_BASE+0xB0)
#define HW_APBH_CH1_SEMA   (HW_APBH_CH1_NXTCMDAR+0x30)

#define HW_APBH_CH2_NXTCMDAR   (HW_APBH_BASE+0x120)
#define HW_APBH_CH2_SEMA   (HW_APBH_CH2_NXTCMDAR+0x30)

#define HW_APBH_CH3_NXTCMDAR   (HW_APBH_BASE+0x190)
#define HW_APBH_CH3_SEMA	(HW_APBH_CH3_NXTCMDAR+0x30)

#define HW_APBH_CH4_NXTCMDAR    (HW_APBH_BASE+0x200)
#define HW_APBH_CH4_SEMA   (HW_APBH_CH4_NXTCMDAR+0x30)

#define HW_APBH_CH5_NXTCMDAR    (HW_APBH_BASE+0x270)
#define HW_APBH_CH5_SEMA        (HW_APBH_CH5_NXTCMDAR+0x30)


#define HW_APBH_DEVSEL 0x80004020

/*DMA_APBX*/
#define HW_APBX_BASE 			0x80024000

#define HW_APBX_CTRL0 			0x80024000
#define HW_APBX_CTRL0_SET 		0x80024004
#define HW_APBX_CTRL0_CLR 		0x80024008
#define HW_APBX_CTRL0_TOG 		0x8002400C

#define HW_APBX_CTRL1 			0x80024010
#define HW_APBX_CTRL1_SET 		0x80024014
#define HW_APBX_CTRL1_CLR 		0x80024018
#define HW_APBX_CTRL1_TOG 		0x8002401C

#define HW_APBX_DEVSEL 			0x80024020


/* Channel 0 - 7*/

#define HW_APBX_CH0_NXTCMDAR  (HW_APBX_BASE+0x40)
#define HW_APBX_CH0_SEMA (HW_APBX_CH0_NXTCMDAR+0x30)

#define HW_APBX_CH1_NXTCMDAR   (HW_APBX_BASE+0xB0)
#define HW_APBX_CH1_SEMA   (HW_APBX_CH1_NXTCMDAR+0x30)

#define HW_APBX_CH2_NXTCMDAR   (HW_APBX_BASE+0x120)
#define HW_APBX_CH2_SEMA   (HW_APBX_CH2_NXTCMDAR+0x30)

#define HW_APBX_CH3_NXTCMDAR   (HW_APBX_BASE+0x190)
#define HW_APBX_CH3_SEMA	(HW_APBX_CH3_NXTCMDAR+0x30)

#define HW_APBX_CH4_NXTCMDAR    (HW_APBX_BASE+0x200)
#define HW_APBX_CH4_SEMA   (HW_APBX_CH4_NXTCMDAR+0x30)


typedef struct as3310_dma_pkg_s{
		u32		NEXT_PKG	;   //	0x00	R/W		
		u32		CTRL        ;  //	0x04	R/W 
		u32		BUFFER      ;  //	0x08	R/W 
		u32		CMD0        ;  //	0x0c	R/W 
		u32		CMD1        ;  //	0x10	R/W 
} /*__attribute__((__packed__))*/ AS3310_DMA_PKG;

/* define error codes */
#define AS3310_DMA_CHANNEL_OK       0
#define AS3310_DMA_CHANNEL_BUSY     1
#define AS3310_DMA_CHANNEL_INVALID  2

/* define chain status codes */
#define AS3310_DMA_STAT_READY 0
#define AS3310_DMA_STAT_BUSY  -1

struct as3310_dma_chain {
    int channel_num;
    int status;
    int pkg_num;
    char * own_dev; /* a char string of the device name */
    void * chain_phy_addr; /* physical addr of 1st dma package */
    struct as3310_dma_pkg_s * chain_head; /* point to the 1st dma package */
};


 /*
init_as3310_dma_chain
inputs:
struct device * dev the device which use this dma chain
int pkg_num,        number of DMA package in this DMA chain
int channel_num,    channel number
*/
struct as3310_dma_chain * request_as3310_dma_chain(int pkg_num,int channel_num);

 /*
free_as3310_dma_chain
inputs:
struct device * dev the device which use this dma chain
dma_chain,          dma_chain struct which need free
*/
void free_as3310_dma_chain(struct as3310_dma_chain * dma_chain);

 /*
Start APBX DMA Chain
inputs:
ulong pkg_addr, Physical address of DMA chain entry
int pkg_num,    number of DMA package in this DMA chain
int ch_num,     channel number
*/
int dma_start_apbx(ulong pkg_addr,int pkg_num,int ch_num);

 /*
Start APBH DMA Chain
inputs:
ulong pkg_addr, Physical address of DMA chain entry
int pkg_num,    number of DMA package in this DMA chain
int ch_num,     channel number
*/
int dma_start_apbh(ulong pkg_addr,int pkg_num,int ch_num);

int dma_apbh_init(void);
int dma_apbx_init(void);

int dma_start_lcd_apbh(ulong pkg_addr,int pkg_num,int ch_num);

int dma_lcd_init();

#define is_apbh_complete(ch) \
    ((inl(HW_APBH_CH0_SEMA + ((ch)*0x70))&0x00ff0000)==0)

#define is_apbx_complete(ch)\
    ((inl(HW_APBX_CH0_SEMA + ((ch)*0x70))&0x00ff0000)==0)    



#define IS_DMA_LCD_COMPLETE() ((inl(HW_APBH_LCD_CH0_SEMA)&0x00ff0000)==0)

#define IS_DMA_APBH_CH1_COMPLETE() ((inl(HW_APBH_CH1_SEMA)&0x00ff0000)==0)
#define IS_DMA_APBH_CH2_COMPLETE() ((inl(HW_APBH_CH2_SEMA)&0x00ff0000)==0)
#define IS_DMA_APBH_CH3_COMPLETE() ((inl(HW_APBH_CH3_SEMA)&0x00ff0000)==0)
#define IS_DMA_APBH_CH4_COMPLETE() ((inl(HW_APBH_CH4_SEMA)&0x00ff0000)==0)

#endif //__DMA_H__
