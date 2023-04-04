#include <common.h>
#include <drivers/sound.h>
//#include <drivers/as3310x_monitor.h>

unsigned char *sound_buffer;
struct timer_list snd_timer;
static struct as3310snd_info     *snd;

#ifdef CONFIG_SYS_MONITOR
void audio_pin_init(void){
    request_as3310_gpio(GPIO_HP_DET_PORT,GPIO_HP_DET_PIN,PIN_FUNCTION_3);
    request_as3310_gpio(GPIO_SPK_PORT,GPIO_SPK_PORT,PIN_FUNCTION_3);
}
static void adc_register_init(void){
    outl(0xc0000000,AUDIOOUT_CTRL_CLR);     //clkgate out & loopback & 16bit PCM
    outl(0xc0000000,AUDIOIN_CTRL_CLR);      //clkgate in & no loopback & 16bit PCM
    outl(0x80000000,AUDIOIN_ANACLKCTRL_CLR); //ana clkgate
//  outl(0x80000000,AUDIOOUT_ANACLKCTRL+8);//ana clkgate
    outl(0x00000100,AUDIOOUT_PWRDN+8);    //pwdn ref
    outl(0x000037a0,REFCTRL);           //ref ctrl

    outl(0x00000020,AUDIOIN_CTRL+4);      //no loopback & 16bit PCM
//  outl(0x00000051,AUDIOOUT_CTRL+4);     //loopback & 16bit PCM

    outl(0x1000ac44,AUDIOIN_ADCSRR);    //44.1kHz

    outl(0x00f000f0,AUDIOIN_ADCVOLUME); //adc volume
    outl(0x00000000,AUDIOIN_ADCDEBUG);  //enable DMA
    //#ifdef LINE
    outl(0x11000066,AUDIOIN_ADCVOL);    //adcmux volume
    //outl(0x11000066,AUDIOIN_ADCVOL);         //ADC Mux Volume
    //#endif
    #ifdef MIC
    outl(0x00000000,AUDIOIN_ADCVOL);    //adcmux volume
    #endif
 //   outl(0x00000010,AUDIOIN_CTRL+4);      //loopback & 16bit PCM
}

int audioout_hw_init_det(void){
    int head_io;
    head_io = read_GPIO(GPIO_HP_DET_PORT,GPIO_HP_DET_PIN);
    if (head_io) {
        clear_GPIO(GPIO_SPK_PORT,GPIO_SPK_PORT);//hw close speaker
        return HEADPHONE;
    }else {
        set_GPIO(GPIO_SPK_PORT,GPIO_SPK_PORT);//hw open speaker
        return SPEAKER;
    }
}
#endif //CONFIG_SYS_MONITOR
static void  audioout_hw_init(void){
    
    #ifdef CONFIG_SYS_MONITOR
    int headio;
    headio = audioout_hw_init_det();
    if(headio == HEADPHONE) {
            printf("headphone inited\n");
            outl(0x00001001,AUDIOOUT_PWRDN_CLR);//power on headphone
            outl(0x01000000,AUDIOOUT_PWRDN_SET);//power down speaker
            outl(0x00010000,AUDIOOUT_HPVOL_CLR);//demute headphone
            outl(0x00000303,AUDIOOUT_HPVOL);//demute headphone
            outl(0x00010000,AUDIOOUT_SPKRVOL_SET);//mute speaker
    }
    if(headio == SPEAKER) {
            printf("speaker inited\n");
            outl(0x01001000,AUDIOOUT_PWRDN_CLR);
            outl(0x00000001,AUDIOOUT_PWRDN_SET);
            outl(0x00010000,AUDIOOUT_SPKRVOL_CLR);
            outl(0x00010000,AUDIOOUT_HPVOL_SET);
    }
    #else //CONFIG_SYS_MONITOR
    printf("****no monitor, headphone inited***\n");
     
    outl(0x00001001,AUDIOOUT_PWRDN_CLR);//power on headphone
    outl(0x01000000,AUDIOOUT_PWRDN_SET);//power down speaker
    outl(0x00010000,AUDIOOUT_HPVOL_CLR);//demute headphone
    outl(0x00010000,AUDIOOUT_SPKRVOL_SET);//mute speaker
    
    #endif //CONFIG_SYS_MONITOR
}

