/******************************************************
* AlphaScale ASAP18xx BootCode
*
* nand_hw.c
*
* Nand Flash Low-Level Support file
*
* Copyright (C) 2009 AlphaScale Inc.
* He Yong <hoffer@sjtu.org>
*
* ------------------- Modifications  ------------------
* ------------------- Version 1.0  ----------------------
* Create File, Support Nand Flash
*  He Yong 2009-11-19
*
******************************************************/

#include <common.h>
#include <drivers/nand_search.h>
// ================== Definitions ====================

// timing parameters

#define  TITC  0x0
#define  TWHR  0x6
#define  TRHW  0x6
#define  TADL  0x0
#define  TCCS  0x0

#define  TWH   0x8
#define  TWP   0x8

#define  TCAD  0x0

// cmd parameters
#define  RESET                        0xFF
#define  SYNC_RESET                   0xFC

#define  READ_ID                      0x90

#define  READ_STATUS                  0x70
#define  READ_STATUS_ENHANCE          0x78

#define  CHANGE_WRITE_COLUMN          0x85
#define  CHANGE_ROW_ADDRESS           0x85

#define  READ_PAGE_1                  0x00
#define  READ_PAGE_2                  0x30

#define  PROGRAM_PAGE_1               0x80
#define  PROGRAM_PAGE_2               0x10

#define  PROGRAM_PAGE1                0x80

#define  WRITE_PAGE                   0x10
#define  WRITE_PAGE_CACHE             0x15
#define  WRITE_MULTIPLANE             0x11

#define  ERASE_BLOCK_1                0x60
#define  ERASE_BLOCK_2                0xD0



// seq parameter
#define  SEQ1     0x21   // 6'b100001
#define  SEQ2     0x22   // 6'b100010
#define  SEQ4     0x24   // 6'b100100
#define  SEQ5     0x25   // 6'b100101
#define  SEQ6     0x26   // 6'b100110
#define  SEQ7     0x27   // 6'b100111
#define  SEQ9     0x29   // 6'b101001
#define  SEQ10    0x2A   // 6'b101010
#define  SEQ11    0x2B   // 6'b101011
#define  SEQ15    0x2F   // 6'b101111
#define  SEQ0     0x00   // 6'b000000
#define  SEQ3     0x03   // 6'b000011
#define  SEQ8     0x08   // 6'b001000
#define  SEQ12    0x0C   // 6'b001100
#define  SEQ13    0x0D   // 6'b001101
#define  SEQ14    0x0E   // 6'b001110
#define  SEQ16    0x30   // 6'b110000
#define  SEQ17    0x11   // 6'b010001

// cmd register
#define  ADDR_SEL_0    0x0
#define  ADDR_SEL_1    0x1

#define  INPUT_SEL_BIU  0x0
#define  INPUT_SEL_DMA  0x1

// control register parameter
#define  DISABLE_STATUS    1
#define  EN_STATUS         0

#define  RNB_SEL           0
#define  NO_RNB_SEL        1

#define  BIG_BLOCK_EN      0
#define  SMALL_BLOCK_EN    1

#define  LOOKUP_EN         1
#define  LOOKUP_DIS        0

#define  WORK_MODE_ASYNC   0
#define  WORK_MODE_SYNC    1

#define  PROT_EN           0
#define  PROT_DIS          1

#define  IO_WIDTH_8        0
#define  IO_WIDTH_16       1

#define  DATA_SIZE_FULL_PAGE  0
#define  DATA_SIZE_CUSTOM     1

#define  PAGE_SIZE_256B        0x0
#define  PAGE_SIZE_512B        0x1
#define  PAGE_SIZE_1024B       0x2
#define  PAGE_SIZE_2048B       0x3
#define  PAGE_SIZE_4096B       0x4
#define  PAGE_SIZE_8192B       0x5
#define  PAGE_SIZE_16384B      0x6
#define  PAGE_SIZE_32768B      0x7
#define  PAGE_SIZE_0B          0x0

