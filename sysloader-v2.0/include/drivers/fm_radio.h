/*
Nuwa-Stone FM Radio

==============================

He Yong 2008-02-26

*/

#ifndef __FM_RADIO_H__
#define __FM_RADIO_H__

#define FM_I2C_ADDR 0xc0
#define FM_AUTO_TUNE_STEP 1
#define FM_TUNE_STEP 6

struct fmradio{
    char fm_conf[8];
    char fm_confro[8];
    int lba_offs;  // addr of LBAs (512 bytes)
};

#define FM_START        0
#define FM_STOP         1
#define FM_SET_MHZ      10
#define FM_TUNE_UP      11
#define FM_TUNE_DOWN    12
#define FM_SEARCH_UP    13
#define FM_SEARCH_DOWN  14


int i2c_read(char * addr, char * buffer, int n_read);
int i2c_write(char * addr,const  char *buffer,int n_write);
void initial_i2c(void);
int fm_stop(struct device * dev);
int init_loopback_audio(void);
/* read FM module configuation */
int fmradio_read(struct device * dev,d_offs ofs,void * buf,int count);
/* set FM module configuation */
int fmradio_write(struct device * dev,d_offs ofs,const void * buf,int count);
void fmradio_set_mhz(int tune_MHZ);
void fmradio_tune_up(struct device * dev);
void fmradio_tune_down(struct device * dev);
void fmradio_search_up(struct device * dev);
void fmradio_search_down(struct device * dev);
int fmradio_probe(struct device * dev);
int fm_start(struct device * dev);

int fmradio_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);

int do_cmd_fmradio(cmd_tbl_t * cmdtp,int argc, char *argv[]);


#endif// __FM_RADIO_H__