static void dac_init(void)
{
  outl(0x00000000,XTAL_CTRL); 
  outl(0x80000000,AUDIOOUT_CTRL_SET);
  outl(0xc0000000,AUDIOOUT_CTRL_CLR);
  outl(0x00000008,AUDIOOUT_CTRL_CLR);
  audioout_hw_init();
  outl(0x00000040,AUDIOOUT_CTRL_SET);

  #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
  alp_printf("set audioout control register\n");
  #endif
    
}

static void dac_ref_ctl(void)
{
    outl(0x00003770,REFCTRL);

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    alp_printf("set audioout reference control register\n");
    #endif
}

static int dac_audio_set_rate(int v)
{

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    alp_printf("set audioout sample rate %d\n",v);
    #endif
    
    switch(v)
    {
    case 8000:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x13170e00,AUDIOOUT_DACSRR);
        return v;
    case 11025:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x13110037,AUDIOOUT_DACSRR);
        return v;
    case 22050:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x11110037,AUDIOOUT_DACSRR);
        return v;
    case 44100:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x10110037,AUDIOOUT_DACSRR);
        return v;
    case 48000:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x100f13ff,AUDIOOUT_DACSRR);
        return v;
    case 96000:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x200f13ff,AUDIOOUT_DACSRR);
        return v;
    case 128000:
        outl(0x80000000,AUDIOOUT_ANACLKCTL_CLR);
        outl(0x40170e00,AUDIOOUT_DACSRR);
        return v;
    default:
    #if INFO_DEBUG
		alp_printf(KERN_ERR "audio sample rate set error, wrong value of %d\n",v);
    #endif
		return -EINVAL;
    }
}

static void dac_volume(void)
{ 
    outl(0x00ff00ff,AUDIOOUT_DACVOLUME);
    outl(0x00000010,AUDIOOUT_ANACTRL);
    outl(0x00000230,AUDIOOUT_TEST);

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    alp_printf("set audioout volume register\n");
    #endif
}

static void as3310snd_init_registers(void){
    dac_init();
    dac_ref_ctl();
    dac_audio_set_rate(44100);
    dac_volume();

    //outl(0x00000001,AUDIOOUT_CTRL_SET);
    //adc_register_init();

    printf("audio initialized\n");
}


static void dac_audio_reset(void)
{
    unsigned char i;

    for(i=0;i<BUFS;i++){
        snd->pkg_buf->chain_full[i] = 0;
        snd->pkg_buf->send_len[i] = BUFFER_SIZE;
    }

    snd->first_call = 0;

    snd->pkg_buf->dma_ptr = 0;
    snd->pkg_buf->usr_ptr = 0;
}

static void sound_memory_get(void){
    int i;

    snd->pkg_buf->one_buffer_len = BUFFER_TOTAL_SIZE;

    /***************** buffer addr define ***************/
    for (i=0;i<BUFS;i++) {
        snd->pkg_buf->buf_chain_start[i] = (unsigned char*)nc_malloc(snd->pkg_buf->one_buffer_len);//(char *)DAC_BASE_ADDR;//(char *)nc_malloc(transfer*PKG_NUM);
        //printf("snd->pkg_buf->buf_chain_start[%d]:0x%x\n",i,snd->pkg_buf->buf_chain_start[i]);
        if (snd->pkg_buf->buf_chain_start[i] == NULL) {
             alp_printf("memory_get sound buffer %d malloc fail\n",i);
             while(1);
         }
    }
    snd->dmachain= request_as3310_dma_chain(SND_PKG_NUM,AS3310_DMA_SNDOUT_CH);
    if (snd->dmachain == NULL) {
          alp_printf("memory_get request_as3310_dma_chain fail\n");
          while(1);
    }
    outl(0x00020000,HW_APBX_CTRL1_SET);  // enable DMA ch 
    outl(0x00010000,HW_APBX_CTRL1_SET);  // enable DMA ch 

    //#ifdef CONFIG_SOUND_AS3310_SND_DEBUG   
    for (i=0;i<BUFS;i++) {
        alp_printf("audio dma pkg_buf %d addr:%p\n",i,snd->pkg_buf->buf_chain_start[i]);
    }
    //#endif //CONFIG_SOUND_AS3310_SND_DEBUG   
     
    snd->in_use = 0;
}

static int clear_as3310_snd(int d){
    outl(0x00000002, HW_APBX_CTRL1_CLR); // Clear
}

static int clear_as3310_snd_in(int d){
    outl(0x00000001, HW_APBX_CTRL1_CLR); // Clear
}

