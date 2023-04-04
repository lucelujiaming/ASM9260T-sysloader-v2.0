#ifndef __NFTL_CORE_H__
#define __NFTL_CORE_H__

#if CONFIG_AFTL_DEBUG
#define aftl_dbg(msg...)    printf(msg)
#else
#define aftl_dbg(msg...)    {}
#endif

typedef struct _nftl_oob { // byte[25:2]--24 bytes
	u8 ecc1;
    u8 ecc2;
    u8 ecc3;
	u8 free_page_pos;
	u8 free_page_pos1;
	u8 reserved;
	u8 Status;
	u8 Status1;    //  8byte 
	u16 logblocknum;
	u16 revblocknum;
	u16 logblocknum1;
	u16 revblocknum1;
    u32 WearInfo;
	u16 EraseMark;
	u16 EraseMark1;    //  16byte 
}__attribute__((packed)) nftl_oob;

// typedef struct _nftl_oob_full { // byte[25:2]--24 bytes
// 	char bad[2];
// 	u8 ecc1;
//     u8 ecc2;
//     u8 ecc3;
// 	u8 free_page_pos;
// 	u8 free_page_pos1;
// 	u8 reserved;
// 	u8 Status;
// 	u8 Status1;    //  8byte + 2
// 	u16 logblocknum;
// 	u16 revblocknum;
// 	u16 logblocknum1;
// 	u16 revblocknum1;
//     u32 WearInfo;
// 	u16 EraseMark;
// 	u16 EraseMark1;    //  16byte + 2
// 	char ecc[36];
// }__attribute__((packed)) nftl_oob_full;

#define PAGE_SECTOR_0  (1<<0)
#define PAGE_SECTOR_1  (1<<1)
#define PAGE_SECTOR_2  (1<<2)
#define PAGE_SECTOR_3  (1<<3)

    /* NAND device ioctls */
/*#define NAND_IOCTL_ECC_NONE         0
#define NAND_IOCTL_ECC_BOOT         1
#define NAND_IOCTL_ECC_OS           2
#define NAND_IOCTL_ERASE_FORCE      4
#define NAND_IOCTL_ERASE_SKIPBAD    5

#define NAND_IOCTL_READ_OOB         10
#define NAND_IOCTL_READ_PAGE_ECCOS  11
#define NAND_IOCTL_WRITE_OOB        12
#define NAND_IOCTL_WRITE_PAGE_ECCOS 13

#define NAND_IOCTL_GET_CHIP_SIZE    100
#define NAND_IOCTL_GET_PAGE_SHIFT    101
#define NAND_IOCTL_GET_BLOCKPAGE_SHIFT   102

#define NAND_STATUS_ECC_BOOT     (1UL<<1)
#define NAND_STATUS_ECC_OS       (1UL<<2)*/

int do_aftlws(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_aftlrs(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_aftlformat(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_aftlinfo(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_aftloff(cmd_tbl_t *cmdtp,int argc,char* argv[]);

/*
void MediaHeader_dump(nftl_mediaheader * header);
void chains_dump(nftl_info * info);
void reptable_dump(nftl_info * info);
void revtable_dump(nftl_info * info);
void EUNtable_dump(nftl_info * info);
void ffptable_dump(nftl_info * info);
int is_badblock(device_t * nandbase_t,nftl_info * tlinfo,int block);
int mark_badblock(device_t * nandbase_t,nftl_info * tlinfo,int block);
u16 find_read_EUN(nftl_info *tlinfo, u16 logblock, char page);
void copy_back(device_t * nandbase_t,nftl_info * tlinfo,u16 thisVUC,u16 preblock,u16 srcblock,u16 tarblock,int beginpage,int endpage);
void fold_page_data(device_t * nandbase_t,nftl_info * tlinfo,u16 thisVUC,u16 secondlastEUN,u16 lastEUN,char ffpage,char newstartpage);
u16 findfreeblock(nftl_info * tlinfo);
u16 makefreeblock(device_t * nandbase_t,nftl_info * tlinfo);
u16 find_write_EUN(device_t * nandbase_t,nftl_info *tlinfo, u16 logblock, char page, u16 * LastBlockNum);
int writeback_lastbuf(device_t * nandbase_t,nftl_info * tlinfo);
int read_sector(nftl_info * tlinfo, u32 sector, char * buffer);
int write_sector(nftl_info * tlinfo, u32 sector, char * buffer);
*/
#endif // __NFTL_CORE_H__
       
       
