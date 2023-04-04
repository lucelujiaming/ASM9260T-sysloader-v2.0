/*

Alpha Scale AS3310X NuwaStone
He Yong, AlpScale Software Engineering,


------------------- Version 1.0  ----------------------
Create File
 He Yong 2007-12-26

*/

#ifndef  __DEVICE_H__
#define  __DEVICE_H__

typedef unsigned long lbaint_t;

typedef struct blk_dev {
	char *		name;	/* name */
	int		fs_index;	/* file system type index */

	lbaint_t		lba;	  	/* number of blocks */
	unsigned long	blksz;		/* block size */
	unsigned long	(*block_read)(int dev, unsigned long start,
                    lbaint_t blkcnt, unsigned long *buffer);
    /* Partition info */
    unsigned long part_offset;// = 0;
    int cur_part;// = 1;
	unsigned char	part_type;  	/* partition type */

    /* other info */
	unsigned char	target;		/* target SCSI ID */
	unsigned char	lun;		/* target LUN */
	unsigned char	type;		/* device type */
	unsigned char	removable;	/* removable device */
	int		if_type;	/* type of the interface */
	int	        dev;	  	/* device number */

}block_dev_desc_t;

typedef struct blk_dev_list{
    int   count;    // Number of Devices
    struct blk_dev ** entry;   // 
} blk_devl_t ;

typedef unsigned long d_offs;

typedef struct device{
    char *name;
    int   dev_id;    // Device id
    int  (*probe)(struct device * dev);
    int  (*remove)(struct device * dev );
    int  (*open)(void);
    int  (*close)(struct device * dev);
    int  (*read)(struct device * dev,d_offs ofs,void * buf,int count);
    int  (*write)(struct device * dev,d_offs ofs,const void * buf,int count);
    int  (*ioctl)(struct device * dev,unsigned int cmd,unsigned long arg);
    int  (*suspend)(struct device * dev);
    int  (*resume)(struct device * dev);
    void *  (*mmap)(struct device * dev,int flag,int count);
    int  status;
    void * priv_data;
} device_t ;

typedef struct device * dev_ptr;

typedef struct device_list{
    int   count;    // Number of Devices
    struct device ** entry;   // 
} devl_t ;

extern long __DEV_START;
extern long __DEV_END;
extern long __BLK_DEV_START;
extern long __BLK_DEV_END;

/* Normal Device Interfaces */
#define __add_device(dev) \
	static struct device * init_struct_##dev \
	__attribute__((__section__(".device"))) = &dev

#define __add_device_late(level,dev,id) \
	static struct device * init_struct_##dev##id \
	__attribute__((__section__(".device." level))) = &dev

#define __add_device_lv0(dev)		__add_device_late("0",dev,0)
#define __add_device_lv1(dev)		__add_device_late("1",dev,1)

/* Block Device Interfaces, for mount fs only */
#define __add_blk_device(dev) \
	static struct blk_dev * init_struct_##dev \
	__attribute__((__section__(".blk_dev"))) = &dev
    
void device_init(void);
void normal_device_init(void);
void blk_dev_init(void);
struct device * device_get(char * name, int * index);
struct blk_dev * blk_device_get(char * name, int * index);
inline struct blk_dev_list * get_blk_dev_list(void);

extern struct device_list dev_list;

inline struct device_list * const device_get_list(void);

int do_lsdev(cmd_tbl_t * cmdtp,int argc, char *argv[]);

/*******************dev name already********************
*   "nand"
*   "sound"
*   "lcd control"
*   "monitor"
*
*
*
*
*
*
*
*
*
********************************************************/

#endif //__DEVICE_H__
