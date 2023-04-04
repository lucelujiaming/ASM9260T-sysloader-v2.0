/*
Nuwa-Stone FM Radio

==============================

He Yong 2008-02-26

*/

#include <common.h>
#include <drivers/sound.h>
#include <drivers/fm_radio.h>

struct fmradio NCached fm_info;

/* ================  I2C interface ================ */

#define IS_DMAI2C_COMPLETE() ((inl(HW_APBX_CH3_SEMA)&0x00ff0000)==0)

AS3310_DMA_PKG NCached i2cr_dma_pkg[3];
AS3310_DMA_PKG NCached i2cw_dma_pkg[3];
char NCached i2c_addr;

int i2c_read(char * addr, char * buffer, int n_read){
    int  retry;

    retry = 0x100000;
    i2cr_dma_pkg[0].NEXT_PKG =  (ulong) ( &( i2cr_dma_pkg[1].NEXT_PKG ));
    i2cr_dma_pkg[1].NEXT_PKG =  (ulong) ( &( i2cr_dma_pkg[2].NEXT_PKG ));
    i2cr_dma_pkg[2].NEXT_PKG =  0;
                 
    i2cr_dma_pkg[0].CTRL = 0x000110c6;
    i2cr_dma_pkg[0].BUFFER = (ulong)addr;
    i2cr_dma_pkg[0].CMD0 = 0x002b0001;
              
    i2cr_dma_pkg[1].CTRL = 0x000010c1 + (n_read<<16);
    i2cr_dma_pkg[1].BUFFER = (ulong)buffer;
    i2cr_dma_pkg[1].CMD0 = 0x02120000 + n_read;

	outl(0x00000000,DMAX_CTRL0_ADDR);
	outl(0x00000002,DMAX_CH3_SEMA);
	outl((ulong) ( &( i2cr_dma_pkg[0].NEXT_PKG )),DMAX_CH3_NXTCMDAR);
		
    while(IS_DMAI2C_COMPLETE()==0){
        if (retry-- < 0) {
            puts("I2C Read time out\n");
            return -1;
        }
    }
    return n_read;
}

int i2c_write(char * addr,const  char *buffer,int n_write){
    int  retry;

    retry = 0x100000;

    i2cw_dma_pkg[0].NEXT_PKG =  (ulong) ( &( i2cw_dma_pkg[1].NEXT_PKG ));
                      
    i2cw_dma_pkg[0].CTRL = 0x000210c6;
    i2cw_dma_pkg[0].BUFFER = (ulong)addr;
    i2cw_dma_pkg[0].CMD0 = 0x002b0001;
                      
    i2cw_dma_pkg[1].CTRL = 0x000010c2 + (n_write<<16);
    i2cw_dma_pkg[1].BUFFER = (ulong)buffer;
    i2cw_dma_pkg[1].CMD0 = 0x00130000 + n_write;

	outl(0x00000000,DMAX_CTRL0_ADDR);
	outl(0x2,DMAX_CH3_SEMA);
	outl((ulong) ( &( i2cw_dma_pkg[0].NEXT_PKG )),DMAX_CH3_NXTCMDAR);

    while(IS_DMAI2C_COMPLETE()==0){
        if (retry-- < 0) {
            puts("I2C Write time out\n");
            return -1;
        }
    }
    return n_write;
}

void initial_i2c(){
	outl(0xffffffc3,I2C_PIN);
    // I2C clear clock gate
	outl(0x80000000,I2C_CTRL0_ADDR +4);
	outl(0x80000000,HW_APBX_CTRL0_SET);

	outl(0x00000000,I2C_CTRL0_ADDR);
    outl(0x40000000,HW_APBX_CTRL0_CLR);//clear the clk gate
    outl(0x80000000,HW_APBX_CTRL0_CLR);//clear the sft reset  
}

int fm_stop(struct device * dev){

    struct fmradio * f_info;
    int TuneN,value;

    f_info = (struct fmradio *) dev->priv_data;

    outl(0x00001f1f,AUDIOOUT_HPVOL);    // set hp vol to lowest
    outl(0x00010000,AUDIOOUT_HPVOL + 4);    // set hp mute
    outl(0x00000001,AUDIOOUT_CTRL+ 8);     //clear RUN bit
    outl(0x00000010,AUDIOOUT_CTRL+ 8);     //clear LoopBack bit
    outl(0x00000001,AUDIOIN_CTRL+ 8);     //clear RUN bit

    dev->read(dev,0,f_info->fm_confro,5);

    f_info->fm_conf[0] = f_info->fm_confro[0] | 0x80; // Mute
    f_info->fm_conf[3] = f_info->fm_conf[3] | 0x40; // Stand by

    dev->write(dev,0,f_info->fm_conf,5); 

    printf("FM Radio Stopped\n");
    return 0;
}

