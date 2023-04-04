#ifndef __NFTL_MOUNT_H__
#define __NFTL_MOUNT_H__

#define BLOCK_FREE       0xffff // free block  
#define BLOCK_RESERVED     0xfffe // bios block or bad block 
#define BLOCK_TAIL         0xfffd // last block of a chain 
#define BLOCK_HEAD         0xfffc // head block of a chain 
#define BLOCK_NOMARK         0xfffb // non explored block, only used during mounting 

#define PAGE_FREE 0xff
#define PAGE_USED 0x55
#define PAGE_IGNORE 0x11
#define PAGE_DELETED 0x00

#define CONFIG_AFTL_FOLD_THRESHOLD 0x20
#define CONFIG_AFTL_SPARE_PRECENTAGE 5

#define AFTL_TIME_OUT 100  // time out jiffies ( 1 sec )

typedef struct _nftl_media_header {
	char DataOrgID[6];
	u16 nftlblock_nums; // used for nftl
	u16 first_nftlblock; //used for nftl
    u16 appblock_nums;   //used for up application
	//u64 FormattedSize;
	//unsigned char UnitSizeFactor;
} nftl_mediaheader;

typedef struct _nftl_info{
    char read_buf[2048];  // page read buff 
    char write_buf[2048];  // page write buff 
    char move_buf[2048];  // page move buff 
    char oob_buf[64];  // oob buff 
    //nand_info * nbase_info;
    nftl_mediaheader nftl_header;
    //u16 lastEUN;                  // should be suppressed 
    //u16 numfreeEUNs;
    //u16 LastFreeEUN; 		// To speed up finding a free EUN 
    u16 *EUNtable; 		// [numvunits]: First EUN for each virtual unit  
    u16 *reptable; 		// [numEUNs]: ReplUnitNumber for each 
    u16 *revtable; 		// [numEUNs]: ReverseUnitNumber for each 
    char *ffptable; 		// [numEUNs]: first free page in each EUN 
    unsigned int tblock_nums;		// number of physical blocks used in nftl
 //   unsigned int pblock_nums;       // number of all physical blocks in nand
    unsigned int lblock_nums;       // number of logical blocks used in nftl 
    unsigned int boot_block_nums;   // number of blocks used by the bios
    unsigned int freeblock_nums;    //number of free lblocks

    int page_shift; // page-byte shift
    int block_shift;  // block-page shift
    int chip_shift; // chip-block shift
    int chip_size;  // MB
    int page_per_block;         // physical page in  physical block , 64 or 128

    // For Cached Read/Write Support 
    int read_buf_status;  // 1:read_cached,  0:none_cached
    int write_buf_status; // 1:sector-0, 10:sector-1, 100:sector-2, 1000:sector-3,0:none_cached
    u16 last_read_VUC;
    u16 last_write_VUC; // Last Visited VUC (Write Only)   
    char last_read_page;
    char last_write_page; // Last Visited Page (Write Only)   
    u16 last_write_EUN;   // Last target physical Block 
    u16 last_pre_block;   // Last previous physical Block 
    
    u16 LastFreeEUN;
    unsigned long write_jif;

}nftl_info;

#define FORMAT_THIS_CHAIN           1
#define ONE_LOGBLOCK_IN_TWO_CHAINS  2
#define ONE_BLOCK_IN_TWO_CHAINS     3

extern nftl_info ALIGN32 alp_nftlinfo;

static inline nftl_info * const get_nftl_info(){
    return (nftl_info * const)&alp_nftlinfo;
 }

    /* AFTL device ioctls */
#define NFTL_IOCTL_FORMAT           0
#define NFTL_IOCTL_OFF              1
#define NFTL_IOCTL_GET_SECTOR_NUM   2
#define NFTL_IOCTL_GET_SECTOR_SHIFT  3

unsigned int get_chip_shift(unsigned int val);
void aftl_format(nftl_info * tlinfo);
int find_media_header(nftl_info * info,device_t * nandbase_t);
int get_chain_length(nftl_info *nftlinfo, unsigned int first_block);
int format_block(device_t * nandbase_t, nftl_info *info, int block);
void format_chain(device_t * nandbase_t, nftl_info *info, unsigned int first_block);
void nftl_read_oob(device_t * nandbase_t,nftl_info * info,int page,char * buffer);
int nftl_write(device_t * nandbase_t,nftl_info * info,int page,char * buffer, char * oob);
int nftl_read(device_t * nandbase_t,nftl_info * info,int page,char * buffer);
u16 fold_chain(device_t * nandbase_t,nftl_info * tlinfo,u16 logblock);
int nftl_init(nftl_info * info);
int nftl_probe(struct device * dev);
int dev_read_sector(struct device * dev,d_offs ofs,void * buf,int count);
int dev_write_sector(struct device * dev,d_offs ofs,const void * buf,int count);
unsigned long  aftl_block_read(int dev, unsigned long start,
		lbaint_t blkcnt, unsigned long *buffer);
void aftl_write_back(void);

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
u16 findfreeblock(nftl_info * tlinfo, int in_folding);
u16 makefreeblock(device_t * nandbase_t,nftl_info * tlinfo,u16 * foldVUC);
u16 find_write_EUN(device_t * nandbase_t,nftl_info *tlinfo, u16 logblock, char page, u16 * LastBlockNum);
int writeback_lastbuf(device_t * nandbase_t,nftl_info * tlinfo);
int read_sector(nftl_info * tlinfo, u32 sector, char * buffer);
int write_sector(nftl_info * tlinfo, u32 sector, const char * buffer);
int nftl_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);

extern struct blk_dev blk_dev_aftl;
extern int write_lock; // 0:unlocked; 1:locked
extern int read_lock;  // 0:unlocked; 1:locked

#endif // __NFTL_MOUNT_H__


