/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader DMA Source file

------------------- Version 1.0  ----------------------
Create File
 He Yong 2007-03-11
 zhangbo 2007-21-08 transplant to alpos
*/

#include <common.h>
//#include <kernel/dma.h>



 /*
Start APBX DMA Chain
inputs:
ulong pkg_addr, Physical address of DMA chain entry
int pkg_num,    number of DMA package in this DMA chain
int ch_num,     channel number
*/
int dma_start_apbx(ulong pkg_addr,int pkg_num,int ch_num){
    int ret;

     if ((ch_num < DMA_CHANNEL_MIN)||(ch_num > DMA_CHANNEL_MAX))  {
         printf("APBX DMA Channel Error!\n");
         return -AS3310_DMA_CHANNEL_INVALID;
     }
     else {
         if (is_apbx_complete(ch_num)) {
             ulong reg_next_addr = HW_APBX_CH0_NXTCMDAR + 
                 (ch_num)*(HW_APBX_CH1_NXTCMDAR - HW_APBX_CH0_NXTCMDAR);

             outl((ulong)pkg_addr ,reg_next_addr );
             outl(pkg_num ,reg_next_addr+(HW_APBX_CH0_SEMA-HW_APBX_CH0_NXTCMDAR));
             ret = AS3310_DMA_CHANNEL_OK;
         }
         else{
        //     printf("APBX DMA Ch(%d) Busy!\n",ch_num);
             ret = -AS3310_DMA_CHANNEL_BUSY;
         }
         return ret;
     }
 }


 /*
Start APBH DMA Chain
inputs:
ulong pkg_addr, Physical address of DMA chain entry
int pkg_num,    number of DMA package in this DMA chain
int ch_num,     channel number
*/
int dma_start_apbh(ulong pkg_addr,int pkg_num,int ch_num){
    int ret;

     if ((ch_num < DMA_CHANNEL_MIN)||(ch_num > DMA_CHANNEL_MAX))  {
         printf("APBH DMA Channel Error!\n");
         return -AS3310_DMA_CHANNEL_INVALID;
     }
     else {
         if (is_apbh_complete(ch_num)) {
          //   printf("APBH DMA Channel (%d) complete!\n",ch_num);
             ulong reg_next_addr = HW_APBH_CH0_NXTCMDAR + (ch_num*0x70);

             outl((ulong)pkg_addr ,reg_next_addr );
             outl(pkg_num ,reg_next_addr+ 0x30);
             ret = AS3310_DMA_CHANNEL_OK;
         }
         else{
         //    printf("APBH DMA Ch(%d) Busy!\n",ch_num);
             ret = -AS3310_DMA_CHANNEL_BUSY;
         }
         return ret;
     }
 }

/*int dma_apbh_reset_ch(int ch_num)
{
     ulong reg_next_addr = HW_APBH_CH0_NXTCMDAR +
              (ch_num)*(HW_APBH_CH1_NXTCMDAR - HW_APBH_CH0_NXTCMDAR);
     outl(0x0,reg_next_addr+(HW_APBH_CH0_SEMA-HW_APBH_CH0_NXTCMDAR));
     outl((1<<(ch_num+16)),HW_APBH_CTRL0_SET);
  
    return 0;
}
*/

 /*
init_as3310_dma_chain
inputs:
struct device * dev the device which use this dma chain
int pkg_num,        number of DMA package in this DMA chain
int channel_num,    channel number
*/
struct as3310_dma_chain * request_as3310_dma_chain(int pkg_num,int channel_num){
    struct as3310_dma_chain * dma_chain;
    struct as3310_dma_pkg_s * chain_ptr;
    int i;

    if ((channel_num < DMA_CHANNEL_MIN)||(channel_num > DMA_CHANNEL_MAX))  {
        return NULL;
    }

	if (!(dma_chain = c_malloc(sizeof(struct as3310_dma_chain)))) {
		printf("DMA: Fail,No system memory\n");
        return NULL;
	}
    //printf("dma_chain:0x%x\n",dma_chain);
    if (!(dma_chain->chain_phy_addr = nc_malloc(sizeof(struct as3310_dma_pkg_s)*pkg_num))){
		printf("DMA: Fail,No dma memory\n");
        return NULL;
    }
    //printf("dma_chain->chain_phy_addr:0x%x\n",dma_chain->chain_phy_addr);
    dma_chain->chain_head = (struct as3310_dma_pkg_s *)dma_chain->chain_phy_addr;

    chain_ptr = dma_chain->chain_head;
    dma_chain->channel_num = channel_num;
    dma_chain->pkg_num = pkg_num;
    dma_chain->status = AS3310_DMA_STAT_READY;

    /* init chain connections */
    for (i = 1; i<pkg_num ; i++) {
        chain_ptr->NEXT_PKG = (ulong)dma_chain->chain_phy_addr + (i*sizeof(struct as3310_dma_pkg_s));
        chain_ptr++;
    }

    return dma_chain;
}


 /*
free_as3310_dma_chain
inputs:
struct device * dev the device which use this dma chain
dma_chain,          dma_chain struct which need free
*/
void free_as3310_dma_chain(struct as3310_dma_chain * dma_chain){

    asfree(dma_chain->chain_phy_addr);
    asfree(dma_chain);
}


int dma_apbh_init(){
    outl(0x40000000,HW_APBH_CTRL0_CLR);//clear the clk gate
    outl(0x80000000,HW_APBH_CTRL0_CLR);//clear the clk gate     
return 0;
}


int dma_apbx_init(){
    outl(0x40000000,HW_APBX_CTRL0_CLR);//clear the clk gate
    outl(0x80000000,HW_APBX_CTRL0_CLR);//clear the clk gate     
return 0;
}



#if CONFIG_LCD_CONTROL


int dma_start_lcd_apbh(ulong pkg_addr,int pkg_num,int ch_num){

         ulong reg_next_addr = HW_APBH_LCD_CH0_NXTCMDAR + 
             (ch_num)*(HW_APBH_LCD_CH1_NXTCMDAR - HW_APBH_LCD_CH0_NXTCMDAR);

         outl((ulong)pkg_addr ,reg_next_addr );
         outl(pkg_num ,reg_next_addr+(HW_APBH_LCD_CH0_SEMA-HW_APBH_LCD_CH0_NXTCMDAR));

         return pkg_num;
}

int dma_lcd_init(){
    outl(0x40000000,HW_APBH_LCD_CTRL0_CLR);//clear the clk gate
    outl(0x80000000,HW_APBH_LCD_CTRL0_CLR);//clear the clk gate     
return 0;

}
#endif
