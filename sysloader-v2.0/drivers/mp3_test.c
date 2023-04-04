#include<common.h>
#include<drivers/sound.h>

#define PCM_SIZE  0x2e2000
#define BUFF_SIZE BUFFER_TOTAL_SIZE
struct device * sndi,*flashi;
void * snd_buff;

int snd_init(void){
    int dev_index;
    if((sndi = device_get("sound",&dev_index))==NULL)
        panic("device_get sound panic\n");
    if((flashi = device_get("nand",&dev_index))==NULL)
        panic("device_get nand panic\n");
    //flashi->open();
    return 0;
}
/*****************************/
#define BREAK
/*****************************/

#ifdef BREAK

int Section_TEXTLIB size = 0;
//unsigned char* Section_TEXTLIB addr_nand = (unsigned char*)0x4000000;
d_offs Section_TEXTLIB addr_nand = 0x10500000;

int mp3_play(void){
    int i,ret;
    unsigned char* addr_get;
    char cmd_buff[256];

    //printf("size %d addr_nand 0x%x\n",size,addr_nand);
    if (size == 0) {
        //if ((ret = snd_audio_open()) < 0) {
        if ((ret = sndi->open()) < 0) {
              printf("open error\n");
              return -EPERM;
         }
    }
    if (size >= PCM_SIZE) {
        size = 0;
        addr_nand = 0x10500000;
        sndi->close(sndi);
        printf("-----------break test ok-----------\n");
        //free_irq(INT_AS3310_DAC_DMA);
        //task_unmark(TASK_LOW,0);
        return 0;
    }

    //if ((ret = snd_buf_prepare(BUFF_SIZE))<=0) {
    if ((snd_buff = sndi->mmap(sndi,0,BUFF_SIZE)) == NULL) {
        return 0;
    }

    //alp_vsprintf(cmd_buff,"nandr data 0x%x 0x6000 0x%x",addr_nand,sound_buffer);
    //printf("%s\n",cmd_buff);
    //do_cmd(cmd_buff);
    flashi->read(flashi,addr_nand,snd_buff,BUFF_SIZE);
    //snd_buf_finish();
    if(sndi->ioctl(sndi,SND_BUFFER_OK,0)<0)
        panic("sound buffer finish error\n");

    size += BUFF_SIZE;
    addr_nand += BUFF_SIZE;


    return 0;
}

#else //BREAK

int mp3_play(void){
    int i,size,ret;
    unsigned char* addr_get;
    char cmd_buff[256];
    //unsigned char* addr_nand;
    d_offs  addr_nand;


    size = 0;  
    addr_nand = 0x10500000;

    if ((ret = sndi->open()) < 0) {
        printf("open error\n");
        return -EPERM;
    }
    while (size < PCM_SIZE) {
        while ((snd_buff = sndi->mmap(sndi,0,BUFF_SIZE)) == NULL ) {
        //while ((ret = snd_buf_prepare(BUFF_SIZE))<=0) {
        }
        flashi->read(flashi,addr_nand,snd_buff,BUFF_SIZE);
        if(sndi->ioctl(sndi,SND_BUFFER_OK,0)<0)
             panic("sound buffer finish error\n");

        //alp_vsprintf(cmd_buff,"nandr data 0x%x 0x6000 0x%x",addr_nand,sound_buffer);
        ////printf("%s\n",cmd_buff);
        //do_cmd(cmd_buff);
        //
        //snd_buf_finish();

        size += BUFF_SIZE;
        addr_nand += BUFF_SIZE;
    }
    sndi->close(sndi);

    printf("-----------no break test ok-----------\n");
    //dac_audio_close();

    return 0;
}
#endif //BREAK


t_ops t_mp3 = {
    .tname      = "as3310_mp3_test",
    .tid        = 1100,
    .task_init  = snd_init,
    .task_in    = NULL,
    .task_out   = NULL,
    .tfunc      = mp3_play,
};

//__define_init_task(t_mp3);

static
int do_audio_in(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
  int t,ret,dev_index,record_len;
  struct device *dev;
  d_offs addr_record;

  if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

  if((dev = device_get("sound",&dev_index))==NULL)
       panic("device_get sound panic\n");

  if ((ret = dev->open()) < 0) {
       printf("open error\n");
       return -EPERM;
   }
  if(dev->ioctl(dev,SND_AUDIO_IN,0)<0)
           panic("sound buffer finish error\n");

  addr_record = 0x20b00000;
  record_len = 0x200000;
  t = record_len/BUFFER_SIZE;
  printf("begin audio in\n");
  while (t --) {
      while (dev->read(dev,0,(void*)addr_record,BUFFER_SIZE) == 0) {
      }
      addr_record += BUFFER_SIZE;
      //printf("->");

  }

  dev->close(dev);


  return 0;
}

//#define OUT_TEST
static
int do_audio_out(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
  int t,ret,dev_index,record_len;
  struct device *dev,*flashi;
  d_offs addr_record;
  char * data;
  d_offs pcm_nand = 0x10500000;


  if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

  if((dev = device_get("sound",&dev_index))==NULL)
       panic("device_get sound panic\n");

  if ((ret = dev->open()) < 0) {
        printf("open error\n");
        return -EPERM;
    }
  if(dev->ioctl(dev,SND_AUDIO_OUT,0)<0)
          panic("sound buffer finish error\n");

  addr_record = 0x20b00000;
  record_len = 0x200000;
  t = record_len/BUFFER_SIZE;

#ifdef OUT_TEST
  if((flashi = device_get("nand",&dev_index))==NULL)
       panic("device_get nand panic\n");

  flashi->read(flashi,pcm_nand,(void*)addr_record,record_len);


#endif //OUT_TEST

  printf("begin audio out\n");

  while (t --) {
      while ((data = dev->mmap(dev,0,BUFFER_SIZE))==NULL) {
      }
      //ret = dev->read(dev,0,(void*)addr_record,BUFFER_SIZE);
      memcpy((void*)data,(void*)addr_record,BUFFER_SIZE);
      addr_record += BUFFER_SIZE;
      if(dev->ioctl(dev,SND_BUFFER_OK,0)<0)
             panic("sound buffer finish error\n");
      //printf("->");
  }

  dev->close(dev);


  return 0;
}

BOOT_CMD(sndin,do_audio_in,
        "#sndin ",
        "test audio in");
BOOT_CMD(sndout,do_audio_out,
        "#sndout ",
        "test audio out");