#define  BLOCK_SIZE_32P        0x0
#define  BLOCK_SIZE_64P        0x1
#define  BLOCK_SIZE_128P       0x2
#define  BLOCK_SIZE_256P       0x3

#define  ECC_DIS          0
#define  ECC_EN           1

#define  INT_DIS          0
#define  INT_EN           1

#define  SPARE_DIS        0
#define  SPARE_EN         1

#define  ADDR0_AUTO_INCR_DIS  0
#define  ADDR0_AUTO_INCR_EN   1

#define  ADDR1_AUTO_INCR_DIS  0
#define  ADDR1_AUTO_INCR_EN   1

#define  ADDR_CYCLE_0      0x0
#define  ADDR_CYCLE_1      0x1
#define  ADDR_CYCLE_2      0x2
#define  ADDR_CYCLE_3      0x3
#define  ADDR_CYCLE_4      0x4
#define  ADDR_CYCLE_5      0x5

// int_mask register
#define  FIFO_ERROR_DIS  0
#define  FIFO_ERROR_EN   1

#define  MEM7_RDY_DIS    0
#define  MEM7_RDY_EN     1

#define  MEM6_RDY_DIS    0
#define  MEM6_RDY_EN     1

#define  MEM5_RDY_DIS    0
#define  MEM5_RDY_EN     1

#define  MEM4_RDY_DIS    0
#define  MEM4_RDY_EN     1

#define  MEM3_RDY_DIS    0
#define  MEM3_RDY_EN     1

#define  MEM2_RDY_DIS    0
#define  MEM2_RDY_EN     1

#define  MEM1_RDY_DIS    0
#define  MEM1_RDY_EN     1

#define  MEM0_RDY_DIS    0
#define  MEM0_RDY_EN     1

#define  ECC_TRSH_ERR_DIS  0
#define  ECC_TRSH_ERR_EN   1

#define  ECC_FATAL_ERR_DIS 0
#define  ECC_FATAL_ERR_EN  1

#define  CMD_END_INT_DIS   0
#define  CMD_END_INT_EN    1

#define  PROT_INT_DIS   0
#define  PROT_INT_EN    1

//ecc ctrl register
#define  ECC_WORD_POS_SPARE  1
#define  ECC_WORD_POS_DATA   0

#define  ECC_THRESHOLD_0     0x0
#define  ECC_THRESHOLD_1     0x1
#define  ECC_THRESHOLD_2     0x2
#define  ECC_THRESHOLD_3     0x3
#define  ECC_THRESHOLD_4     0x4
#define  ECC_THRESHOLD_5     0x5
#define  ECC_THRESHOLD_6     0x6
#define  ECC_THRESHOLD_7     0x7
#define  ECC_THRESHOLD_8     0x8
#define  ECC_THRESHOLD_9     0x9
#define  ECC_THRESHOLD_10    0xA
#define  ECC_THRESHOLD_11    0xB
#define  ECC_THRESHOLD_12    0xC
#define  ECC_THRESHOLD_13    0xD
#define  ECC_THRESHOLD_14    0xE
#define  ECC_THRESHOLD_15    0xF

#define  ECC_CAP_2    0x0
#define  ECC_CAP_4    0x1
#define  ECC_CAP_6    0x2
#define  ECC_CAP_8    0x3
#define  ECC_CAP_10   0x4
#define  ECC_CAP_12   0x5
#define  ECC_CAP_14   0x6
#define  ECC_CAP_16   0x7

// boot parameter
#define  BOOT_REQ      0x1