static int as3310_snd_handler(int d){

    snd->pkg_buf->chain_full[snd->pkg_buf->dma_ptr] = 0;
    //printf("transfer complete,snd->pkg_buf->dma_ptr:%d\n",snd->pkg_buf->dma_ptr);

    snd->pkg_buf->dma_ptr = (snd->pkg_buf->dma_ptr+1)%BUFS;

    if(snd->pkg_buf->chain_full[snd->pkg_buf->dma_ptr] == 1){
        snd_pkg_set(snd->pkg_buf->dma_ptr,snd->pkg_buf->send_len[snd->pkg_buf->dma_ptr],snd);
        start_dma_transfer();

    }else 
    {
        #ifdef CONFIG_SOUND_AS3310_SND_DEBUG   
        printf("all buffer is clear\n");
        #endif
        snd->first_call = 0;
    }
}

static int as3310_audioin_handler(int d){
    //printf("as3310_audioin_handler now\n");

    snd->pkg_buf->chain_full[snd->pkg_buf->dma_ptr] = 1;
    //printf("transfer complete,snd->pkg_buf->dma_ptr:%d\n",snd->pkg_buf->dma_ptr);

    snd->pkg_buf->dma_ptr = (snd->pkg_buf->dma_ptr+1)%BUFS;

    if(snd->pkg_buf->chain_full[snd->pkg_buf->dma_ptr] == 0){
        audio_in_pkg_set(snd->pkg_buf->dma_ptr,BUFFER_SIZE,snd);
        start_audioin_dma();

    }else 
    {
        #ifdef CONFIG_SOUND_AS3310_SND_DEBUG   
        printf("all buffer is clear\n");
        #endif
        snd->first_call = 1;
    }
}

int as3310_sound_init(struct device * dev){
    int ret;
    int i;
    struct irq_action snd_out_irq,snd_in_irq;

    snd_out_irq.irq = INT_AS3310_DAC_DMA;
    snd_out_irq.irq_handler = as3310_snd_handler;
    snd_out_irq.clear = clear_as3310_snd;
    snd_out_irq.priv_data = INT_AS3310_DAC_DMA;
    request_irq(&snd_out_irq);

    snd_in_irq.irq = INT_AS3310_ADC_DMA;
    snd_in_irq.irq_handler = as3310_audioin_handler;
    snd_in_irq.clear = clear_as3310_snd_in;
    snd_in_irq.priv_data = INT_AS3310_ADC_DMA;
    request_irq(&snd_in_irq);

    if(!(snd =c_malloc(sizeof(struct as3310snd_info)))){
              puts("snd memory alloc error\n");
              return -ENOMEM;
    }

    if(!(snd->pkg_buf = c_malloc(sizeof(struct as3310_snd_buf)))){
       puts("snd->buf memory alloc error\n");
       return -ENOMEM;
    }

    sound_memory_get();
    
#ifdef CONFIG_SYS_MONITOR
    audio_pin_init();
#endif //CONFIG_SYS_MONITOR
    //as3310snd_init_registers();

    return 0;
}


static int snd_pkg_set(unsigned char which,int sent,struct as3310snd_info *sndi){
    struct as3310_dma_chain * pdmachain;
    struct as3310_dma_pkg_s * dmapkg;
    unsigned long dma_phy_addr;
    unsigned char i;

    pdmachain = sndi->dmachain;
    dmapkg = pdmachain->chain_head;
    dma_phy_addr = (ulong)sndi->pkg_buf->buf_chain_start[which];

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    printf("dma buffer which:%d\n",which);
    printf("dma_phy_addr:0x%x\n",(unsigned int)dma_phy_addr );
    printf("dma sent set:%d\n",sent);
    #endif

    for(i=0;i<SND_PKG_NUM;i++){
        dmapkg[i].CTRL = 0x00001046 + (sent<<16);
        dmapkg[i].BUFFER = (ulong)(dma_phy_addr + i*BUFFER_SIZE);
        dmapkg[i].CMD0 = 0x00000041;       
    }
    dmapkg[SND_PKG_NUM-1].NEXT_PKG = 0;
    dmapkg[SND_PKG_NUM-1].CTRL = 0x0000104a + (sent<<16);
    dmapkg[SND_PKG_NUM-1].BUFFER = (ulong)(dma_phy_addr + (SND_PKG_NUM-1)*BUFFER_SIZE);
    return 0;
}

