#ifndef __SD_MMC_H__
#define __SD_MMC_H__

#define SSP_PAD_BANK 0
#define SSP_PAD_PIN_LOW 26
#define SSP_PAD_PIN_HIGH 31
#define SSP_DETECT_PIN  25
#define SSP_PAD_TYPE 0


#define SECTOR_SHIFT 9 // 4096 bytes


#define AS3310_SSP_BASE         0x80010000

typedef struct _ssp_reg{
      AS3310_REG32 ctrl0[4];            //  0x00
      AS3310_REG32 cmd0[4];             //  0x10
      AS3310_REG32 cmd1[4];             //  0x20
      AS3310_REG32 compref[4];          //  0x30
      AS3310_REG32 compmask[4];         //  0x40
      AS3310_REG32 timing[4];           //  0x50
      AS3310_REG32 ctrl1[4];            //  0x60
      AS3310_REG32 data[4];             //  0x70
      AS3310_REG32 sdresp0[4];          //  0x80
      AS3310_REG32 sdresp1[4];          //  0x90
      AS3310_REG32 sdresp2[4];          //  0xa0
      AS3310_REG32 sdresp3[4];          //  0xb0
      AS3310_REG32 status[4];           //  0xc0
      AS3310_REG32 debug[4];            //  0xd0
}AS3310_SSP;

static inline AS3310_SSP * const AS3310_GetBase_SSP(void)
{
	return (AS3310_SSP * const)AS3310_SSP_BASE;
}

typedef struct _resp_struct{
	u32 resp[4];
}sd_mmc_resp;

typedef struct _commond_struct{
	u32 opcode;
	u32 argument;
	u32 flag;
}sd_mmc_commond;

typedef struct _sd_mmc_card{
	char card_type;
    u32 relative_addr;
	u32 ocr;
	u32 cid[4];
	u32 csd[4];
	u32 capacity;
	char prod_name[5];
}sd_mmc_card;


#define TIMEOUT -2

#define NO_RESP         1 
#define LONG_RESP       2
#define APP_CMD         3

#define GET_RESPONSE (1<<17)
#define LONG_RESPONSE (1<<19)

#define SD_CARD 1
#define MMC_CARD 2
#define NO_CARD 0

/* SD commands                           type  argument     response */
  /* class 0 */
/* This is basically the same command as for MMC with some quirks. */
#define SD_SEND_RELATIVE_ADDR     3   /* bcr                     R6  */
#define SD_SEND_IF_COND           8   /* bcr  [11:0] See below   R7  */

  /* class 10 */
#define SD_SWITCH                 6   /* adtc [31:0] See below   R1  */

  /* Application commands */
#define SD_APP_SET_BUS_WIDTH      6   /* ac   [1:0] bus width    R1  */
#define SD_APP_SEND_NUM_WR_BLKS  22   /* adtc                    R1  */
#define SD_APP_OP_COND           41   /* bcr  [31:0] OCR         R3  */
#define SD_APP_SEND_SCR          51   /* adtc                    R1  */

/*
 * SD_SWITCH argument format:
 *
 *      [31] Check (0) or switch (1)
 *      [30:24] Reserved (0)
 *      [23:20] Function group 6
 *      [19:16] Function group 5
 *      [15:12] Function group 4
 *      [11:8] Function group 3
 *      [7:4] Function group 2
 *      [3:0] Function group 1
 */

/*
 * SD_SEND_IF_COND argument format:
 *
 *	[31:12] Reserved (0)
 *	[11:8] Host Voltage Supply Flags
 *	[7:0] Check Pattern (0xAA)
 */

/*
 * SCR field definitions
 */

#define SCR_SPEC_VER_0		0	/* Implements system specification 1.0 - 1.01 */
#define SCR_SPEC_VER_1		1	/* Implements system specification 1.10 */
#define SCR_SPEC_VER_2		2	/* Implements system specification 2.00 */

/*
 * SD bus widths
 */
#define SD_BUS_WIDTH_1		0
#define SD_BUS_WIDTH_4		2

/*
 * SD_SWITCH mode
 */
#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SET		1

/*
 * SD_SWITCH function groups
 */
#define SD_SWITCH_GRP_ACCESS	0

/*
 * SD_SWITCH access modes
 */
#define SD_SWITCH_ACCESS_DEF	0
#define SD_SWITCH_ACCESS_HS	1