int init_loopback_audio() //(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
    printf("FM Radio Start Loopback...\n");

    outl(0x80000000,AUDIOIN_CTRL+4);
    outl(0x80000000,AUDIOOUT_CTRL+4);

    outl(0xc0000000,AUDIOOUT_CTRL+8);     //clkgate out & loopback & 16bit PCM
    outl(0xc0000000,AUDIOIN_CTRL+8);      //clkgate in & no loopback & 16bit PCM
    outl(0x00000080,AUDIOIN_CTRL+8);

    outl(0xc0000000,AUDIOIN_ANACLKCTRL+8); //ana clkgate
    outl(0xc0000000,AUDIOOUT_ANACLKCTRL+8);//ana clkgate

  //  #ifdef HEAD
    outl(0x00000010,AUDIOOUT_PWRDN);    //pwdn ref
   // #endif
   // #ifdef SPK
   // outl(0x01001000,AUDIOOUT_PWRDN+8);
   // #endif
    outl(0x00003770,REFCTRL);           //ref ctrl

    outl(0x00000020,AUDIOIN_CTRL+4);      //no loopback & 16bit PCM
    outl(0x00000040,AUDIOOUT_CTRL+4);     //16bit PCM
#if CONFIG_FM_USE_AD_DA_LOOP
    outl(0x00000010,AUDIOOUT_CTRL+4);     //loopback
#else// FM_USE_AD_DA_LOOP
    outl(0x00000010,AUDIOOUT_CTRL+8);     //no loopback
#endif// FM_USE_AD_DA_LOOP

    outl(0x1000ac44,AUDIOIN_ADCSRR);    //44.1kHz

//  outl(cmd0,AUDIOIN_ADCVOLUME);
    outl(0x00fe00fe,AUDIOIN_ADCVOLUME); //adc volume
//  outl(0x00000000,AUDIOIN_ADCDEBUG);  //enable DMA
    outl(0x11000066,AUDIOIN_ADCVOL);    //adcmux volume

    outl(0x00000001,AUDIOIN_CTRL+4);

//  delay(1000);

    outl(0x10110037,AUDIOOUT_DACSRR);   //44.1kHz
//    #ifdef SPK
//    outl(0x00000000,AUDIOOUT_SPKRVOL);
//    #endif
//  outl(cmd1,AUDIOOUT_DACVOLUME);

    outl(0x00da00da,AUDIOOUT_DACVOLUME); //dac volume
//  outl(0xc0000000,AUDIOOUT_ANACLKCTRL+8);//ana clkgate
//  outl(0x00000000,AUDIOOUT_DACDEBUG);  //enable DMA
                                         // 
#if CONFIG_FM_USE_AD_DA_LOOP
    outl(0x00000303,AUDIOOUT_HPVOL);    // hpvol choose dac
    outl(0x00000001,AUDIOOUT_CTRL+4);     //run
#else// FM_USE_AD_DA_LOOP
    outl(0x00000100,AUDIOIN_ADCVOL +4);    //adcmux mute
    outl(0x01000303,AUDIOOUT_HPVOL);    // hpvol choose line 1
    outl(0x00000001,AUDIOOUT_CTRL+8);     // dac off
    outl(0x00000001,AUDIOIN_CTRL+8); // adc off
#endif// FM_USE_AD_DA_LOOP

    outl(0x00000010,AUDIOOUT_ANACTRL);  //anactrl
    outl(0x00000000,AUDIOOUT_TEST);     //test

    return 0;
}


/* read FM module configuation */
int fmradio_read(struct device * dev,d_offs ofs,void * buf,int count){
    i2c_addr = FM_I2C_ADDR | 1; // read addr
    return i2c_read(&i2c_addr,buf,count);     
}

/* set FM module configuation */
int fmradio_write(struct device * dev,d_offs ofs,const void * buf,int count){
    i2c_addr = FM_I2C_ADDR ; // write addr
    return i2c_write(&i2c_addr,buf,count);     
}