static int audio_in_pkg_set(unsigned char which,int sent,struct as3310snd_info *sndi){
    struct as3310_dma_chain * pdmachain;
    struct as3310_dma_pkg_s * dmapkg;
    unsigned long dma_phy_addr;
    unsigned char i;

    pdmachain = sndi->dmachain;
    dmapkg = pdmachain->chain_head;
    dma_phy_addr = (ulong)sndi->pkg_buf->buf_chain_start[which];

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    printf("dma buffer which:%d\n",which);
    printf("dma_phy_addr:0x%x\n",(unsigned int)dma_phy_addr );
    printf("dma sent set:%d\n",sent);
    #endif

    for(i=0;i<SND_PKG_NUM;i++){
        dmapkg[i].CTRL = 0x00001045 + (sent<<16);
        dmapkg[i].BUFFER = (ulong)(dma_phy_addr + i*BUFFER_SIZE);
        dmapkg[i].CMD0 = 0x00000061;       
    }
    dmapkg[SND_PKG_NUM-1].CTRL = 0x00001049 + (sent<<16);
    dmapkg[SND_PKG_NUM-1].BUFFER = (ulong)(dma_phy_addr + (SND_PKG_NUM-1)*BUFFER_SIZE);
    dmapkg[i].CMD0 = 0x00000061;       

    return 0;
}

static void start_dma_transfer(void){
    int wait_ch_ready;

    
    wait_ch_ready = 0;

    //#ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    //printf("snd->dmachain->chain_phy_addr:0x%x\n",snd->dmachain->chain_phy_addr);
    //#endif

    while( dma_start_apbx((ulong)snd->dmachain->chain_phy_addr,SND_PKG_NUM,AS3310_DMA_SNDOUT_CH) == -1) {
        if(wait_ch_ready++ > MAX_DATA_WAIT){
            printf("DMA ch(%d) busy,can't started\n",AS3310_DMA_SNDOUT_CH);
            break;
        }
    }
}

static void start_audioin_dma(void){
    int wait_ch_ready;
    wait_ch_ready = 0;

    //#ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    //printf("snd->dmachain->chain_phy_addr:0x%x\n",snd->dmachain->chain_phy_addr);
    //#endif

    while( dma_start_apbx((ulong)snd->dmachain->chain_phy_addr,SND_PKG_NUM,AS3310_DMA_SNDIN_CH) == -1) {
        if(wait_ch_ready++ > MAX_DATA_WAIT){
            printf("DMA ch(%d) busy,can't started\n",AS3310_DMA_SNDIN_CH);
            break;
        }
    }
}

//int snd_buf_prepare(int count)
void * snd_mmap(struct device * dev,int flag,int count)
{
    unsigned long ret;
    void * ret_buf;

    if(count > BUFFER_SIZE)count = BUFFER_SIZE;

    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    printf("data to be write :%d\n",count);
    #endif
    
    if(snd->pkg_buf->chain_full[snd->pkg_buf->usr_ptr] == 0){
        sound_buffer = snd->pkg_buf->buf_chain_start[snd->pkg_buf->usr_ptr];
        ret_buf = (void*)sound_buffer;
        snd->pkg_buf->send_len[snd->pkg_buf->usr_ptr] = count;
        #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
            printf("sound_buffer addr now:0x%x\n",sound_buffer);
        #endif
    }else
    {
        #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
        //printf("all buffer is full, now wait buffer %d\n",snd->pkg_buf->usr_ptr);
        #endif
        return NULL;
    }
    return ret_buf;
}

void snd_buf_finish(void)
{

    snd->pkg_buf->chain_full[snd->pkg_buf->usr_ptr] = 1;
    snd->pkg_buf->usr_ptr = (snd->pkg_buf->usr_ptr + 1)%BUFS;
    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    printf("snd_buf_finish\n");
    #endif

    if(snd->first_call>0){
        snd->first_call--;
    }else if(snd->first_call == 0){
        snd_pkg_set(snd->pkg_buf->dma_ptr,snd->pkg_buf->send_len[snd->pkg_buf->dma_ptr],snd);
        start_dma_transfer();
        #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
        printf("first start dma transfer\n");
        #endif
        snd->first_call--;
    }

}


