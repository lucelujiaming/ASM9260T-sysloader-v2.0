#include <common.h>
#include <drivers/sd_mmc.h>

#define SSP_DMA_PKG_NUM 8

sd_mmc_card card;
AS3310_DMA_PKG NCached ssp_dma_pkt[SSP_DMA_PKG_NUM];
volatile char NCached tmp_buf[4096];

void SSP_dma_trans(char dir, int block_num, int block_size, void *buf){
	int i;
	// build dma chain
	for (i=0;i<block_num-1;i++) {
		ssp_dma_pkt[i].NEXT_PKG = (int)&ssp_dma_pkt[i+1];
		ssp_dma_pkt[i].BUFFER = (int)(buf+block_size*i);
		if (block_num == 1) {
			ssp_dma_pkt[i].CTRL = 0x000010c0+(block_size<<16)+(2>>dir);   // no chain
		}else{
			ssp_dma_pkt[i].CTRL = 0x000010c4+(block_size<<16)+(2>>dir);
		}
        ssp_dma_pkt[i].CMD0 = 0x05420200|(dir<<25); //4 bit
	}
	// last package, send stop cmd
    ssp_dma_pkt[block_num-1].BUFFER = (int)(buf + block_size*(block_num-1));
	if (block_num == 1) {
	    ssp_dma_pkt[block_num-1].CTRL = 0x000010c0+(block_size<<16)+(2>>dir);   // no chain
		ssp_dma_pkt[block_num-1].CMD0 = 0x05420200|(dir<<25); //4 bit, send no cmd
	}else{
		ssp_dma_pkt[block_num-1].CTRL = 0x000020c0+(block_size<<16)+(2>>dir);   // no chain
		ssp_dma_pkt[block_num-1].CMD0 = 0x05720200|(dir<<25)|(1<<16); //4 bit, send cmd
		ssp_dma_pkt[block_num-1].CMD1 = 0xc; //stop cmd
	}
	// first package, send cmd
	ssp_dma_pkt[0].CMD0 |= (1<<16);
	
    dma_start_apbh((int)&ssp_dma_pkt[0],block_num,1);
 //   printf("package addr is %x block_num is %d\n",(int)&ssp_dma_pkt[0],block_num);
    while(!IS_DMA_APBH_CH1_COMPLETE()){}; 
}

void SSP_pin_assign(){
    int i;
    /*     Pin Assign      */
    for (i = SSP_PAD_PIN_LOW; i <= SSP_PAD_PIN_HIGH;i++) {
        request_as3310_gpio(SSP_PAD_BANK,i,SSP_PAD_TYPE);
    }
	// set ssp detect pin as gpio
	request_as3310_gpio(SSP_PAD_BANK,SSP_DETECT_PIN,PIN_FUNCTION_GPIO);
}

void SSP_controller_init(){
	 AS3310_SSP * sd_ctrl;
	 sd_ctrl = AS3310_GetBase_SSP();

	 sd_ctrl->ctrl0[1] = 0x80000000;    //soft reset
	 sd_ctrl->ctrl0[2] = 0xc0000000;    // clear gate
	 // SD mode select
	 sd_ctrl->ctrl1[0] = 0x04002473;//falling
	 sd_ctrl->timing[0] = 0xfffff000;   //375k for init
}

void ssp_detect_irq_init(){
	irq_action_t ssp_detect_irq;
	int ret;

	ssp_detect_irq.irq = INT_AS3310_GPIO0;
	ssp_detect_irq.irq_handler = sd_mmc_detect_irq;
	ssp_detect_irq.clear = clear_detect_pin_irq;
	ssp_detect_irq.priv_data = INT_AS3310_GPIO0;
	ret = request_irq(&ssp_detect_irq); 
    if (ret) {
        printf("SSP Detect IRQ %d Error\n",INT_AS3310_GPIO0);
    }
	io_irq_enable_edge(SSP_PAD_BANK,SSP_DETECT_PIN,GPIO_IRQ_EDGE_FALLING);
}

