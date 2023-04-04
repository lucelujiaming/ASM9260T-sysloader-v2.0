/*

Alpha Scale AS3310X NuwaStone
He Yong, AlpScale Software Engineering,


------------------- Version 1.0  ----------------------
Create File
 He Yong 2007-12-26

*/

#include <common.h>

struct device_list dev_list;
//struct blk_dev_list blk_dev_l;

//inline struct blk_dev_list * get_blk_dev_list(){
//    return &blk_dev_l;
//}

inline struct device_list * const device_get_list(void){
	return (struct device_list * const)&dev_list;
}

void normal_device_init(){

    struct device * dev;
    int dev_i;

    dev_i = 0;
    dev_list.count = 0;
    dev_list.entry = (struct device **)&__DEV_START;

    for ( ; (ulong)&dev_list.entry[dev_i] < (ulong)&__DEV_END ; dev_i++) {
        puts("<Init Device>: ");
        puts(dev_list.entry[dev_i]->name);puts("\t Probing...\n");
        dev_list.entry[dev_i]->probe(dev_list.entry[dev_i]);
        dev_list.count++;
    }
}

struct device * device_get(char * name, int * index){

    struct device * dev;
    int dev_i;

    for (dev_i = 0 ; dev_i < dev_list.count ; dev_i++) {
        if (!strcmp( name, dev_list.entry[dev_i]->name)) {
            * index = dev_i;
            return dev_list.entry[dev_i];
        }
    }

    return NULL;
}

//void blk_dev_init(){
//    int dev_i;
//
//    dev_i = 0;
//    blk_dev_l.count = 0;
//    blk_dev_l.entry = (struct blk_dev **)&__BLK_DEV_START;
//
//    puts("<Init Block Device>: ");
//    for ( ; (ulong)&blk_dev_l.entry[dev_i] < (ulong)&__BLK_DEV_END ; dev_i++) {
//        puts(blk_dev_l.entry[dev_i]->name);putc('\t');
//        blk_dev_l.entry[dev_i]->fs_index = -1; // initial as no fs mounted
//        blk_dev_l.count++;
//    }
//    putc('\n');
//}

//
//struct blk_dev * blk_device_get(char * name, int * index){
//
//    struct blk_dev * dev;
//    int dev_i;
//
//    for (dev_i = 0 ; dev_i < blk_dev_l.count ; dev_i++) {
//        if (!strcmp( name, blk_dev_l.entry[dev_i]->name)) {
//            * index = dev_i;
//            return blk_dev_l.entry[dev_i];
//        }
//    }
//    return NULL;
//}

void device_init(){
    normal_device_init();
    //blk_dev_init();
}


#if CONFIG_DEV_TEST

int do_lsdev(cmd_tbl_t * cmdtp,int argc, char *argv[]){

    int i;
    struct device * m_dev;
    printf("Devices:\n");
    for (i = 0; i < dev_list.count; i++) {
        printf("/dev/%s\t\t(%s)\n",dev_list.entry[i]->name,dev_list.entry[i]->name);
    }
}


int do_deviotst(cmd_tbl_t * cmdtp,int argc, char *argv[]){

    int i,index,src_blk,dest_blk,count,address;
    u32 sector = 0;
    char buffer[512];
    char buffer2[512];
    char * tarbuf;
    struct device * dev, * dev2;
    char * target_buf;

    if (argc < 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

    dev = device_get(argv[2], &index);
    if (dev == NULL) {
        printf("no such device [%s]\n",argv[2]);
        return -1;
    }

    src_blk = TextToLong_TRL(argv[3]);

    if (argv[1][0] == 'r') {
        /* read */
        if (argc == 6) {
            count = TextToLong_TRL(argv[4]);
            target_buf = (char *)TextToLong_TRL(argv[5]);
        }
        else{
            count = 1;
            target_buf = buffer;
        }
        dev->read(dev,src_blk,target_buf,count);
        if (target_buf == buffer) {
            dbg_hexdump(target_buf,512);
        }
    }
    else if (argv[1][0] == 'w') {
        /* write */
        if (argc == 6) {
            count = TextToLong_TRL(argv[4]);
            target_buf = (char *)TextToLong_TRL(argv[5]);
            dev->write(dev,src_blk,target_buf,count);
        }
    }
    else if (argv[1][0] == 'c') {
        /* copy */
        if (argc == 7) {
            dev2 = device_get(argv[4], &index);
            if (dev2 == NULL) {
                printf("no such device [%s]\n",argv[4]);
                return -1;
            }
            dest_blk = TextToLong_TRL(argv[5]);
            count = TextToLong_TRL(argv[6]);
            printf("Copy from device [%s] block (%d) to device [%s] block (%d), <%d> blocks\n",
                   dev->name, src_blk ,dev2->name, dest_blk , count);
            for (i = 0; i < count ; i++) {
                dev->read(dev,src_blk,buffer,1);
                dev2->write(dev2,dest_blk,buffer,1);
                src_blk++ ; dest_blk++;
            }
        }
    }
    else if (argv[1][0] == 'd') {
        /* diff */
        if (argc == 7) {
            dev2 = device_get(argv[4], &index);
            if (dev2 == NULL) {
                printf("no such device [%s]\n",argv[4]);
                return -1;
            }
            dest_blk = TextToLong_TRL(argv[5]);
            count = TextToLong_TRL(argv[6]);
            for (i = 0; i < count ; i++) {
                dev->read(dev,src_blk,buffer,1);
                dev2->read(dev2,dest_blk,buffer2,1);
                if(memcmp(buffer,buffer2,512)){
                    printf ("Diff at src block %d\nData:\n",src_blk);
                    dbg_hexdump(buffer,512);
                    printf ("dest block %d\nData:\n",dest_blk);
                    dbg_hexdump(buffer2,512);
                    return -1;
                }
                src_blk++ ; dest_blk++;
            }
            printf ("The Same.\n");
        }
    }

    return 0;
}

BOOT_CMD(devtst,do_deviotst,
    "#devtst ops [options]\n"
    "#devtst cp src_dev blk_n dest_dev blk_n count\n"
    "#devtst r src_dev blk_n [count addr]\n"
    "#devtst w src_dev blk_n count addr\n"
    "#devtst diff src_dev blk_n dest_dev blk_n count\n"
    ,"Test Devices IO ops"
    );

BOOT_CMD(lsdev,do_lsdev,
    "#lsdev\n"
    "List All Devices\n"
    , "List All Devices");

#endif// CONFIG_DEV_TEST