typedef struct
{
    AS_REG32  REG_NAND_COMMAND          ;   // (NAND_BASE_ADDR+0x00)-
    #define NAND_CTRL_FD_CUSTOM_SIZE    11
    #define NAND_CTRL_FD_ECC_EN         5
    AS_REG32  REG_NAND_CONTROL          ;   // (NAND_BASE_ADDR+0x04)
    AS_REG32  REG_NAND_STATUS           ;   // (NAND_BASE_ADDR+0x08)
    AS_REG32  REG_NAND_INT_MASK         ;   // (NAND_BASE_ADDR+0x0c)
    AS_REG32  REG_NAND_INT_STATUS       ;   // (NAND_BASE_ADDR+0x10)-
    AS_REG32  REG_NAND_ECC_CTRL         ;   // (NAND_BASE_ADDR+0x14)
    AS_REG32  REG_NAND_ECC_OFFSET       ;   // (NAND_BASE_ADDR+0x18)
    AS_REG32  REG_NAND_ADDR0_L          ;   // (NAND_BASE_ADDR+0x1C)
    AS_REG32  REG_NAND_ADDR1_L          ;   // (NAND_BASE_ADDR+0x20)-
    AS_REG32  REG_NAND_ADDR0_H          ;   // (NAND_BASE_ADDR+0x24)
    AS_REG32  REG_NAND_ADDR1_H          ;   // (NAND_BASE_ADDR+0x28)
    AS_REG32  REG_NAND_RSVD0            ;   // (NAND_BASE_ADDR+0x2c)
    AS_REG32  REG_NAND_SPARE_SIZE       ;   // (NAND_BASE_ADDR+0x30)-
    AS_REG32  REG_NAND_RSVD1            ;   // (NAND_BASE_ADDR+0x34)
    AS_REG32  REG_NAND_PROTECT          ;   // (NAND_BASE_ADDR+0x38)
    AS_REG32  REG_NAND_RSVD2            ;   // (NAND_BASE_ADDR+0x3c)
    AS_REG32  REG_NAND_LOOKUP_EN        ;   // (NAND_BASE_ADDR+0x40)-
    AS_REG32  REG_NAND_LOOKUP0          ;   // (NAND_BASE_ADDR+0x44)
    AS_REG32  REG_NAND_LOOKUP1          ;   // (NAND_BASE_ADDR+0x48)
    AS_REG32  REG_NAND_LOOKUP2          ;   // (NAND_BASE_ADDR+0x4C)
    AS_REG32  REG_NAND_LOOKUP3          ;   // (NAND_BASE_ADDR+0x50)-
    AS_REG32  REG_NAND_LOOKUP4          ;   // (NAND_BASE_ADDR+0x54)
    AS_REG32  REG_NAND_LOOKUP5          ;   // (NAND_BASE_ADDR+0x58)
    AS_REG32  REG_NAND_LOOKUP6          ;   // (NAND_BASE_ADDR+0x5C)
    AS_REG32  REG_NAND_LOOKUP7          ;   // (NAND_BASE_ADDR+0x60)-
    AS_REG32  REG_NAND_DMA_ADDR         ;   // (NAND_BASE_ADDR+0x64)
    AS_REG32  REG_NAND_DMA_CNT          ;   // (NAND_BASE_ADDR+0x68)
    AS_REG32  REG_NAND_DMA_CTRL         ;   // (NAND_BASE_ADDR+0x6C)
    AS_REG32  REG_NAND_RSVD3            ;   // (NAND_BASE_ADDR+0x70)-
    AS_REG32  REG_NAND_RSVD4            ;   // (NAND_BASE_ADDR+0x74)
    AS_REG32  REG_NAND_RSVD5            ;   // (NAND_BASE_ADDR+0x78)
    AS_REG32  REG_NAND_RSVD6            ;   // (NAND_BASE_ADDR+0x7c)
    AS_REG32  REG_NAND_MEM_CTRL         ;   // (NAND_BASE_ADDR+0x80)-
    AS_REG32  REG_NAND_DATA_SIZE        ;   // (NAND_BASE_ADDR+0x84)
    AS_REG32  REG_NAND_READ_STATUS      ;   // (NAND_BASE_ADDR+0x88)
    AS_REG32  REG_NAND_TIME_SEQ         ;   // (NAND_BASE_ADDR+0x8C)
    AS_REG32  REG_NAND_TIMINGS_ASYN     ;   // (NAND_BASE_ADDR+0x90)-
    AS_REG32  REG_NAND_TIMINGS_SYN      ;   // (NAND_BASE_ADDR+0x94)
    AS_REG32  REG_NAND_FIFO_DATA        ;   // (NAND_BASE_ADDR+0x98)
    AS_REG32  REG_NAND_TIME_MODE        ;   // (NAND_BASE_ADDR+0x9C)
    AS_REG32  REG_NAND_DMA_ADDR_OFFSET  ;   // (NAND_BASE_ADDR+0xA0)-
    AS_REG32  REG_NAND_RSVD7            ;   // (NAND_BASE_ADDR+0xa4)
    AS_REG32  REG_NAND_RSVD8            ;   // (NAND_BASE_ADDR+0xa8)
    AS_REG32  REG_NAND_RSVD9            ;   // (NAND_BASE_ADDR+0xac)
    AS_REG32  REG_NAND_FIFO_INIT        ;   // (NAND_BASE_ADDR+0xB0)
} /*__attribute__((__packed__))*/ ASAP1826_NAND_CTRL;

