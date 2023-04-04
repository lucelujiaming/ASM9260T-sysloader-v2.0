


#include <common.h>
#include <drivers/sd.h>



void RXWriteCmdPkg()
{
	int *memaddr;
	memaddr=(int*)DMA_CMD_PKG_ADDR1;
	*memaddr=0x0;
	memaddr+=1;
	*memaddr=0x020020c1;           //00000000000100000000000011000010
	memaddr+=1;
	*memaddr=RX_BASE_ADDR;
	memaddr+=1;
#if _1BIT_
	*memaddr=0x07030200; // 1 bit
#endif
#if _4BIT_
    *memaddr=0x07430200; // 4 bit
#endif
	memaddr+=1;
	*memaddr=0x00000011;
	
	
}

void TXWriteCmdPkg()
{
	int *memaddr;
	memaddr=(int*)DMA_CMD_PKG_ADDR3;
	*memaddr=0x0;
	memaddr+=1;
	*memaddr=0x020020c2;           //00000000000100000000000011000010
	memaddr+=1;
	*memaddr=TX_BASE_ADDR;
	memaddr+=1;
#if _1BIT_
	*memaddr=0x05030200;// 1 bit
#endif
#if _4BIT_
    *memaddr=0x05430200;// 4 bit
#endif 
	memaddr+=1;
	*memaddr=0x00000018;
	
	
}

void RXDMAStart()
{
	outl(0x00000000,DMA_CTRL0);
	
	outl(DMA_CMD_PKG_ADDR1,CH4_NXTCMDAR);
	outl(0x00000001,CH4_SEMA);
}

void TXDMAStart()
{
	outl(0x00000000,DMA_CTRL0);
	
	outl(DMA_CMD_PKG_ADDR3,CH4_NXTCMDAR);
	outl(0x00000001,CH4_SEMA);
}

int initial_ssp(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
           
       ssp_reg * sd_ctrl;
        AS3310_REG32 card_addr;

        sd_ctrl = (ssp_reg * const)SSP_CTRL0; 

        sd_ctrl->ctrl0[1] = 0x80000000;    //soft reset
        sd_ctrl->ctrl0[2] = 0xc0000000;    // clear gate
        
        outl(0x0003ffff,SSP_PIN);
        outl(0x7f000000,SSP_8MA);
        outl(0x00000001,SSP_CLKCTRL);

        sd_ctrl->ctrl0[2] = 0xc0000000;
        sd_ctrl->ctrl1[0] = 0x00002073;//falling
              
        //send CMD0

        sd_ctrl->timing[0] = 0xfffff000;   //400k for CMD0
        sd_ctrl->cmd0[0] = 0x0;
        sd_ctrl->ctrl0[0] = 0x264301ff;

        printf("cmd0 sent. no resp.\n");

        send_acmd41();
        printf("acmd41 sent.  resp is %x.\n",inl(sd_ctrl->sdresp0));

        send_cmd(2,0);
        delay(1000);  //wait for resp
        printf("cmd2 sent.  resp is %x.\n",inl(sd_ctrl->sdresp0));

        while ((inl(sd_ctrl->sdresp0)&0x0000ffff) != 0x00000700) {
            send_cmd(3,0);
            delay(1000);   //wait fot resp
            card_addr = inl(SSP_SDRESP0);
            printf("cmd3 sent.  resp is %x.\n",card_addr);
            
        }

        card_addr = card_addr & 0xffff0000;

        send_cmd(7,card_addr);
        printf("cmd7 sent.  arg is %x.\n",card_addr);

#if _4BIT_
        send_cmd(55,card_addr);
        delay(1000);  //wait fot resp
        send_cmd(6,2);
        printf("swicth to 4 bit\n");
#endif

        send_cmd(16,0x200);  // block len
        delay(1000);  //wait for resp
        printf("cmd16 sent.  resp is %x.\n",inl(sd_ctrl->sdresp0));

        printf("SD card initialled successfully!\n");

     return 0;
}