void ssp_irq_init(){
	irq_action_t ssp_irq;
	int ret;

	ssp_irq.irq = INT_AS3310_SPI;
    ssp_irq.irq_handler = ssp_irq_handler;
	ssp_irq.clear = NULL;
	ssp_irq.priv_data = INT_AS3310_SPI;
	ret = request_irq(&ssp_irq); 
    if (ret) {
        printf("SSP IRQ %d Error\n",INT_AS3310_SPI);
    }
}

int sd_mmc_probe(){
	int err;
	SSP_pin_assign();
	SSP_controller_init();
    ssp_detect_irq_init();
	ssp_irq_init();
	err = sd_mmc_card_init();
    return err;
}

void sd_mmc_read(int sd_addr,void * buf,int count){
	int block_num,block_size;
	int cp_len;
 //   char tmp_buf[4096];
    int tmp_sd_addr;
	d_offs tmp_ofs;
  //  AS3310_SSP * sd_ctrl;
	sd_mmc_commond cmd;
	sd_mmc_resp resp;
	AS3310_SSP * sd_ctrl;
	sd_ctrl = AS3310_GetBase_SSP();

	// handle the sd addr
    tmp_sd_addr = sd_addr;
	tmp_ofs = sd_addr&0x1ff;
	
    if ((sd_addr&0x1ff)!=0) { //ofs is not block size align
	   tmp_sd_addr = sd_addr & 0xfffffe00;
	   count += sd_addr&0x1ff;
	}

  //  /***************************/
  //  block_num = (count>>9);
  //  block_size = 0x200;
  //  cp_len = count;
  //  /***************************/

      // calc the block number, block size is always 0x200
      while (count>0) {
      	if (count>4096) {   // if read length is greater than 4096, split it 
      		block_num = 8;
      		block_size = 0x200;
      		cp_len = 4096;
      		count -= 4096;
      	}else {
      	    if ((count&0x1ff)==0) {   // count is block size align
      			block_num = (count>>9);
      		}else{
      			block_num = (count>>9)+1;
      		}
      		block_size = 0x200;
    
      		// handle the copy length
      		if ((count-tmp_ofs)<=tmp_ofs) {
      			cp_len = count-tmp_ofs;
      		}else{
      			cp_len = count;
      		}
      		count -= (block_num<<9);
      	}

		// send read commonds
		if (block_num==1) {
			cmd.opcode = MMC_READ_SINGLE_BLOCK;
		}else{
			cmd.opcode = MMC_READ_MULTIPLE_BLOCK;
		}
		cmd.argument = tmp_sd_addr;
		cmd.flag = 0;
    //    printf("ssp read, blk = %d, blksz = %d, ofs = %x\n",block_num,block_size,cmd.argument);

		//resp = send_commond(cmd);
		sd_ctrl->cmd0[0] = cmd.opcode;
		sd_ctrl->cmd1[0] = cmd.argument;

		// read data using DMA
		SSP_dma_trans(READ,block_num,block_size,tmp_buf);
	    //while((sd_ctrl->status[0]&0x1) != 0){};  //CMD_BUSY   

		// copy data from temp buffer to user buffer
	//	printf("copy 0x%x to 0x%x 0x%xbytes\n",(int)(tmp_buf+tmp_ofs),(int)buf,cp_len);
		memcpy(buf, (tmp_buf+tmp_ofs), cp_len);
        buf += cp_len; 
   }
}