#define  NAND_BASE_ADDR         0x80600000

static inline ASAP1826_NAND_CTRL *   ASAP1826_GetBase_NAND(void)
{
    return(ASAP1826_NAND_CTRL *  )NAND_BASE_ADDR;
}

#define  NAND_COMMAND           (NAND_BASE_ADDR+0x00)
#define  NAND_CONTROL           (NAND_BASE_ADDR+0x04)
#define  NAND_STATUS            (NAND_BASE_ADDR+0x08)
#define  NAND_INT_MASK          (NAND_BASE_ADDR+0x0c)
#define  NAND_INT_STATUS        (NAND_BASE_ADDR+0x10)
#define  NAND_ECC_CTRL          (NAND_BASE_ADDR+0x14)
#define  NAND_ECC_OFFSET        (NAND_BASE_ADDR+0x18)
#define  NAND_ADDR0_H           (NAND_BASE_ADDR+0x24)
#define  NAND_ADDR0_L           (NAND_BASE_ADDR+0x1C)
#define  NAND_ADDR1_H           (NAND_BASE_ADDR+0x28)
#define  NAND_ADDR1_L           (NAND_BASE_ADDR+0x20)
#define  NAND_SPARE_SIZE        (NAND_BASE_ADDR+0x30)
#define  NAND_PROTECT           (NAND_BASE_ADDR+0x38)
#define  NAND_LOOKUP_EN         (NAND_BASE_ADDR+0x40)
#define  NAND_LOOKUP0           (NAND_BASE_ADDR+0x44)
#define  NAND_LOOKUP1           (NAND_BASE_ADDR+0x48)
#define  NAND_LOOKUP2           (NAND_BASE_ADDR+0x4C)
#define  NAND_LOOKUP3           (NAND_BASE_ADDR+0x50)
#define  NAND_LOOKUP4           (NAND_BASE_ADDR+0x54)
#define  NAND_LOOKUP5           (NAND_BASE_ADDR+0x58)
#define  NAND_LOOKUP6           (NAND_BASE_ADDR+0x5C)
#define  NAND_LOOKUP7           (NAND_BASE_ADDR+0x60)
#define  NAND_DMA_ADDR          (NAND_BASE_ADDR+0x64)
#define  NAND_DMA_CNT           (NAND_BASE_ADDR+0x68)
#define  NAND_DMA_CTRL          (NAND_BASE_ADDR+0x6C)
#define  NAND_MEM_CTRL          (NAND_BASE_ADDR+0x80)
#define  NAND_DATA_SIZE         (NAND_BASE_ADDR+0x84)
#define  NAND_READ_STATUS       (NAND_BASE_ADDR+0x88)
#define  NAND_TIME_SEQ          (NAND_BASE_ADDR+0x8C)
#define  NAND_TIMINGS_ASYN      (NAND_BASE_ADDR+0x90)
#define  NAND_TIMINGS_SYN       (NAND_BASE_ADDR+0x94)
#define  NAND_FIFO_DATA         (NAND_BASE_ADDR+0x98)
#define  NAND_TIME_MODE         (NAND_BASE_ADDR+0x9C)
#define  NAND_DMA_ADDR_OFFSET   (NAND_BASE_ADDR+0xA0)
#define  NAND_FIFO_INIT         (NAND_BASE_ADDR+0xB0)


