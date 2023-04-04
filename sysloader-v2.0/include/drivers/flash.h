/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Flash Header file

------------------- Version 1.0  ----------------------
Create File, Support Nand Flash
 He Yong 2006-11-06

*/
#ifndef __FLASH_H__
#define __FLASH_H__

#define IS_DMA_NAND_COMPLETE() ((inl(HW_APBH_CH4_SEMA+ (0x70*nand_i->chip))&0x00ff0000)==0)
#define NAND_PKG_NUM 5

extern volatile AS3310_DMA_PKG NCached nand_dma_pkg[NAND_PKG_NUM];

void Nand_Read_ID(nand_info * nand_i,ulong* dev_id,int n_byte,uchar cmd);

#define NAND_READ_RAW_DATA  0
#define NAND_READ_RAW_OOB   1
#define NAND_READ_PAGE_BOOT 2
#define NAND_READ_PAGE_OS   3

#define NAND_WRITE_RAW_DATA  0
#define NAND_WRITE_RAW_OOB   1
#define NAND_WRITE_PAGE_BOOT 2
#define NAND_WRITE_PAGE_OS   3

#define MAX_RBB_WAIT 0x200000

void Nand_Reset(void);
int NAND_Erase(nand_info * nand_i, int page);
int nand_send_cmd(nand_info * nand_i,char cmd);
int nand_wait_rbb(nand_info * nand_i);
int nand_send_cmd_and_wait(nand_info * nand_i,char cmd);
int nand_send_addr(nand_info * nand_i,char * addr, int cycles);
int nand_read_buf(nand_info * nand_i,char * buf, int len);
int nand_write_buf(nand_info * nand_i,char * buf, int len);
int nand_block_is_bad(nand_info * nand_i,int block);
void nand_block_markbad(nand_info * nand_i,int block);
uchar nand_read_status(nand_info * nand_i);
int nand_read(nand_info * nand_i,int read_type,char *target_addr,ulong addr,ulong length);
void Nand_Write_AS3310(nand_info * nand_i,uchar addr1,ulong addr,ulong* buffer,ulong length);
int AS3310_Nand_Read_Page(nand_info * nand_i,int page,char * buffer,char * oob,int type,int len);
int AS3310_Nand_Read_Page_oob(nand_info * nand_i,int page,char * oob,int len);
int AS3310_Nand_Write_Page(nand_info * nand_i,int page,char * buffer,char * oob,int type);
int do_nande(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_nandwr(nand_info * nand_i,int write_type,char *src_addr,ulong nand_addr,ulong length);
int do_m2nand_ecc(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_m2nand(cmd_tbl_t *cmdtp,int argc,char* argv[]);

//int Nor_Write_AS3310_single(ulong addr,ushort data);
//int Nor_Write_AS3310(ulong addr,ulong* buffer,ulong length);
//void nor_erase(ulong sta_addr,ulong len);
//int NOR_MBM29LV160TE_Erase(ulong addr);
void select_nand_cen(u8 cen);
int do_nandr(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_nandcheck(cmd_tbl_t *cmdtp,int argc,char* argv[]);
void start_nand_dma_routine(nand_info * nand_i,ulong addr,int pkg_num);


    /* NAND device ioctls */
#define NAND_IOCTL_ECC_NONE         0
#define NAND_IOCTL_ECC_BOOT         1
#define NAND_IOCTL_ECC_OS           2
#define NAND_IOCTL_ERASE_FORCE      4
#define NAND_IOCTL_ERASE_SKIPBAD    5

#define NAND_IOCTL_READ_OOB         10
#define NAND_IOCTL_READ_PAGE_ECCOS  11
#define NAND_IOCTL_WRITE_OOB        12
#define NAND_IOCTL_WRITE_PAGE_ECCOS 13

#define NAND_IOCTL_GET_CHIP_SIZE    100
#define NAND_IOCTL_GET_CHIP_SHIFT    103
#define NAND_IOCTL_GET_PAGE_SHIFT    101
#define NAND_IOCTL_GET_BLOCKPAGE_SHIFT   102


#define NAND_STATUS_ECC_BOOT     (1UL<<1)
#define NAND_STATUS_ECC_OS       (1UL<<2)

int  dev_nand_read(struct device * dev,d_offs ofs,void * buf,int count);
int  dev_nand_write(struct device * dev,d_offs ofs,const void * buf,int count);
int  dev_nand_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);

struct nand_read_ops {
    int page;
    char * data;
    char * oob;
    int ret;
};

struct nand_write_ops {
    int page;
    char * data;
    char * oob;
    int ret;
};

#endif // __FLASH_H__