int sd_read_cmd(cmd_tbl_t *cmdtp,int argc,char * argv[]){
	
    ulong sd_addr;
	ulong buf_addr;
	int len;

	if (argc < 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

	sd_addr = TextToLong_TRL(argv[1]);
	buf_addr = TextToLong_TRL(argv[2]);
	len = TextToLong_TRL(argv[3]);

  //  static char buffer[0x200*PKG_NUM];
  //printf("sd addr is 0x%x\n",sd_addr);
    sd_mmc_read(sd_addr,buf_addr,len);
   // printf("data stored at 0x%x\n",&buffer[0]);

        return 0;

}

void sd_mmc_write(int sd_addr,void * buf,int count){
	int block_num,block_size;
	int cp_len;
 //   char tmp_buf[4096];
	d_offs tmp_ofs;
	sd_mmc_commond cmd;
	sd_mmc_resp resp;
	AS3310_SSP * sd_ctrl;
	sd_ctrl = AS3310_GetBase_SSP();

   /***************************/
	tmp_ofs = sd_addr;
	cp_len = count;
	block_num = count>>9;
	block_size = 0x200;
   /***************************/
 //   printf("copy %x to %x\n",(int)buf,(int)(tmp_buf));

	memcpy(tmp_buf, buf, cp_len);

	// send write commonds
	if (block_num==1) {
		cmd.opcode = MMC_WRITE_SINGLE_BLOCK;
	}else{
		cmd.opcode = MMC_WRITE_MULTIPLE_BLOCK;
	}
	cmd.argument = tmp_ofs;
	cmd.flag = 0;

	sd_ctrl->cmd0[0] = cmd.opcode;
	sd_ctrl->cmd1[0] = cmd.argument;

 //   printf("ssp write, blk = %d, blksz = %d, ofs = %x\n",block_num,block_size,cmd.argument);

  //  resp = send_commond(cmd);

	// write data using DMA
	SSP_dma_trans(WRITE,block_num,block_size,tmp_buf);
  //  while((sd_ctrl->status[0]&0x1) != 0){};  //CMD_BUSY   

	// copy data from temp buffer to user buffer
   // memcpy(buf, tmp_buf, cp_len);

     
}


int sd_write_cmd(cmd_tbl_t *cmdtp,int argc,char * argv[]){
	
    ulong sd_addr;
	ulong buf_addr;
	int len;

	if (argc < 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

	sd_addr = TextToLong_TRL(argv[1]);
	buf_addr = TextToLong_TRL(argv[2]);
	len = TextToLong_TRL(argv[3]);

  //  static char buffer[0x200*PKG_NUM];
    sd_mmc_write(sd_addr,buf_addr,len);
   // printf("data stored at 0x%x\n",&buffer[0]);

        return 0;

}



sd_mmc_resp send_commond(sd_mmc_commond cmd){
	AS3310_SSP * sd_ctrl;
	sd_mmc_resp resp;
	u32 ssp_ctrl0;

	sd_ctrl = AS3310_GetBase_SSP();

   // sd_ctrl->ctrl1[0] = 0x04002473;

    if (cmd.flag == APP_CMD) {    // send app cmd 55
		sd_ctrl->cmd0[0] = MMC_APP_CMD;
		sd_ctrl->cmd1[0] = card.relative_addr;
	   // printf("[Opcode]:%x [ARGU]:%x ",sd_ctrl->cmd0[0],sd_ctrl->cmd1[0]);
		sd_ctrl->ctrl0[0] = 0x266301ff;
		delay(10);
		while((sd_ctrl->status[0]&0x1) != 0){};  //CMD_BUSY   
	 //   printf("[Resp]:%x\n",sd_ctrl->sdresp0[0]);

	}

	// send commond
	sd_ctrl->cmd0[0] = cmd.opcode;
	sd_ctrl->cmd1[0] = cmd.argument;

	ssp_ctrl0 = 0x066301ff;
    if (cmd.flag == LONG_RESP) {
		ssp_ctrl0 |= LONG_RESPONSE;
	}
	if(cmd.flag == NO_RESP){
		ssp_ctrl0 &= (~GET_RESPONSE);
	}
  //  printf("[Opcode]:%x [ARGU]:%x ctrl0=%x ",sd_ctrl->cmd0[0],sd_ctrl->cmd1[0],ssp_ctrl0);

	ssp_ctrl0 |= 0x20000000;
	sd_ctrl->ctrl0[0] = ssp_ctrl0;
    delay(10);

	while((sd_ctrl->status[0]&0x1) != 0){};  //CMD_BUSY   

	// get response
    resp.resp[0] = sd_ctrl->sdresp0[0];
    resp.resp[1] = sd_ctrl->sdresp1[0];
    resp.resp[2] = sd_ctrl->sdresp2[0];
    resp.resp[3] = sd_ctrl->sdresp3[0];

	//printf("[Resp0]:%x [Resp1]:%x [Resp2]:%x [Resp3]:%x\n",sd_ctrl->sdresp0[0],sd_ctrl->sdresp1[0],sd_ctrl->sdresp2[0],sd_ctrl->sdresp3[0]);

	return resp;
}

int sd_mmc_card_init(){
    int err;
	card.card_type = NO_CARD;
	card.relative_addr = 0;
	card.ocr = 0x00100000;
	go_idle();
	go_idle();
	//mmc_send_if_cond();
	err = sd_send_app_op_cond();
	if (err == 0) {   // ACMD41 send successful
		card.card_type = SD_CARD;
		printf("Detect a SD Card.\n");
	}else if (err == TIMEOUT) { // ACMD41 time out, try to find a MMC card
	   // printf("ACMD41 timeout.\n");
		err = mmc_send_op_cond(); // send CMD1
		if (err == 0) {
			card.card_type = MMC_CARD;
			printf("Detect a MMC Card.\n");
		}
		if (err == TIMEOUT) {
	  //  	printf("CMD1 timeout.\n");
			printf("No Card Detect.\n");
			return err;
		}
	}
	all_send_cid();
	err = send_relative_addr();
	if (err == TIMEOUT) {
		printf("send relative address time out.\n");
		return err;
	}
	decode_cid();
	printf("Product Name: %c%c%c%c%c.  ",card.prod_name[0],card.prod_name[1],card.prod_name[2],card.prod_name[3],card.prod_name[4]);
	send_csd();
	decode_csd();
	printf("Capacity: %dKBytes\n",card.capacity>>1); //card.capacity means Block numbers(512bytes/Block)
	select_card();
	//mmc_app_send_scr();
	set_clock();
	if (card.card_type == SD_CARD) {
		app_set_bus_width();
	}else if (card.card_type == MMC_CARD) {
		mmc_switch(EXT_CSD_CMD_SET_NORMAL,EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_4);
	}
	set_block_length();
	printf("SD/MMC card init ok.\n");
	return 0;
}

void go_idle(){
	sd_mmc_commond cmd;

	cmd.opcode = MMC_GO_IDLE_STATE;
	cmd.argument = 0;
	cmd.flag = NO_RESP;

	send_commond(cmd);
}

void mmc_send_if_cond(){
}

int sd_send_app_op_cond(){
    int i;
	sd_mmc_commond cmd;
	sd_mmc_resp    resp; 
    
	cmd.opcode = SD_APP_OP_COND;
	cmd.argument = card.ocr;
	cmd.flag = APP_CMD;

	for (i=0;i<1000;i++) {   // try 100 times
		resp = send_commond(cmd);
	   // printf("resp is %x\n",resp.resp[0]);
		if (resp.resp[0]&MMC_CARD_BUSY) {
			return 0;
		}
	}
	return TIMEOUT;
}

int mmc_send_op_cond(){
    int i;
	sd_mmc_commond cmd;
	sd_mmc_resp    resp; 
	u32 ocr;

	ocr = 0;
    
	cmd.opcode = MMC_SEND_OP_COND;
 //   cmd.argument = 0;
	cmd.flag = 0;

	for (i=0;i<1000;i++) {   // try 100 times
		cmd.argument = ocr;
		resp = send_commond(cmd);
		ocr = resp.resp[0];
	   // printf("resp is %x\n",resp.resp[0]);
		if (resp.resp[0]&MMC_CARD_BUSY) {
			return 0;
		}
	}
	return TIMEOUT;
}

void all_send_cid(){
	sd_mmc_commond cmd;
	sd_mmc_resp    resp; 

	cmd.opcode = MMC_ALL_SEND_CID;
	cmd.argument = 0;
	cmd.flag = LONG_RESP;

	resp = send_commond(cmd);
	card.cid[0] = resp.resp[3];
	card.cid[1] = resp.resp[2];
	card.cid[2] = resp.resp[1];
	card.cid[3] = resp.resp[0];
}

int send_relative_addr(){
	int i;
	sd_mmc_commond cmd;
	sd_mmc_resp    resp; 

	cmd.opcode = MMC_SET_RELATIVE_ADDR;
	cmd.argument = 0;
	cmd.flag = 0;

	for (i=10;i>0;i--) {  // try 10 times
		resp = send_commond(cmd);
	   // printf("resp is %x\n",resp.resp[0]);

		if (card.card_type == SD_CARD) {
			if ((resp.resp[0]&0xffff) == 0x700) {
				card.relative_addr = resp.resp[0]&0xffff0000;
				return 0;
			}
		}
		if (card.card_type == MMC_CARD) {
			if ((resp.resp[0]&0xffff) == 0x500) {
				card.relative_addr = resp.resp[0]&0xffff0000;
				return 0;
			}
		}
	}
	return TIMEOUT;
}

void send_csd(){
	sd_mmc_commond cmd;
	sd_mmc_resp    resp; 

	cmd.opcode = MMC_SEND_CSD;
	cmd.argument = card.relative_addr;
	cmd.flag = LONG_RESP;

	resp = send_commond(cmd);
	//memcpy(card.csd, resp.resp, 4);
	card.csd[0]=resp.resp[3];
	card.csd[1]=resp.resp[2];
	card.csd[2]=resp.resp[1];
	card.csd[3]=resp.resp[0];
}

void select_card(){
	sd_mmc_commond cmd;

	cmd.opcode = MMC_SELECT_CARD;
	cmd.argument = card.relative_addr;
	cmd.flag = 0;

	send_commond(cmd);
}

void mmc_app_send_scr(){
}

void app_set_bus_width(){
	sd_mmc_commond cmd;

	cmd.opcode = SD_APP_SET_BUS_WIDTH;
	cmd.argument = 2;
	cmd.flag = APP_CMD;

	send_commond(cmd);
}

void mmc_switch(u8 set, u8 index, u8 value){
	sd_mmc_commond cmd;

	cmd.opcode = MMC_SWITCH;
	cmd.argument = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		  (index << 16) |
		  (value << 8) |
		  set;
	cmd.flag = 0;

	send_commond(cmd);
}

void set_block_length(){
	sd_mmc_commond cmd;

	cmd.opcode = MMC_SET_BLOCKLEN;
	cmd.argument = 0x200;
	cmd.flag = 0;

	send_commond(cmd);
}

void set_clock(){
    char div, rate;
	int sd_clk;
	AS3310_SSP * sd_ctrl;
	sd_ctrl = AS3310_GetBase_SSP();

	/*****************************************************
	 SD_CLK = SSPCLK/(CLOCK_DIV x (1+CLOCK_RATE))
	 Note: Clock_div must be an even value from 2 to 254.
	******************************************************/
	div = 2;
	rate = 1;

    sd_ctrl->timing[0] = 0xffff0000+(div<<8)+rate;
    sd_clk = 90/(div*(1+rate));
	printf("Set SD/MMC clk to %dMHz.\n",sd_clk);
}

void decode_cid()
{
	u32 *resp = card.cid;

	/*
	 * SD doesn't currently have a version field so we will
	 * have to assume we can parse this.
	 */
	card.prod_name[0]		= UNSTUFF_BITS(resp, 96, 8);
	card.prod_name[1]		= UNSTUFF_BITS(resp, 88, 8);
	card.prod_name[2]		= UNSTUFF_BITS(resp, 80, 8);
	card.prod_name[3]		= UNSTUFF_BITS(resp, 72, 8);
	card.prod_name[4]		= UNSTUFF_BITS(resp, 64, 8);

}


/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
int decode_csd()
{
	unsigned int e, m, csd_struct;
	u32 *resp = card.csd;

	csd_struct = UNSTUFF_BITS(resp, 126, 2);

	if (csd_struct == 0 || card.card_type == MMC_CARD) {

		/********************************************
		 We only care about the capacity of the card
		*********************************************/
		e = UNSTUFF_BITS(resp, 47, 3);
		m = UNSTUFF_BITS(resp, 62, 12);
		card.capacity = (1 + m) << (e + 2);

	}else{
		printf("Do not support such CSD structure version %d\n",csd_struct);
	}

	return 0;
}



BOOT_CMD(initsd,sd_mmc_probe,
         " #sd card initial ",
         "initial SD card");

BOOT_CMD(sdr,sd_read_cmd,
         " #sdr sd_addr buf_addr len",
         "read data from sd card");  

BOOT_CMD(sdw,sd_write_cmd,
         " #sdw sd_addr buf_addr len",
         "write data to sd card");  


int  dev_sd_mmc_read(struct device * dev,d_offs ofs,void * buf,int count){
    int sd_addr,read_cnt;
	sd_addr = ofs<<SECTOR_SHIFT;
	read_cnt = count<<SECTOR_SHIFT;
    sd_mmc_read(sd_addr,buf,read_cnt);
	return 0;
}

int  dev_sd_mmc_write(struct device * dev,d_offs ofs,void * buf,int count){
    int sd_addr,write_cnt;
	sd_addr = ofs<<SECTOR_SHIFT;
	write_cnt = count<<SECTOR_SHIFT;
	//printf("#count = %d\n",count);
    sd_mmc_write(sd_addr,buf,write_cnt);
	return 0;
}

int  dev_sd_mmc_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    switch (cmd) {
	case SD_MMC_IOCTL_GET_SECTOR_NUM:
		return card.capacity;
	case SD_MMC_IOCTL_GET_SECTOR_SHIFT:
		return SECTOR_SHIFT;
	default:
		return 0;
	}
   // return 0;
}

unsigned long  sd_mmc_block_read(int dev, unsigned long start,
		lbaint_t blkcnt, unsigned long *buffer){
	int count,sd_addr;
	count = blkcnt<<9;
	sd_addr = start<<9;
	printf("sd mmc block read.start = %x,sd_addr = 0x%x,count = 0x%x\n",start,sd_addr,count);

    sd_mmc_read(sd_addr,buffer,count);
	return blkcnt;
}



int sd_mmc_detect_irq(){
  //  while (read_GPIO(SSP_PAD_BANK,SSP_DETECT_PIN)){};
	delay(100);
	printf("Detect Card Plug in.\n");
	SSP_controller_init();
	sd_mmc_card_init();
	return 0;
}

int clear_detect_pin_irq(){
	io_irq_clr(SSP_PAD_BANK,SSP_DETECT_PIN);
	return 0;
}

int ssp_irq_handler(){
	AS3310_SSP * sd_ctrl;
	u32 ctrl1,timing;
	sd_ctrl = AS3310_GetBase_SSP();

	ctrl1 = sd_ctrl->ctrl1[0];
	timing = sd_ctrl->timing[0];

    /*****Only RESP TIMEOUT IRQ is enabled*****/ 

 //   printf("SSP IRQ.ctrl1 = 0x%x\n",ctrl1);

  //  if (ctrl1 & (1<<25)) {
	//	printf("Data Time Out.\n");
//	}

	sd_ctrl->ctrl0[1] = 0x80000000;    //soft reset
	sd_ctrl->ctrl0[2] = 0xc0000000;    // clear gate
	// SD mode select
	sd_ctrl->ctrl1[0] = 0x04002473;//falling
	sd_ctrl->timing[0] = timing;   //375k for init

    return 0;
}

struct device dev_sd_mmc = {
    .name       = "sd/mmc",
    .dev_id     = 0,
    .probe      = sd_mmc_probe,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = dev_sd_mmc_read,  
    .write      = dev_sd_mmc_write, 
    .ioctl      = dev_sd_mmc_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = NULL,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device(dev_sd_mmc);

struct blk_dev blk_dev_sd_mmc = {
    .name       = "sd/mmc",
    .block_read = sd_mmc_block_read,  
    .lba        = 0,
    .blksz      = 512,
};

__add_blk_device(blk_dev_sd_mmc);



