/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader BCB Header file

------------------- Version 1.0  ----------------------
Create File, Support 5 types of boot up , 26 types of Nand Flash
 He Yong 2006-11-29

*/

#include <kernel/console.h>

/*  =========== BCB Sructure  ===========*/
#ifndef __BCB_STRUCT__
#define __BCB_STRUCT__

#define BCB_MAGIC1     0x53504C41// ALPS
#define BCB_MAGIC2     0x32424342// BCB
#define BCB_VERSION_MAJOR    0x0001
#define BCB_VERSION_MINOR    0x0000
#define BCB_VERSION_SUB      0x0000


#define BOOT_INFO_MAGIC     0x544F4F42// BOOT
#define HEADER_SIZE     0x20


typedef struct Version_struct{
            u16 Major;
            u16 Minor;
            u16 Sub;
        }__attribute__((packed)) Version;

typedef union _nand_tag{
    struct _nand_tag_{
        u32 Auto_search         : 1;    // Searching in nand table, clear to use settings above
        u32 ECC_Enable          : 1;    // Enable ECC when relocating code
        u32 nand_cen            : 2;    // cen number of nand chip for boot
        u32 page_shift          : 4;    // nand page shift
        u32 col_cycle           : 4;    // nand col cycles, zero for some mlc nand
        u32 page_cycle          : 4;    // nand page cycles
        u32 block_page_shift    : 4;    // Block page shift.
        u32 starting_block      : 12;    // 12b boot block no.
    } __attribute__((packed)) field ;
    u32 value;
}nand_tag;


typedef union _config_tag{
    struct _config_tag_{
        u32 usb_external_phy        : 1;    // set:external phy
        u32 SBZ2                    : 1;    // should be zero
        u32 nand_table_dev          : 2;    // 1 = I2C  , 2 = NorFlash , 3 = ROM
        u32 sdram_latency           : 1;    // set: 0x2 , clear 0x3
        u32 SBZ1                    : 1;    // should be zero
        u32 sdram_cen               : 2;    // cen number of sdram chip, default 10b
        u32 sdram_col               : 4;    // nand page shift
        u32 sdram_row               : 4;    // nand col cycles, zero for some mlc nand
        u32 source_device           : 4;    // 0 == Nand , 1 = I2C  , 2 = NorFlash , 3 = ROM , 4 = USB , 5 = UART
        u32 nand_table_offset       : 12;   // 12b where to locate the nand_table
    } __attribute__((packed)) field ;
    u32 value;
}config_tag;


 /*  Length 0x20*/
typedef struct boot_info_struct {
    u32         Magic;                  //  0x544F4F42 BOOT
    u32         src_addr;               // the address we got the code
    u32         dest_addr;              // dest address , where the app will run
    u32         n_bytes;                // code length to recloate
    u32         starting_offset;        // entry offset
    nand_tag    nand_t;                 // nand tag
    config_tag   config_t;              // config tag
    u32          header_checksum;       // Header CheckSum
        }__attribute__((packed)) boot_header;



#endif // __BCB_STRUCT__