void fmradio_set_mhz(int tune_MHZ){
    int tune_N;

    tune_N = ((tune_MHZ * 100000 + 225*1000) >>13);
    //fm_info.fm_conf[0] = ((tune_N>>8) & 0x3f) | (fm_info.fm_conf[0] & 0xc0);
    fm_info.fm_conf[0] = (tune_N>>8) & 0x3f ;
    fm_info.fm_conf[1] = (tune_N & 0xff);
}


void fmradio_tune_up(struct device * dev){

    struct fmradio * f_info;
    int TuneN,value;

    f_info = (struct fmradio *) dev->priv_data;

    dev->read(dev,0,f_info->fm_confro,5);

    TuneN = ((f_info->fm_confro[0] &0x3f)<<8) + f_info->fm_confro[1];

    TuneN += FM_TUNE_STEP; 

    f_info->fm_conf[0] = (((TuneN>>8) &0x3f) ); 
    f_info->fm_conf[1] = (TuneN & 0xff);

    dev->write(dev,0,f_info->fm_conf,5); 

    value = (((TuneN<<13) - 225000) + 50000) / 100000;
    printf("\rFreq = %d.%d MHz",value / 10, value % 10);
}


void fmradio_tune_down(struct device * dev){

    struct fmradio * f_info;
    int TuneN,value;

    f_info = (struct fmradio *) dev->priv_data;

    dev->read(dev,0,f_info->fm_confro,5);

    TuneN = ((f_info->fm_confro[0] &0x3f)<<8) + f_info->fm_confro[1];

    TuneN -= FM_TUNE_STEP; 

    f_info->fm_conf[0] = (((TuneN>>8) &0x3f) ); 
    f_info->fm_conf[1] = (TuneN & 0xff);

    dev->write(dev,0,f_info->fm_conf,5); 

    value = (((TuneN<<13) - 225000) + 50000) / 100000;
    printf("\rFreq = %d.%d MHz",value / 10, value % 10);
}

void fmradio_search_up(struct device * dev){

    struct fmradio * f_info;
    int TuneN;

    f_info = (struct fmradio *) dev->priv_data;

    dev->read(dev,0,f_info->fm_confro,5);

    TuneN = ((f_info->fm_confro[0] &0x3f)<<8) + f_info->fm_confro[1];

    TuneN += FM_AUTO_TUNE_STEP; 

    f_info->fm_conf[0] = (((TuneN>>8) &0x3f) |  0x40); // enable auto search
    f_info->fm_conf[1] = (TuneN & 0xff);
    f_info->fm_conf[2] = (f_info->fm_conf[2] | 0x80); // swarch up

    dev->write(dev,0,f_info->fm_conf,5); 
}


void fmradio_search_down(struct device * dev){

    struct fmradio * f_info;
    int TuneN;

    f_info = (struct fmradio *) dev->priv_data;

    dev->read(dev,0,f_info->fm_confro,5);

    TuneN = ((f_info->fm_confro[0] &0x3f)<<8) + f_info->fm_confro[1];

    TuneN -= FM_AUTO_TUNE_STEP; 

    f_info->fm_conf[0] = (((TuneN>>8) &0x3f) |  0x40); // enable auto search
    f_info->fm_conf[1] = (TuneN & 0xff);
    f_info->fm_conf[2] =  (f_info->fm_conf[2] & 0x7f); // search down

    dev->write(dev,0,f_info->fm_conf,5); 
}

int fmradio_probe(struct device * dev){

    initial_i2c();

    return 0;
}

int fm_start(struct device * dev){

    init_loopback_audio();

    fm_info.fm_conf[0] = 0;
    fm_info.fm_conf[2] = 0x30; // search down, level low, injection HIGH
    //fm_info.fm_conf[2] = 0x50; // search down, level mid, injection HIGH
    //fm_info.fm_conf[2] = 0x70; // search down, level high, injection HIGH
    fm_info.fm_conf[3] = 0x17; // Europe band, 32.768 kHz, high cut noise cancel
    fm_info.fm_conf[4] = 0; 

    dev->ioctl(dev,FM_SET_MHZ,1017);  // 101.7 MHz
    dev->write(dev,0,fm_info.fm_conf,5); 

    printf("FM Radio Started\n");
}

