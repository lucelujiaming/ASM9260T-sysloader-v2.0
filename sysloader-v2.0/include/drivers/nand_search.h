
/*
Alpha Scale AS3310X BootLoader H-BOOT
He Yong, AlpScale Software Engineering, hoffer@sjtu.org
 
AS3310 Boot Loader Nand table Header file

------------------- Version 2.0  ----------------------
Support Big Capacity(>= 2Gb) Nand Flash by 4th DevID
 He Yong 2007-05-15

------------------- Version 1.0  ----------------------
Create File, Support 26 types of Nand Flash
 He Yong 2006-11-28
 
*/
#ifndef __NAND_TABLE_STRUCT__
#define __NAND_TABLE_STRUCT__
  

#define NAND_READ_RAW_DATA  0
#define NAND_READ_RAW_OOB   1
#define NAND_READ_PAGE_BOOT 2
#define NAND_READ_PAGE_OS   3

#define NAND_WRITE_RAW_DATA  0
#define NAND_WRITE_RAW_OOB   1
#define NAND_WRITE_PAGE_BOOT 2
#define NAND_WRITE_PAGE_OS   3

/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 * @name:	Identify the device type
 * @id:		device ID code
 * @pagesize:	Pagesize in bytes. Either 256 or 512 or 0
 *		If the pagesize is 0, then the real pagesize
 *		and the eraseize are determined from the
 *		extended id bytes in the chip
 * @erasesize:	Size of an erase block in the flash device.
 * @chipsize:	Total chipsize in Mega Bytes
 * @options:	Bitfield to store chip relevant options
 */
struct nand_flash_dev {
	char *name;
	char id;
	unsigned long pagesize;
	unsigned long chipsize;
	unsigned long erasesize;
	int options;
} flash_dev;

#define NAND_BUSWIDTH_16 1
#define LP_OPTIONS 2
#define LP_OPTIONS16 (NAND_BUSWIDTH_16 | LP_OPTIONS)


/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_SAMSUNG	0xec
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SANDISK	0x45
#define NAND_MFR_STMICRO	0x20
#define NAND_MFR_HYNIX		0xad
#define NAND_MFR_MICRON		0x2c
#define NAND_MFR_RENESAS	0x07
#define NAND_MFR_NATIONAL	0x8f
#define NAND_MFR_FUJITSU	0x04

/**
 * struct nand_manufacturers - NAND Flash Manufacturer ID Structure
 * @name:	Manufacturer name
 * @id:		manufacturer ID code of device.
*/
struct nand_manufacturers {
	int id;
	char * name;
};

/* Nand info Struct */
typedef struct nand_info_struct
{
    u8  maker_code;
    u8  dev_id;
    u8  third_id;
    u8  fourth_id;
    u32 extra_3;
    u32 page_shift;
    u32 block_shift;
 
    u32 addr_cycles;
  
    u32 row_cycles; 
    u32 col_cycles;
    u8  * maker_name;
    int page_size;       // in Byte
    int block_size;     // in Byte
    int sector_per_page;
    int page_per_block;
    int oob_size;
    int chip;
    int chip_size;  // MB
    int options;  // x8 or x16
    char * oob_buf; // support up to 8KB page
    
    #define NAND_MODE_16P512    1   // Spare area 16 B / 512 B
    #define NAND_MODE_218P4096  2   // Spare area 218 B / 4096 B
    int mode; // 1 - 16/512;  2 - 218/4096    

    #define NAND_CORRECT_8_BITS_PARITY_BYTES    13
    #define NAND_CORRECT_16_BITS_PARITY_BYTES   26
    int ecc_correctable_bits;

    int spare_ecc_offs;

}/*__attribute__((packed)) */nand_info;

extern nand_info alp_nandinfo;

static inline nand_info * const get_nand_info(){
    return (nand_info * const)&alp_nandinfo;
 }

/*  ===========  NAND FLASH General 4th ID  ============ */
        /*  =  Samsung / ST / Micron / HYNIX  == */

/*
Col Cycle = 2
Row Cycle = 3  default 2Gb+
PageShift = nand_general_id4.PageSize + 10
BlockPageShift = 6 + nand_general_id4.BlockSize - nand_general_id4.PageSize
*/

#define NAND_GENERAL_PAGE_SIZE_1KB  0x0
#define NAND_GENERAL_PAGE_SIZE_2KB  0x1
#define NAND_GENERAL_PAGE_SIZE_4KB  0x2
#define NAND_GENERAL_PAGE_SIZE_8KB  0x3

#define NAND_GENERAL_PAGE_SIZE_64KB     0x0
#define NAND_GENERAL_PAGE_SIZE_128KB    0x1
#define NAND_GENERAL_PAGE_SIZE_256KB    0x2
#define NAND_GENERAL_PAGE_SIZE_512KB    0x3

#define NAND_GENERAL_ORG_X8     0x0
#define NAND_GENERAL_ORG_X16    0x1

typedef union _nand_general_id4{
    struct _nand_general_id4_{
        u8 PageSize             : 2;    // PageSize
        u8 SpareAreaSize        : 1;    // 
        u8 AccessTime1          : 1;    // 
        u8 BlockSize            : 2;    // BlockSize
        u8 Orgniz               : 1;    // x8 or x16
        u8 AccessTime2          : 1;    //
    } __attribute__((packed)) field ;
    u8 value;
}nand_general_id4;


int NAND_init(nand_info * nand_i);
int NandSearch(nand_info * nand_i);
int nand_probe(struct device * dev);

unsigned int get_shift(unsigned int val);
void Nand_init_pins(void);

#endif // __NAND_TABLE_STRUCT__

