/*
Nuwa-Stone Memory Device

==============================

He Yong 2008-01-06

*/

#ifndef __DEV_MEM_H__
#define __DEV_MEM_H__

#define DEV_MEM_WRITE_BUFFER_OK 1
#define DEV_MEM_GETSIZE         2
#define DEV_MEM_GET_SECTOR_SHIFT 3

typedef struct devmem{
    char * mem_lba_buf;
    char * mem_buf;
    int dev_busy;
    int lba_size;  // size of LBAs (512 bytes)
    int lba_offs;  // addr of LBAs (512 bytes)
}DEV_MEM;

int devmem_probe(struct device * dev);
int devmem_remove(struct device * dev);
int devmem_read(struct device * dev,d_offs ofs,void * buf,int count);
int devmem_write(struct device * dev,d_offs ofs,const void * buf,int count);
void * devmem_mmap(struct device * dev,int flag,int count);
int devmem_map_write_finish(void);
int devmem_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);

int do_cmd_devmem(cmd_tbl_t * cmdtp,int argc, char *argv[]);

#endif// __DEV_MEM_H__

