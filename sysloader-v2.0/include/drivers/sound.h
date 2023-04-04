/*
Change log: 

------------------- Version 1.0  ----------------------
zhangbo 2007-12-14
    -- Create file

*/

#ifndef __SOUND_H__
#define __SOUND_H__

#define BUFFER_SIZE 0x6000                 //24kX4   buffer 
#define BUFFER_TOTAL_SIZE    0x6000
#define SND_PKG_NUM  BUFFER_TOTAL_SIZE/BUFFER_SIZE
#define BUFS 3
#define AS3310_DMA_SNDOUT_CH 1
#define AS3310_DMA_SNDIN_CH  0
#define MAX_DATA_WAIT 0x10000
#define SND_INFO

/*
 * ----------------------------------------------------------------------------
 * audio
 * ----------------------------------------------------------------------------
 */

#define XTAL_CTRL     0x80040050
#define AUDIOOUT_CTRL 0X80048000 
#define AUDIOOUT_CTRL_SET AUDIOOUT_CTRL+0x4
#define AUDIOOUT_CTRL_CLR AUDIOOUT_CTRL+0x8
#define AUDIOOUT_CTRL_TOG AUDIOOUT_CTRL+0xc

#define AUDIOOUT_PWRDN 0X80048070
#define AUDIOOUT_PWRDN_CLR AUDIOOUT_PWRDN+0x8
#define AUDIOOUT_PWRDN_SET AUDIOOUT_PWRDN+0x4

#define REFCTRL 0x80048080

#define AUDIOOUT_ANACLKCTL 0x800480e0
#define AUDIOOUT_ANACLKCTL_CLR AUDIOOUT_ANACLKCTL+0x8
#define AUDIOOUT_ANACLKCTL_SET AUDIOOUT_ANACLKCTL+0x4

#define AUDIOOUT_DACSRR 0x80048020

#define AUDIOOUT_DACVOLUME 0x80048030
#define AUDIOOUT_ANACTRL 0X80048090
#define AUDIOOUT_TEST 0x800480a0
#define AUDIOOUT_HPVOL 0x80048050
#define AUDIOOUT_HPVOL_SET AUDIOOUT_HPVOL + 0x4
#define AUDIOOUT_HPVOL_CLR AUDIOOUT_HPVOL + 0x8

#define AUDIOOUT_DATA 0x800480f0

#define AUDIOOUT_DEBUG 0x80048040

#define AUDIOOUT_SPKRVOL 0x80048060
#define AUDIOOUT_SPKRVOL_CLR AUDIOOUT_SPKRVOL+0x8
#define AUDIOOUT_SPKRVOL_SET AUDIOOUT_SPKRVOL+0x4

#define AUDIOIN_CTRL 0x8004c000  
#define AUDIOIN_CTRL_SET AUDIOIN_CTRL+0x4
#define AUDIOIN_CTRL_CLR AUDIOIN_CTRL+0x8
#define AUDIOIN_CTRL_TOG AUDIOIN_CTRL+0xc

#define AUDIOIN_ADCVOL       0x8004c050


///*   DIGFILT register   */
//audioin adc register
#define AUDIOIN_ADCSRR         0x8004c020
#define AUDIOIN_ADCVOLUME      0x8004c030
#define AUDIOIN_ADCDEBUG       0x8004c040
#define AUDIOIN_ANACLKCTRL     0x8004c070
#define AUDIOIN_ANACLKCTRL_SET AUDIOIN_ANACLKCTRL+0x4
#define AUDIOIN_ANACLKCTRL_CLR AUDIOIN_ANACLKCTRL+0x8
#define AUDIOIN_ANACLKCTRL_TOG AUDIOIN_ANACLKCTRL+0xc
//
////audioout dac register
//#define AUDIOOUT_DACSRR        0x80048020
//#define AUDIOOUT_DACVOLUME     0x80048030
//#define AUDIOOUT_DACDEBUG      0x80048040
//#define AUDIOOUT_SPKRVOL       0x80048060
#define AUDIOOUT_ANACLKCTRL    0x800480e0

// DMA register
#define DMA_CTRL0    0x80024000
#define DMA_CTRL1    0x80024010
#define CH1_CURCMDAR 0x800240a0
#define CH1_NXTCMDAR 0x800240b0
#define CH1_CMD      0x800240c0
#define CH1_BAR      0x800240d0
#define CH1_SEMA     0x800240e0
#define CH0_CURCMDAR 0x80024030
#define CH0_NXTCMDAR 0x80024040
#define CH0_CMD      0x80024050
#define CH0_BAR      0x80024060
#define CH0_SEMA     0x80024070

#define SND_BUFFER_OK 10
#define SND_AUDIO_IN  11
#define SND_AUDIO_OUT 12

#define HEADPHONE 3
#define SPEAKER 4

struct as3310_snd_buf {
    //ulong               buf_chain_start_dma[BUFS];
    //unsigned char       *buf_chain_start_cpu[BUFS];
    unsigned char*      buf_chain_start[BUFS];
    unsigned char       dma_ptr;
    unsigned char       usr_ptr;
    u_int               one_buffer_len;
    unsigned char       chain_full[BUFS];
    int                 send_len[BUFS];

};

struct as3310snd_info {
    struct as3310_snd_buf      * pkg_buf; 
	/* raw memory addresses */

    unsigned char *		map_cpu;	
    u_int           buf_size;

    struct as3310_dma_chain * dmachain;

    int in_use;
    char first_call;
    int audio_in_out;//out:1 in:0

};

//extern unsigned char *sound_buffer;

int as3310_sound_init(struct device * dev);
//int snd_buf_prepare(int count);
void snd_buf_finish(void);
int snd_audio_open(void);
int dac_audio_close(struct device * dev);
void * snd_mmap(struct device * dev,int flag,int count);
int snd_ioctl(struct device * dev,unsigned int cmd,unsigned long arg);


static void  audioout_hw_init(void);
static void dac_init(void);
static void dac_ref_ctl(void);
static int dac_audio_set_rate(int v);
static void dac_volume(void);
static void as3310snd_init_registers(void);
static void dac_audio_reset(void);

static void sound_memory_get(void);
static int as3310_snd_handler(int d);
static int clear_as3310_snd(int d);
static int snd_pkg_set(unsigned char which,int sent,struct as3310snd_info *sndi);
static void start_dma_transfer(void);
static void dac_audio_sync(void);
#ifdef CONFIG_SYS_MONITOR
void audio_pin_init(void);
int audioout_hw_init_det(void);
#endif //CONFIG_SYS_MONITOR

static int audio_in_pkg_set(unsigned char which,int sent,struct as3310snd_info *sndi);
static void start_audioin_dma(void);


int mp3_play(void);
int snd_init(void);

/***************** from ironpalms ********************/
//#define file 0x2ee000                              //
//#define transfer 0x9600                            //
//#define PKG_NUM  40                                //
//                                                   //
//#define DAC_BASE_ADDR	    0x20300000               //
//#define TEST_MEM_AUDIO      0x20300000             //
////#define HEAD                                     //
//#define HEAD                                       //
/***************** from ironpalms ********************/


#endif