#define HW_DIGCTL_NAND_CTRL     0x8001C100
#define HW_DIGCTL_NAND_BOOT0    0x8001C110
#define HW_DIGCTL_NAND_BOOT1    0x8001C120
#define HW_DIGCTL_NAND_BOOT2    0x8001C130
#define HW_DIGCTL_NAND_BOOT3    0x8001C140
#define HW_DIGCTL_NAND_BOOT4    0x8001C150
#define HW_DIGCTL_NAND_BOOT5    0x8001C160
#define HW_DIGCTL_NAND_BOOT6    0x8001C170

// ========= Global Vars and Functions ===========

char ALIGN4 nand_tmp_addr[8];
void convert_nand_page_addr(nand_info * nand_i,int page,
                            int type, char * addr_array);
void Nand_Read_ID(nand_info * nand_i,ulong* dev_id,int n_byte,uchar cmd);
void select_nand_cen(int cen);
int AS3310_Nand_Read_Page(nand_info * nand_i,int page,
                          char * buffer,char * oob,int type,int len);

int oob_flag = 0x00B00000;
void select_nand_cen(int cen)
{
    nand_info * nand_i;
    ASAP1826_NAND_CTRL * nand_ctrl;

    nand_ctrl = ASAP1826_GetBase_NAND();
    nand_i = get_nand_info();

   // as_puts("Nand Chip ");as_putb(cen);as_putc('\n');
    nand_i->chip = cen;

    nand_ctrl->REG_NAND_MEM_CTRL= (0xff00 |  cen) ;
    nand_ctrl->REG_NAND_MEM_CTRL ^= (1UL<<(cen+8)); // clear WP reg

    /* Cen Pin Assign */
    /*
    if ( cen <= 1 )
    {
        // GPIO3[1-0]
        set_pin_mux(3,cen,1);
        // outl(0x3 ,HW_PINCTRL_MUXSEL4 + 8);  // GPIO2[D0]
    }
    else
    {
        // GPIO0[15-14]
        set_pin_mux(0,12 + cen,2);
        //  outl(0x3 << ((cen)*2 + 2) ,HW_PINCTRL_MUXSEL1 + 8);
    }*/
}


void Nand_Read_ID(nand_info * nand_i,ulong* dev_id,int n_byte,uchar cmd){

    ASAP1826_NAND_CTRL * nand_ctrl;
    nand_ctrl = ASAP1826_GetBase_NAND();

    nand_ctrl->REG_NAND_CONTROL |=  (1UL<<NAND_CTRL_FD_CUSTOM_SIZE);  // set custom size
    nand_ctrl->REG_NAND_CONTROL &= ( ~(1UL<<NAND_CTRL_FD_ECC_EN));  // disable ecc
    nand_ctrl->REG_NAND_DATA_SIZE = n_byte;

    // address 0x00000000
    nand_ctrl->REG_NAND_ADDR0_L = 0;

    nand_ctrl->REG_NAND_COMMAND = (cmd<<8) |
                              (ADDR_SEL_0<<7) |
                              (INPUT_SEL_BIU<<6) |
                              (SEQ1) ;

    // wait for device ready
    while (!((nand_ctrl->REG_NAND_STATUS) &  (1<<nand_i->chip))) {} ;

    while (n_byte > 0)
    {
        *dev_id++ =  nand_ctrl->REG_NAND_FIFO_DATA;
        n_byte -= 4;
    };
}