/* Standard MMC commands (4.1)           type  argument     response */
   /* class 1 */
#define	MMC_GO_IDLE_STATE         0   /* bc                          */
#define MMC_SEND_OP_COND          1   /* bcr  [31:0] OCR         R3  */
#define MMC_ALL_SEND_CID          2   /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR     3   /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR               4   /* bc   [31:16] RCA            */
#define MMC_SWITCH                6   /* ac   [31:0] See below   R1b */
#define MMC_SELECT_CARD           7   /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD          8   /* adtc                    R1  */
#define MMC_SEND_CSD              9   /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID             10   /* ac   [31:16] RCA        R2  */
#define MMC_READ_DAT_UNTIL_STOP  11   /* adtc [31:0] dadr        R1  */
#define MMC_STOP_TRANSMISSION    12   /* ac                      R1b */
#define MMC_SEND_STATUS	         13   /* ac   [31:16] RCA        R1  */
#define MMC_GO_INACTIVE_STATE    15   /* ac   [31:16] RCA            */

  /* class 2 */
#define MMC_SET_BLOCKLEN         16   /* ac   [31:0] block len   R1  */
#define MMC_READ_SINGLE_BLOCK    17   /* adtc [31:0] data addr   R1  */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0] data addr   R1  */

  /* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20   /* adtc [31:0] data addr   R1  */

  /* class 4 */
#define MMC_SET_BLOCK_COUNT      23   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_SINGLE_BLOCK   24   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25   /* adtc                    R1  */
#define MMC_PROGRAM_CID          26   /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27   /* adtc                    R1  */

  /* class 6 */
#define MMC_SET_WRITE_PROT       28   /* ac   [31:0] data addr   R1b */
#define MMC_CLR_WRITE_PROT       29   /* ac   [31:0] data addr   R1b */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0] wpdata addr R1  */

  /* class 5 */
#define MMC_ERASE_GROUP_START    35   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE_GROUP_END      36   /* ac   [31:0] data addr   R1  */
#define MMC_ERASE                38   /* ac                      R1b */

  /* class 9 */
#define MMC_FAST_IO              39   /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE         40   /* bcr                     R5  */

  /* class 7 */
#define MMC_LOCK_UNLOCK          42   /* adtc                    R1b */

  /* class 8 */
#define MMC_APP_CMD              55   /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD              56   /* adtc [0] RD/WR          R1  */


/*
 * EXT_CSD fields
 */

#define EXT_CSD_BUS_WIDTH	183	/* R/W */
#define EXT_CSD_HS_TIMING	185	/* R/W */
#define EXT_CSD_CARD_TYPE	196	/* RO */
#define EXT_CSD_SEC_CNT		212	/* RO, 4 bytes */
#define EXT_CSD_REV 		192	/* RO */


/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1<<0)
#define EXT_CSD_CMD_SET_SECURE		(1<<1)
#define EXT_CSD_CMD_SET_CPSECURE	(1<<2)

#define EXT_CSD_CARD_TYPE_26	(1<<0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1<<1)	/* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */

/*
 * MMC_SWITCH access modes
 */

#define MMC_SWITCH_MODE_CMD_SET		0x00	/* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01	/* Set bits which are 1 in value */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02	/* Clear bits which are 1 in value */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	/* Set target to value */




#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})



#define MMC_CARD_BUSY	0x80000000	/* Card Power up status bit */

// dma director
#define READ   1
#define WRITE  0


// sd mmc ioctl commond definition
#define SD_MMC_IOCTL_GET_SECTOR_NUM 1
#define SD_MMC_IOCTL_GET_SECTOR_SHIFT 2

void SSP_dma_trans(char, int, int, void *);
void SSP_pin_assign();
void SSP_controller_init();
int sd_mmc_probe();
sd_mmc_resp send_commond(sd_mmc_commond);
int sd_mmc_card_init();
void go_idle();
void mmc_send_if_cond();
int sd_send_app_op_cond();
void all_send_cid();
int send_relative_addr();
void select_card();
void mmc_app_send_scr();
void app_set_bus_width();
void set_block_length();
int sd_mmc_detect_irq();
int clear_detect_pin_irq();
int ssp_irq_handler();

#define SSP_CLKCTRL             0x80040080


#endif // __SD_MMC_H__