static int  snd_audio_read(struct device * dev,d_offs ofs,void * buf,int count)
{
    int i;
    char *data_buffer;
    unsigned long ret;

    data_buffer = (char *)buf;

    if(count > BUFFER_SIZE)count = BUFFER_SIZE;

    if (snd->first_call > 0) {
        snd->first_call--;
    }else if(snd->first_call == 0)
        {
        //printf("need dma free HW_APBX_CH0_SEMA is 0x%x\n",inl(HW_APBX_CH0_SEMA));
        if ((inl(HW_APBX_CH0_SEMA)==0)&&(snd->pkg_buf->chain_full[snd->pkg_buf->dma_ptr] == 0)) {

            for (i=0;i<BUFS;i++) {
                snd->pkg_buf->send_len[i]= BUFFER_SIZE;
            }
            printf("first start dma\n");
            audio_in_pkg_set(snd->pkg_buf->dma_ptr,BUFFER_SIZE,snd);
            start_audioin_dma();
            #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
            printf("first start dma transfer\n");
            #endif
            snd->first_call--;
        }
     }
    //i=1000;
    //while(snd->pkg_buf->chain_full[snd->pkg_buf->usr_ptr] == 0){
    //    if (i--) {
    //        //printf("HW_APBX_CH0_SEMA is 0x%x\n",inl(HW_APBX_CH0_SEMA));
    //        return -5;
    //    }
    //}
    //printf("get buffer\n");
    if (snd->pkg_buf->chain_full[snd->pkg_buf->usr_ptr] == 1) {

        if (count < snd->pkg_buf->send_len[snd->pkg_buf->dma_ptr]) {
            snd->pkg_buf->send_len[snd->pkg_buf->dma_ptr] = count;
        }
        ret = memcpy((void*)data_buffer,(void*)(snd->pkg_buf->buf_chain_start[snd->pkg_buf->usr_ptr]),snd->pkg_buf->send_len[snd->pkg_buf->dma_ptr]);
        if(ret == NULL){
            printf("copy from user error, ret=%d\n",(int)ret);
        }
        snd->pkg_buf->chain_full[snd->pkg_buf->usr_ptr] = 0;
        snd->pkg_buf->usr_ptr = (snd->pkg_buf->usr_ptr + 1)%BUFS;
        return count;
    }
    return 0;

}

static void dac_audio_sync(void)
{
    unsigned char empty,i;
    empty = 0;

    do{
        for(i=0;i<BUFS;i++)
            empty += snd->pkg_buf->chain_full[i];
    }while(empty);
    #ifdef CONFIG_SOUND_AS3310_SND_DEBUG
    printf("syncing...\n");
    #endif
}

int snd_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    switch (cmd) {
    case SND_BUFFER_OK:
        snd_buf_finish();
        return 0;
    case SND_AUDIO_IN:
         snd->audio_in_out = 0;
         adc_register_init();
         return 0;
    case SND_AUDIO_OUT:
         snd->audio_in_out = 1;
         as3310snd_init_registers();
         return 0;
    }
    return -1;
}

int snd_audio_open(void)
{
    //as3310snd_init_registers();
    dac_audio_reset();
    #ifdef SND_INFO
    printf("audio opened\n");
    #endif
	if (snd->in_use)
		return -EBUSY;

	snd->in_use = 1;

    return 0;
}

int dac_audio_close(struct device * dev)
{
    #ifdef SND_INFO
    printf("audio released\n");
    #endif
    if (snd->audio_in_out) {
        dac_audio_sync();
    }
    if (snd->audio_in_out) {
        outl(0x00020000,HW_APBX_CTRL0_SET);
        outl(0x00000001,AUDIOOUT_CTRL_CLR);
    }else{
        outl(0x00010000,HW_APBX_CTRL0_SET);
        outl(0x00000001,AUDIOIN_CTRL_CLR);
    }
    //outl(0x00010000,HW_APBX_CTRL0_SET);
    //outl(0x00020000,HW_APBX_CTRL0_SET);

	snd->in_use = 0;

	return 0;
}

struct device dev_sound = {
    .name       = "sound",
    .dev_id     = 0,
    .probe      = as3310_sound_init,
    .remove     = NULL,  
    .open       = snd_audio_open,  
    .close      = dac_audio_close,  
    .read       = snd_audio_read,  
    .write      = NULL, 
    .ioctl      = snd_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = snd_mmap,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device(dev_sound);

//__init_device(as3310_sound_init);

/*
BOOT_CMD(testda,test_da,
         " for the test of dac ",
         " for the test of dac ");

*/

//BOOT_CMD(da_s,DACDMAStart,
//         " for the test dma start of dac ",
//         " for the test dma start of dac ");

//TEST_MODULE(AUDIOOUT,test_da);
