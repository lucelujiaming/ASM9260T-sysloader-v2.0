/*
Nuwa-Stone Memory Device

==============================

He Yong 2008-01-06

*/


#include <common.h>
#include <drivers/devmem.h>

#define NO_MALLOC 1 /* we do not have malloc in sram test mode */

#define DEV_MEM_SECTOR_SHIFT 9
#define DEV_MEM_SECTOR_SIZE (1<<DEV_MEM_SECTOR_SHIFT)

struct devmem dev_m;
//static char SRAM ALIGN32 mem_lba_buffer[DEV_MEM_SECTOR_SIZE];

#if NO_MALLOC
    #if CONFIG_DEVMEM_IN_SDRAM
    #else
    static char dev_memory_for_sram[(CONFIG_DEVMEM_SIZE_KB<<10)];
    #endif
#endif// NO_MALLOC

int devmem_probe(struct device * dev){
    //dev_m.mem_lba_buf = mem_lba_buffer;
    dev_m.lba_size = ((CONFIG_DEVMEM_SIZE_KB<<10)>>DEV_MEM_SECTOR_SHIFT);
    dev_m.dev_busy = 0;

#if NO_MALLOC
    #if CONFIG_DEVMEM_IN_SDRAM
    dev_m.mem_buf = (char *)TextToLong_TRL(CONFIG_DEVMEM_SDRAM_BASE);
    #else
    dev_m.mem_buf = dev_memory_for_sram;
    #endif
#else
    if(!(dev_m.mem_buf = c_malloc(dev_m.lba_size<<DEV_MEM_SECTOR_SHIFT))){
       puts("mem_buf memory alloc error\n");
       return -ENOMEM;
    }
#endif// NO_MALLOC
    printf("Devmem inited at 0x%08x size: %d KB\n",dev_m.mem_buf ,CONFIG_DEVMEM_SIZE_KB);

    return 0;
}

int devmem_remove(struct device * dev){

#if NO_MALLOC
#else
    asfree(dev_m.mem_buf);
#endif// NO_MALLOC
    printf("Devmem Removed\n");

    return 0;
}


/* 
here 'ofs' and 'count' is Based on LBA
*/
int devmem_read(struct device * dev,d_offs ofs,void * buf,int count){

    if ((ofs >= dev_m.lba_size)||(dev_m.dev_busy)) return -1;
    printf("Devmem: Read Block %d - %d\n",ofs,ofs+count-1);
    memcpy(buf,dev_m.mem_buf + DEV_MEM_SECTOR_SIZE * ofs, DEV_MEM_SECTOR_SIZE*count);
    return count;
}

/* 
here 'ofs' and 'count' is Based on LBA
*/
int devmem_write(struct device * dev,d_offs ofs,const void * buf,int count){

    if ((ofs >= dev_m.lba_size)||(dev_m.dev_busy)) return -1;
    printf("Devmem: Write Block %d - %d\n",ofs,ofs+count-1);
    memcpy(dev_m.mem_buf + DEV_MEM_SECTOR_SIZE * ofs, buf, DEV_MEM_SECTOR_SIZE*count);
    return 0;
}

// 
// /*
// here 'count' is LBA number
// */
// void * devmem_mmap(struct device * dev,int flag,int count){
// 
//     unsigned long ret;
// 
//     if ((count >= dev_m.lba_size)||(dev_m.dev_busy)) return NULL;
// 
//     dev_m.lba_offs = count;
//     dev_m.dev_busy = 1;
// 
//     return dev_m.mem_lba_buf;
// }
// 
// int devmem_map_write_finish(){
// 
//     memcpy(dev_m.mem_buf + DEV_MEM_SECTOR_SIZE*dev_m.lba_offs, dev_m.mem_lba_buf, DEV_MEM_SECTOR_SIZE);
//     dev_m.dev_busy = 0;
// 
//     return 0;
// }

int devmem_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    switch (cmd) {
 //   case DEV_MEM_WRITE_BUFFER_OK:
 //       devmem_map_write_finish();
 //       return 0;
    case DEV_MEM_GETSIZE:        
        return dev_m.lba_size;
    case DEV_MEM_GET_SECTOR_SHIFT:        
        return DEV_MEM_SECTOR_SHIFT;
    }
    return -1;
}


struct device dev_mem = {
    .name       = "devmem",
    .dev_id     = 0,
    .probe      = devmem_probe,
    .remove     = devmem_remove,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = devmem_read,  
    .write      = devmem_write, 
    .ioctl      = devmem_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
  //  .mmap       = devmem_mmap,
    .status     = 0, 
    .priv_data  = &dev_m, 
};

__add_device(dev_mem);

/* =============== Block Device Interface =============== */

unsigned long  devmem_block_read(int dev, unsigned long start,
		lbaint_t blkcnt, unsigned long *buffer){

    return devmem_read(&dev_mem,start,buffer,blkcnt);
}

struct blk_dev blk_dev_mem = {
    .name       = "ram",
    .block_read = devmem_block_read,  
    .lba        = ((CONFIG_DEVMEM_SIZE_KB<<10)>>DEV_MEM_SECTOR_SHIFT),
    .blksz      = (1<<DEV_MEM_SECTOR_SHIFT),

};

__add_blk_device(blk_dev_mem);


/* =============== CMD =============== */
#ifdef CONFIG_DEVMEM_DEBUG_CMD

int do_cmd_devmem(cmd_tbl_t * cmdtp,int argc, char *argv[]){

    struct device * m_dev;
    struct devmem * dev_i;
    int lba_ofs, f_addrs;
    char * tmp_buf[DEV_MEM_SECTOR_SIZE];
    void * buf;
    int dev_index;

    m_dev = device_get("devmem",&dev_index);
    dev_i = (struct devmem *)m_dev->priv_data;

	if(argc < 2){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

    if (argv[1][0]=='r') {
        lba_ofs = TextToLong_TRL(argv[2]);
        if (lba_ofs >= dev_i->lba_size ) {
            printf("Request LBA %d exceeded Device size %d\n",lba_ofs,dev_i->lba_size );
            return 0;
        }
        m_dev->read(m_dev,lba_ofs,tmp_buf,1);
        dbg_hexdump(tmp_buf,DEV_MEM_SECTOR_SIZE);

    }else  if(argv[1][0]=='w'){
        lba_ofs = TextToLong_TRL(argv[2]);
        f_addrs = TextToLong_TRL(argv[3]);
        if (lba_ofs >= dev_i->lba_size ) {
            printf("Request LBA %d exceeded Device size %d\n",lba_ofs,dev_i->lba_size );
            return 0;
        }
        m_dev->write(m_dev,lba_ofs,(void *)f_addrs,1);

    }        
    else if(argv[1][0]=='m') {
        lba_ofs = TextToLong_TRL(argv[2]);
        if (lba_ofs >= dev_i->lba_size ) {
            printf("Request LBA %d exceeded Device size %d\n",lba_ofs,dev_i->lba_size );
            return 0;
        }
        buf = m_dev->mmap(m_dev,0,lba_ofs);
        printf("mmap returned 0x%08x\n",buf);
    }
    else if(argv[1][0]=='f') {
        m_dev->ioctl(m_dev,DEV_MEM_WRITE_BUFFER_OK,0);
    }

return 0;
}

BOOT_CMD(devmem,do_cmd_devmem,
    "#devmem [options]\n"
    "r LBA_num \n"
    "w LBA_num data_addr\n"
    "m (mmap)\n"
    "f (finish write)\n"
    , "Test Memory Device");

#endif// CONFIG_DEVMEM_DEBUG_CMD