int NAND_init(nand_info * nand_i)
{
   // nand_info * nand_i;
    ASAP1826_NAND_CTRL * nand_ctrl;
    int error;
    int twh;
    int twp;
    int twhr;
    int trhw;
    int titc = TITC;
    int tadl = TADL;
    int tccs = TCCS;
    //int tcad = TCAD;

   // nand_i = get_nand_info();
    nand_ctrl = ASAP1826_GetBase_NAND();

    //Pin_assign_dev(LOCATION_NAND2);

    // WP Pin Assign , not need?
   // set_GPIO(1,20);

    // open nand clock here ??????????????
   // reg_set_and_verify((void *)&clk->NANDCLKCTRL[0],0x40000000,REG_TYPE_32 | REG_TYPE_CLR);
   // reg_set_and_verify((void *)&clk->NANDCLKCTRL[0],0x80000000,REG_TYPE_32 | REG_TYPE_CLR);

    // init nand ctrl
  //  outl(0,HW_DIGCTL_NAND_CTRL);

    // default config before read id
    nand_ctrl->REG_NAND_CONTROL = (WORK_MODE_ASYNC<<15) |
                              (PROT_DIS<<14) |
                              (IO_WIDTH_8<<12) |
                              (DATA_SIZE_CUSTOM<<11) |
                              ((0)<<8) |
                              ((0)<<6) |
                              (ECC_DIS<<5) |
                              (INT_DIS<<4) |
                              (SPARE_DIS<<3) |
                              (ADDR1_AUTO_INCR_DIS<<2) |
                              (ADDR0_AUTO_INCR_DIS<<1) |
                              (0) ;

    // init timing registers
    twh = TWH;    twp = TWP;    twhr = TWHR;    trhw = TRHW;

    nand_ctrl->REG_NAND_TIME_SEQ = ( (twhr<<24)  |
                               (trhw<<16)  |
                               (tadl<<8)  |
                               (tccs)     ) ;


    nand_ctrl->REG_NAND_TIMINGS_ASYN = ( (twh<<4) | // after power on ,device is in async mode0, twh >= 50
                                   (twp) );        // twp >= 50 and twhr >= 120

    select_nand_cen(0);
#if 0
    if (nand_i->mode == NAND_MODE_218P4096)
    {
        nand_i->col_cycles = 2;
        nand_i->row_cycles = 3;
        nand_i->addr_cycles = 5;

        nand_i->page_shift = 12;
        nand_i->block_shift = 7; // Really ? so far, we don't find any 1MB / block Nand
        nand_i->block_size = 0x00080000; // 512KB

        nand_i->sector_per_page = (1<<(nand_i->page_shift-9)); // 512 Byte / Sector
        nand_i->page_per_block = (1<<nand_i->block_shift);// ???
        nand_i->page_size = (1<<nand_i->page_shift);
        nand_i->oob_size = 218;
        nand_i->ecc_correctable_bits = 16; // 16 bits BCH -> 26 Bytes ECC data
        nand_i->spare_ecc_offs = nand_i->oob_size -
        nand_i->sector_per_page * NAND_CORRECT_16_BITS_PARITY_BYTES;

        nand_ctrl->REG_NAND_ECC_CTRL = (ECC_WORD_POS_SPARE<<13) |
                                    (15<<8) | //ECC_THRESHOLD
                                    (ECC_CAP_16<<5) ;
        error = 0;
    }
    else{
#endif
        error = NandSearch(nand_i); // original nand
        nand_i->ecc_correctable_bits = 8; // 8 bits BCH -> 13 Bytes ECC data
        nand_i->spare_ecc_offs = nand_i->oob_size -
        	nand_i->sector_per_page * NAND_CORRECT_8_BITS_PARITY_BYTES;

        nand_ctrl->REG_NAND_ECC_CTRL = (ECC_WORD_POS_SPARE<<13) |
                                  (7<<8) | //ECC_THRESHOLD
                                  (ECC_CAP_8<<5) ;
    //}

    nand_ctrl->REG_NAND_ECC_OFFSET = nand_i->spare_ecc_offs + nand_i->page_size;

    // reconfig CONTROL reg after inited nand info
    nand_ctrl->REG_NAND_CONTROL = (WORK_MODE_ASYNC<<15) |
                              (PROT_DIS<<14) |
                              (IO_WIDTH_8<<12) |
                              (DATA_SIZE_CUSTOM<<11) |
                              ( ((nand_i->page_shift - 8)&0x7)  <<8) |
                              ( ((nand_i->block_shift - 5)&0x3) <<6) |
                              (ECC_EN<<5) |
                              (INT_DIS<<4) |
                              (SPARE_EN<<3) |
                              (ADDR1_AUTO_INCR_DIS<<2) |
                              (ADDR0_AUTO_INCR_DIS<<1) |
                              ((nand_i->addr_cycles-4) & 0x1) ;

    nand_i->oob_buf = NULL; // we don't need such buffer

    return error;
}


void convert_nand_page_addr(nand_info * nand_i,int page,
                            int type, char * addr_array)
{
    u32 i,j;

    /* === Col addr === */
    i=0;
    addr_array[i++] = 0;  // 1st col addr

    if ( nand_i->col_cycles > 1 )
    {
        addr_array[i++] = 0;
    }

    /* === Row addr === */
    for ( j=0;i<(nand_i->addr_cycles); )
    { // 1st,2nd,3rd row addr
        addr_array[i++] = ((page>>((j++)*8)) & 0xff);
    }
}

/*
AS3310 Nand Read Page
Using Ecc Check
type 0: raw read data               ret=len
type 1: raw read oob                ret=oob size
type 3: data + ecc (kernel mode)    ret=page size
*/

int AS3310_Nand_Read_Page(nand_info * nand_i,int page,
                          char * buffer,char * oob,int type,int len)
{
    int tmp;
    int n_read;
    int i;
    int ret;
    int * target_ptr;
    volatile unsigned int test_value;
    ASAP1826_NAND_CTRL * nand_ctrl;
    nand_ctrl = ASAP1826_GetBase_NAND();
//    delay(0);
    if ( len > nand_i->page_size )
    {
        // only read raw data can 'len' > nand_i->page_size
        len = nand_i->page_size;
    }
  //  if ((((int)buffer) & 3) || (((int)oob) & 3))
  //  {
  //      report_error(ERROR_CODE_NAND_BUF_NOT_ALIGNED,0);
  //      as_puts("nand: unaligned buf ptr\n");
  //      return -1;
  //  }

    n_read = 0; ret = 0;
    nand_ctrl->REG_NAND_FIFO_INIT = 1;
    nand_ctrl->REG_NAND_CONTROL = ( (EN_STATUS<<23) |
                              (NO_RNB_SEL<<22) |
                              (BIG_BLOCK_EN<<21) |
                              ((nand_i->addr_cycles)<<18)  |
                              (ADDR1_AUTO_INCR_DIS<<17) |
                              (ADDR0_AUTO_INCR_DIS<<16) |
                              (WORK_MODE_ASYNC<<15) |
                              (PROT_DIS<<14) |
                              (LOOKUP_DIS<<13) |
                              (IO_WIDTH_8<<12) |
                              (DATA_SIZE_CUSTOM<<11) |
                              ( ((nand_i->page_shift - 8)&0x7)  <<8) |
                              ( ((nand_i->block_shift - 5)&0x3) <<6) |
                              (ECC_DIS<<5) |
                              (INT_DIS<<4) |
                              (SPARE_DIS<<3) |
                               (nand_i->addr_cycles  )) ;
#if 0
    if ( type == NAND_READ_RAW_DATA )
    {
        n_read = len;
        target_ptr = (int *)buffer;
    }
    else if ( type == NAND_READ_RAW_OOB )
    {
        n_read = min(nand_i->oob_size,len);
        target_ptr = (int *)oob;
    }
    else
    {
#endif
     // with ECC
        n_read = nand_i->page_size;// & 0x1fff;
        target_ptr = (int *)buffer;
        nand_ctrl->REG_NAND_CONTROL |= ((1<<NAND_CTRL_FD_ECC_EN) |(SPARE_EN<<3));  // enable ecc
	nand_ctrl->REG_NAND_CONTROL &= (~(1UL<<NAND_CTRL_FD_CUSTOM_SIZE));
	nand_ctrl->REG_NAND_ECC_CTRL = ( (ECC_WORD_POS_SPARE<<13) |
                      (ECC_THRESHOLD_8<<8) |
                      (ECC_CAP_8<<5) );
	nand_ctrl->REG_NAND_SPARE_SIZE= 0xC;
	nand_ctrl->REG_NAND_ECC_OFFSET=nand_i->page_size+0xc;
 //   }

    ret = n_read;
	//puts("\nlen :"); puth(ret);
    // set address
    nand_ctrl->REG_NAND_DATA_SIZE = n_read;
    convert_nand_page_addr(nand_i,page,type, nand_tmp_addr);
    nand_ctrl->REG_NAND_ADDR0_L = inl(nand_tmp_addr);
    nand_ctrl->REG_NAND_ADDR0_H = inl(nand_tmp_addr+4);
    //start reag page operation
    nand_ctrl->REG_NAND_COMMAND = (READ_PAGE_2<<16) |
                              (READ_PAGE_1<<8) |
                              (0<<7) |
                              (INPUT_SEL_BIU<<6) |
                              (SEQ10) ;
    // wait for controller ready
    while((nand_ctrl->REG_NAND_STATUS) &  (1<<8) ) {/*puts("REG_NAND_STATUS");*/} ;

    // read data
    for(i=0; i<(n_read>>2);  i++) {

	if (oob_flag == 0x00B00001)	
		tmp = nand_ctrl->REG_NAND_FIFO_DATA;
	else
        	*target_ptr++ = nand_ctrl->REG_NAND_FIFO_DATA;
    }

    if(type == NAND_WRITE_PAGE_OS)
    {
        target_ptr = (int *)oob;
        for(i=0; i< 3 ;i++)
        {
        	*target_ptr++ = nand_ctrl->REG_NAND_FIFO_DATA ;
        }
        // check ecc status here
        if (nand_ctrl->REG_NAND_ECC_CTRL & (1<<1)) {
     //       report_error(ERROR_CODE_NAND_ECC_CHECK_FAIL,0);
            return -2;
        }
    }
    return ret;
}

char NCached spare_check_buf[12];

int AS3310_Nand_Read_Page_oob(nand_info * nand_i,int page,char * oob,int len)
{
	int ret = 0;

	oob_flag = 0x00B00001;
	ret = AS3310_Nand_Read_Page(nand_i,page,NULL,oob, NAND_READ_PAGE_OS,nand_i->page_size);
	oob_flag = 0x00B00000;

	return ret;
}


int nand_block_is_bad(nand_info * nand_i,int block)
{
    int ret;
    char bad_byte;

    bad_byte = 0xff;
    /* 1st page */
    AS3310_Nand_Read_Page_oob(nand_i,(block<<(nand_i->block_shift)),spare_check_buf,16);
    //MemDisp_TRL((uchar *)spare_check_buf,16,4); 
    bad_byte &= spare_check_buf[0];

    /* 2nd page */
    AS3310_Nand_Read_Page_oob(nand_i,(block<<(nand_i->block_shift)) + 1,spare_check_buf,16);
    //MemDisp_TRL((uchar *)spare_check_buf,16,4); 
    bad_byte &= spare_check_buf[0];

	/* last page */
	AS3310_Nand_Read_Page_oob(nand_i,((block+1)<<(nand_i->block_shift)) - 1,spare_check_buf,16);
	//MemDisp_TRL((uchar *)spare_check_buf,16,4); 
	bad_byte &= spare_check_buf[0];  

    ret = 0;
    if ( bad_byte != 0xff )
    {
        //report_error(ERROR_CODE_NAND_BAD_BLOCK,block);
        ret = -1;
    }

    return ret;
}