//void send_acmd41(cmd_tbl_t *cmdtp,int argc,char * argv[]){
void send_acmd41(){
	
        int end0;
        int end1;
     
        ssp_reg * sd_ctrl;

        sd_ctrl = (ssp_reg * const)SSP_CTRL0; 

        while ((inl(sd_ctrl->sdresp0))!=0x80ff8000) {
                    
        
        end0 = inl(SSP_STATUS);
     //   puts("send acmd to sd card.\n");

        //send 55
        send_cmd(0x37,0);
     
       
        end1 = inl(SSP_STATUS);
        while((end1&0x00040000) == (end0&0x00040000)){
        	end1 = inl(SSP_STATUS);
        }
        
        //send 41
        send_cmd(0x29,0x00ff8000);
        delay(5000);  //wait fot resp

    
        }
        
       
       

}




int send_sd_cmd(cmd_tbl_t *cmdtp,int argc,char * argv[])
{

    ulong cmd0;
	ulong cmd1;
  //  ulong time;
	cmd0 = TextToLong_TRL(argv[1]);
	cmd1 = TextToLong_TRL(argv[2]);

    send_cmd(cmd0,cmd1);
return 0;
}


void send_cmd(ulong cmd0,ulong cmd1)
{

	ssp_reg * sd_ctrl;

        sd_ctrl = (ssp_reg * const)SSP_CTRL0; 


        sd_ctrl->ctrl0[1] = 0x80000000;
        sd_ctrl->ctrl0[2] = 0xc0000000;
        sd_ctrl->ctrl1[0] = 0x00002073;
        sd_ctrl->timing[0] = 0xfffff000;   //200k for CMD1
	
	   sd_ctrl->cmd0[0] = cmd0;
       sd_ctrl->cmd1[0] = cmd1;

      
        if (cmd0==9){
           outl(0x264b01ff,SSP_CTRL0);
        }else{
           outl(0x264301ff,SSP_CTRL0);
        }
        
              
}


int sd_read(cmd_tbl_t *cmdtp,int argc,char * argv[]){
	
	ulong addr;
	addr = TextToLong_TRL(argv[1]);

    ssp_reg * sd_ctrl;

    sd_ctrl = (ssp_reg * const)SSP_CTRL0; 
	
	//puts("start sd read.\n");
	
  //  TXWriteCmdPkg();   
        RXWriteCmdPkg();  
        
        outl(addr,0x40009008);
    //    outl(0xc003ffff,SSP_PIN);
        outl(0xff000000,SSP_8MA);
        
       sd_ctrl->ctrl0[1] = 0x80000000;
       sd_ctrl->ctrl0[2] = 0xc0000000;
       sd_ctrl->ctrl1[0] = 0x00002073;
       sd_ctrl->timing[0] = 0xfffff000;

   
        
        RXDMAStart();
        
        puts("read finished.\n");

        return 0;

}


int sd_write(cmd_tbl_t *cmdtp,int argc,char * argv[]){
	
	ulong addr;
	addr = TextToLong_TRL(argv[1]);

    ssp_reg * sd_ctrl;
    sd_ctrl = (ssp_reg * const)SSP_CTRL0; 
	
	//puts("start sd read.\n");
	
	TXWriteCmdPkg();   
      //  RXWriteCmdPkg();  
        
        outl(addr,0x40008108);
        outl(0xf7000000,SSP_8MA);

        sd_ctrl->ctrl0[1] = 0x80000000;
       sd_ctrl->ctrl0[2] = 0xc0000000;
       sd_ctrl->ctrl1[0] = 0x00002673;
       sd_ctrl->timing[0] = 0xfffff000;
    
        
        TXDMAStart();
        
        puts("write finished.\n");
   return 0;
}



BOOT_CMD(initsd,initial_ssp,
         " #sd card initial ",
         "initial SD card");
/*         
BOOT_CMD(acmd41,send_acmd41,
         " send acmd41 to sd card ",
         "acmd41 ");
*/         
         
BOOT_CMD(sdcmd,send_sd_cmd,
         " #send sd commond ",
         "sdcmd #cmd0 #cmd1");      
         
BOOT_CMD(sdr,sd_read,
         " #read data from sd card ",
         "sdr #addr");  
         
BOOT_CMD(sdw,sd_write,
         " #write data to sd card ",
         "sdw #addr");                 
