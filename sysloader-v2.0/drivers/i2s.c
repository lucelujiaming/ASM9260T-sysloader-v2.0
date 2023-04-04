

/*

Alpha Scale AS3310X IronPalms Console
zhangbo, AlpScale Software Engineering

*/

/*****************************
// i2s test progam 
*****************************/
#include <common.h>

#define IS_DMA_I2S_COMPLETE() ((inl(HW_APBX_CH2_SEMA)&0x00ff0000)==0)
#define SELECT_I2S()  (outl(0x00000100,HW_APBX_DEVSEL))
//#define CFG_I2S(data) (outl(data,I2SCFG))//ask lx
//#define XCLK_SET(div) (outl(div,HW_CLKCTRL_XBUSCLKCTRL))
//#define I2S_PKG_NUM 2
//#define i2sdebug

void INIT_I2S(void) 
{
    outl(0x40000000,I2s_CTRL_CLR);
    outl(0x80000000,I2s_CTRL_CLR);
}


void CFG_I2S(u32 data){
   // outl(data,AS3310_I2S_BASE);
    outl(data,I2SCFG);
}


uchar i2s_write(u32 buffer,int dlength)
{
    char i;
    ulong transfer;
    ulong num_word,pkg_num;
    ulong limit=0x9600;
    int retry=0x1000000;
    int wait_i2s=0x10000;
    int cycle_sin=1000; 
    void * i2s;
    printf("buffer:0x%x,dlength:0x%x\n",buffer,dlength);
    /************************************************************/  
//    ulong * test=0x20000000;
   
//    for (i=0;i<0x50;i++) {
//        *(test++)=0xa5a5a5a5;
//        *(test++)=0x5a5a5a5a;
//    }
   
  /************************************************************/ 
    dlength-=0x20;

    if((dlength%limit)==0)pkg_num=(dlength/limit)+1;
    else pkg_num=(dlength/limit)+2;//determine number of pkg
    printf("pkg_num:%d\n",pkg_num);
    AS3310_DMA_PKG  * i2s_dma_pkg[pkg_num];

    //for (i=0;i<pkg_num;i++) {
    //    i2s_dma_pkg[i] = (AS3310_DMA_PKG *)(I2S_PKG + i*sizeof(AS3310_DMA_PKG));
    //}
    i2s= nc_malloc(pkg_num*sizeof(AS3310_DMA_PKG));
    for(i=0;i<pkg_num;i++)
    	{
    		i2s_dma_pkg[i] = ( AS3310_DMA_PKG *)((int)i2s + i*sizeof(AS3310_DMA_PKG)); 
    	}
 

	
    for (i=0;i<pkg_num-1;i++) {
        i2s_dma_pkg[i]->NEXT_PKG =  (ulong) ( &( i2s_dma_pkg[i+1]->NEXT_PKG ));
    }
    i2s_dma_pkg[pkg_num-1]->NEXT_PKG = 0;
  /************************************************************/  

    outl(0x20000000,HW_PINCTRL_MUXSEL6_CLR);//pin control
    outl(0x3F000000,HW_PINCTRL_MUXSEL3_CLR);


    INIT_I2S();//clr clockgate

    outl(0x0,DMAX_CTRL0_ADDR);
    outl(0x0,DMAX_CTRL0_ADDR);

    SELECT_I2S();//i2s or spdif

    CFG_I2S(0x003f10f0);//i2s config
  /************************************************************/  

    transfer=limit;
    num_word=transfer/4;

    i2s_dma_pkg[0]->CTRL = 0x000010c6 + (0x20<<16);
    i2s_dma_pkg[0]->BUFFER = (u32)buffer ;
    i2s_dma_pkg[0]->CMD0 = (0x10000000 + 0x8);
				//puth(buffer);
				//putc('\n');
    buffer+=0x20;

    for(i=1;i<(pkg_num-1);i++)
    {
        i2s_dma_pkg[i]->CTRL = 0x000020c6 + (transfer<<16);
        i2s_dma_pkg[i]->BUFFER = (u32)buffer;  
        i2s_dma_pkg[i]->CMD0 = (0x10000000 + num_word);      
        i2s_dma_pkg[i]->CMD1 = 0x00000061;

	//puth(buffer);
	//putc(uchar data)('\n');
	
        buffer+=transfer;
    }

    i2s_dma_pkg[pkg_num-1]->CTRL = 0x000020c2 + (transfer<<16);
    i2s_dma_pkg[pkg_num-1]->BUFFER = (u32)buffer;
    i2s_dma_pkg[pkg_num-1]->CMD0 = (0x10000000 + num_word);      
    i2s_dma_pkg[pkg_num-1]->CMD1 = 0x00000061;

    outl((ulong)(&(i2s_dma_pkg[0]->NEXT_PKG)),DMAX_CH2_NXTCMDAR);
    outl(pkg_num,DMAX_CH2_SEMA);
 
    printf("########waiting########\n");
    while(IS_DMA_I2S_COMPLETE()==0){
          if(wait_i2s--==0)
          	{
          	  printf("*");
			  wait_i2s=0x10000;
          	}
            if (retry-- < 0)
		{
			printf("out of time\n");			
			return 1;
            	}
    }
    printf("\n###transfer is over###\n");
  /************************************************************/  
    //not determine the last pkg in different situation
  /************************************************************/  
    asfree(i2s);
    return 0;
}

int do_i2s_write(cmd_tbl_t *cmdtp,int argc,char * argv[])
{

	ulong addr;
	ulong buffer;
	int   nbyte;
 
    if (argc != 3) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}   

	buffer = TextToLong_TRL(argv[1]);
	nbyte = TextToLong_TRL(argv[2]);
    printf("prepare to send using i2s\n");
	i2s_write(buffer,nbyte);
    return 0;
}

BOOT_CMD(i2sw,do_i2s_write,
         " #i2sw addr nbytes", 
         "send data using i2s");
