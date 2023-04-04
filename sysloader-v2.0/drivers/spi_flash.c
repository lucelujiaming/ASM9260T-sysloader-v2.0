/******************************************************
* AlphaScale ASAP18xx BootCode
*  
* spi_flash.c 
*  
* SPI-Flash boot support
* 
* Copyright (C) 2009 AlphaScale Inc.
* He Yong <hoffer@sjtu.org>
* 
* ------------------- Modifications  ------------------
* Create File, Support 4 types of SPI Flash
*  He Yong 2009-05-16
* 
******************************************************/

#include <common.h>
#include "drivers/spi_flash.h"

#define spi_debug(x) if(0) puts(x)
#define spi_puth(x)  if(0) puth(x)

static NCached AS3310_DMA_PKG spi_dma_pkg;
static volatile int spi_locked_cs;
//static volatile int spi_io_mode;
static ASAP18XX_SPI * sd_ctrl;	

static ASAP18XX_SPI *  ASAP18XX_GetBase_SPI(void)
{
	return (ASAP18XX_SPI * )ASAP18XX_SPI_BASE;
}




int spi_flash_init(int io_mode,int spi_clk_div, int internal_clk_div){

    sd_ctrl = ASAP18XX_GetBase_SPI();

	//as_puts("SPI-Flash init.\n");

    // spi clock uses control's ssp clock, 
    // div by 6, set the same with clk_init();
    outl(spi_clk_div,SSP_CLKCTRL);
   
    outl((1<<30) ,HW_APBH_CTRL0 + 8);/* clear apbh reset & clock gate */ 
    outl((1<<31) ,HW_APBH_CTRL0 + 8);/* clear apbh reset & clock gate */ 

    #define SPI_DMA_CH 5
    /* apbh channel X clear clockgate and reset */ 
    outl((1<<(SPI_DMA_CH + 8)) ,HW_APBH_CTRL0 + 8);
    outl((1<<(SPI_DMA_CH + 16)) ,HW_APBH_CTRL0 + 4);


    sd_ctrl->ctrl0[1] = 0x80000000;    //soft reset
    sd_ctrl->ctrl0[2] = 0x40000000;    // clear reset
    sd_ctrl->ctrl0[2] = 0x80000000;    // clear reset

   // if (io_mode == MODE_BACKUP_1)
   // {
        //Pin_assign_dev(LOCATION_SPI_BACKUP);
   //     spi_io_mode = (1<<16);  // Use NAND-D[7-5]
   // }
   // else
   // {
        //Pin_assign_dev(LOCATION_SPI);
    //    spi_io_mode = (0<<16);   // Use Default IO
   // }

    sd_ctrl->ctrl1[0] = 0x00002038;   // SPI mode select,8 bit wordlength    

    sd_ctrl->timing[0] =0xffff0000|(internal_clk_div<<8); 
    sd_ctrl->ctrl1[1]=0; // enable timing setting
    return 0;
}


static void spi_lock_cs(void){
    spi_locked_cs = 1;
}

static void spi_unlock_cs(void){
    spi_locked_cs = 0;
    sd_ctrl->ctrl0[2] =(1<<27);
}

#define MAX_BUSY_WAIT 0x00050000

static int start_spi_dma_routine(AS3310_DMA_PKG * pkg)
{
    int wait_dma;
    int err;

    err = 0;
    
    outl((ulong)&pkg->NEXT_PKG , HW_APBH_CH5_NXTCMDAR);
    outl(1 ,HW_APBH_CH5_SEMA);

    //mem_display(&pkg->NEXT_PKG,0x20);

    wait_dma = 0;
     //spi_debug("start_spi_dma_routine1\n");
    while (!IS_DMA_APBH_CH5_COMPLETE()) 
    {
        if (wait_dma++ > MAX_BUSY_WAIT) 
        {            
            err = 0x00000006;//ERROR_CODE_SPI_DMA_TO;
            //report_error(ERROR_CODE_SPI_DMA_TO,0);
            //as_puts("DMA Not Ready.\n"); 
            puts("DMA Not Ready.\n");
            break;
        }
    }
   // spi_debug("start_spi_dma_routine2\n");
    return err;
}

static int spi_flash_write_buf(u8 * buf, int n){

    spi_dma_pkg.CTRL    = 0x000010c2|(n<<16);     // 0x002010c6;
    spi_dma_pkg.BUFFER  = (ulong)buf;
    spi_dma_pkg.CMD0    = 0x11000000|n|(spi_locked_cs<<27)|0;
    //spi_debug("spi_flash_write_buf\n");
    return start_spi_dma_routine(&spi_dma_pkg);
}

static int spi_flash_read_buf(u8 * buf, int n){

    spi_dma_pkg.CTRL    = 0x000010c1|(n<<16);
    spi_dma_pkg.BUFFER  = (ulong)buf;
    spi_dma_pkg.CMD0    = 0x17000000|n|(spi_locked_cs<<27)|0;
    return start_spi_dma_routine(&spi_dma_pkg);
}


/*  ========================================= */
/*  ========================================= */

static u8 NCached spi_cmd[4];

static int spi_flash_send_cmd(int cmd){

    spi_cmd[0] = cmd;
    return spi_flash_write_buf(spi_cmd,1);
}

static u8 NCached spi_addr[4];

static int spi_flash_send_addr(u32 faddr){

  
    spi_addr[0] = (u8)(faddr>>16);
    spi_addr[1] = (u8)((faddr>>8) & 0xff);
    spi_addr[2] = (u8)(faddr & 0xff);
   
    return spi_flash_write_buf(spi_addr,3);
}


static u8 NCached spi_flash_status[4];
static void spi_flash_read_status(char * status){

    spi_lock_cs();
    spi_flash_send_cmd(SPI_FLASH_CMD_READ_STATUS);
    spi_flash_read_buf(spi_flash_status,1);
    * status = spi_flash_status[0];
    spi_unlock_cs();

    return ;
}

void spi_delay(int n){
    int i;
    while (n-->0) {
        for(i=0;i<1000;i++);
    }
}

int spi_flash_read(u8 * buf, u32 faddr, int count){

    spi_debug("SPI Read : 0x");spi_puth(faddr);
    spi_debug("\tlen 0x");spi_puth(count);
    spi_debug("\n");
    spi_lock_cs();

   // spi_delay(1000);
   // spi_debug("spi_flash_send_cmd...\n");
    spi_flash_send_cmd(SPI_FLASH_CMD_READ);
    spi_delay(1000);
    //spi_debug("spi_flash_send_addr...\n");
    spi_flash_send_addr(faddr);
    spi_delay(1000);
    //spi_debug("spi_flash_read_buf...\n");
    spi_flash_read_buf(buf,count);
   // spi_delay(1000);
    //spi_debug("spi_flash_read finish\n");
    spi_unlock_cs();

    return count;
}