int fmradio_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    switch (cmd) {
    case FM_START: // fm start
        fm_start(dev);
        return 0;
    case FM_STOP: // fm stop
        fm_stop(dev);
        return 0;
    case FM_SET_MHZ: // manually set station freq
        fmradio_set_mhz(arg);
        return 0;
    case FM_TUNE_UP:    // manually tuning station freq    
        fmradio_tune_up(dev);
        return 0;
    case FM_TUNE_DOWN:  // manually tuning station freq      
        fmradio_tune_down(dev);
        return 0;
    case FM_SEARCH_UP:    // searching station freq    
        fmradio_search_up(dev);
        return 0;
    case FM_SEARCH_DOWN:    // searching station freq       
        fmradio_search_down(dev);
        return 0;
    }
    return -1;
}


struct device dev_fmradio = {
    .name       = "fmradio",
    .dev_id     = 0,
    .probe      = fmradio_probe,
//    .remove     = fmradio_remove,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = fmradio_read,  
    .write      = fmradio_write, 
    .ioctl      = fmradio_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
//    .mmap       = fmradio_mmap,
    .status     = 0, 
    .priv_data  = &fm_info, 
};

__add_device(dev_fmradio);

/* =============== CMD =============== */
//#ifdef CONFIG_DEVMEM_DEBUG_CMD

int do_cmd_fmradio(cmd_tbl_t * cmdtp,int argc, char *argv[]){

    struct device * fm_dev;
    struct fmradio * dev_i;
    ulong value,tune_N;
    int dev_index;
    int conf_i;

    fm_dev = device_get("fmradio",&dev_index);
    dev_i = (struct fmradio *)fm_dev->priv_data;

	if(argc < 2){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

    if (argv[1][0]=='s') {
        fm_dev->read(fm_dev,0,fm_info.fm_confro,5);
        printf("FM Status:\n");
        if (fm_info.fm_confro[0] & 0x80) {
            printf("<=== Tune Ready ===>\n");
        }
        if (fm_info.fm_confro[0] & 0x40) {
            printf("!!!Reached Band Limit!!!\n");
        }
        tune_N = (( fm_info.fm_confro[0] & 0x3f) << 8) + fm_info.fm_confro[1];
        value = (((tune_N<<13) - 225000) + 50000) / 100000;
        printf("Freq = %d.%d MHz\n",value / 10, value % 10);
        printf("Output Level = %d\n",fm_info.fm_confro[3]>>4);
        dbg_hexdump(fm_info.fm_confro,8);

    }
    else  if(argv[1][0]=='o'){
        if (argv[1][1]=='n') { // on
            fm_dev->ioctl(fm_dev,FM_START,value);
        }
        else{ // off
            fm_dev->ioctl(fm_dev,FM_STOP,value);
        }
    }        
    else  if(argv[1][0]=='t'){
            value = TextToLong_TRL(argv[2]);
            fm_dev->ioctl(fm_dev,FM_SET_MHZ,value);
            fm_dev->write(fm_dev,0,fm_info.fm_conf,5);
    }        
    else  if(argv[1][0]=='u'){
            fm_dev->ioctl(fm_dev,FM_SEARCH_UP,0);
    }        
    else  if(argv[1][0]=='d'){
            fm_dev->ioctl(fm_dev,FM_SEARCH_DOWN,0);
    }        
    else  if(argv[1][0]=='+'){
            fm_dev->ioctl(fm_dev,FM_TUNE_UP,0);
    }        
    else  if(argv[1][0]=='-'){
            fm_dev->ioctl(fm_dev,FM_TUNE_DOWN,0);
    }        
    else  if(argv[1][0]=='w'){
        if (argc == 4) {
            conf_i = TextToLong_TRL(argv[2]);
            value = TextToLong_TRL(argv[3]);
            if (conf_i == 5) { // config the 5th byte
                fm_info.fm_conf[4] = value;
            }
            else { // config 1-4 byte
                printf("write 0x%08x to addr 0x%08x\n",value,fm_info.fm_conf);
                outl(value,fm_info.fm_conf);
            }
        }
        dbg_hexdump(fm_info.fm_conf,8);
        fm_dev->write(fm_dev,0,fm_info.fm_conf,5);
    }        
    return 0;
}

BOOT_CMD(fm,do_cmd_fmradio,
    "#fm [options]\n"
    "on\n"
    "off\n"
    "s (print status)\n"
    "t MHz (e.g. '1017' (101.7MHz ))\n"
    "u (search up)\n"
    "d (search down)\n"
    "+ (tune up)\n"
    "- (tune down)\n"
    "w\n"
    "w [1|5] value\n"
    , "Test FM Device");

//#endif// CONFIG_DEVMEM_DEBUG_CMD


