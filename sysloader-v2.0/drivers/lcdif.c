#include <common.h>
#include <drivers/lcdif.h>
#include <drivers/Display.h>

#ifdef CONFIG_LCD_240x320

AS3310_DMA_PKG * lcdif_dma_pkg[LCDIF_PKG_NUM];
long Section_SOURCE row_now = 0;
long Section_SOURCE col_now = 0;

int vector_n=0;
void* lcdif_data_buffer;
struct as3310_dma_chain * lcdif_dmachain;

void lcd_delay(int n){
    int i;
    while (n-->0) {
        for(i=0;i<1000;i++);
    }
}


void lcd_write_reg(unsigned short n){
        while ((0x01000000 & inl(HW_LCDIF_CTRL)) ==0 ) 
		{ 
			printf("FIFO is full,HW_LCDIF_CTRL = %x\n",inl(HW_LCDIF_CTRL)); 
		}
        //lcd_delay(1);
        outw(n,HW_LCDIF_DATA);
}


int lcdif_dma_pkg_init()
{
    int i=0;
    lcdif_data_buffer = nc_malloc(FRAME_BUFFER_LEN);
    if (lcdif_data_buffer == NULL) {
          panic("lcdif_dma_pkg_init frame buffer malloc fail\n");
    }
    alp_printf("frame buffer addr:0x%x\n",lcdif_data_buffer);

    lcdif_dmachain = request_as3310_dma_chain(LCDIF_PKG_NUM,AS3310_DMA_LCDIF);
    if (lcdif_dmachain == NULL) {
           panic("lcdif_dma_pkg_init request_as3310_dma_chain fail\n");
    }
    //printf("lcdif_dmachain addr:0x%x\n",lcdif_dmachain->chain_phy_addr);

    for (i=0;i<LCDIF_PKG_NUM;i++) {
        lcdif_dmachain->chain_head[i].CTRL = 0x960010c6;
        lcdif_dmachain->chain_head[i].BUFFER = ((ulong)lcdif_data_buffer + i*PER_BUFFER_LEN);
        lcdif_dmachain->chain_head[i].CMD0 = 0x21154b00;
    }
    lcdif_dmachain->chain_head[LCDIF_PKG_NUM-1].CTRL =  0x960010c2;
    
    return 0;
}


int lcdif_init()
{
    /* turn on back light */
    request_as3310_gpio(LCDIF_BL_PORT,LCDIF_BL_PIN,PIN_FUNCTION_GPIO);
    set_GPIO(LCDIF_BL_PORT,LCDIF_BL_PIN);

    /* Pin-assign */
    outl(0x0,HW_PINCTRL_MUXSEL2);
    outl(0xcff,HW_PINCTRL_MUXSEL3_CLR);

    outl( 0xc0000000,HW_LCDIF_CTRL_CLR); // CLEAR bit 31,30
    outl( 0xc0000000,HW_LCDIF_CTRL_CLR); // CLEAR bit 31,30
    outl( 0x02020202,HW_LCDIF_TIMING); // Timing
    
    lcd_delay(10);
    outl( 0x00100000 ,HW_LCDIF_CTRL_SET); // SET RESET = 1  
    //outl( 0x00080000 ,HW_LCDIF_CTRL_SET); // 6800 mode
	lcd_delay(2);

    outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD
    outl( 0x40000 ,HW_LCDIF_CTRL_CLR); // command mode
    outl( 0xffff ,HW_LCDIF_CTRL_SET); // SET Count 
    outl( 0x10000 ,HW_LCDIF_CTRL_SET); // RUN LCD
    lcd_delay(10);

    PowerOnLcdinit();

    lcdif_dma_pkg_init();

    memset(lcdif_data_buffer,0xee,FRAME_BUFFER_LEN);

    //blank_buff(0xeeeeeeee);
    refresh_dma();

#ifdef CONFIG_ASC_DISPLAY
    lprintf("       **************************");
    lprintf("       =====    Alpscale    =====");
    lprintf("       **************************");
#endif //CONFIG_ASC_DISPLAY


}


void lcdif_dma_start()
{
    int wait_ch_ready;

    wait_ch_ready = 0;
    //printf("lcdif_dmachain->chain_phy_addr:0x%x\n",lcdif_dmachain->chain_phy_addr);
    while( dma_start_apbx((ulong)lcdif_dmachain->chain_phy_addr,LCDIF_PKG_NUM,AS3310_DMA_LCDIF)== -1) {
        if(wait_ch_ready++ > 0x100){
            printf("DMA ch(%d) busy,can't started\n",AS3310_DMA_LCDIF);
            break;
        }
    }
}

void LCD_Command(unsigned int cmd)
{
     outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD 
     outl( 0x40000 ,HW_LCDIF_CTRL_CLR); // SET cmd MODE
     outl( 0x10000 ,HW_LCDIF_CTRL_SET); // run LCD 
    // outl( 0xffff ,HW_LCDIF_CTRL_SET); // SET Count 
    outl( 0x01 ,HW_LCDIF_CTRL_SET); // SET Count 
     lcd_write_reg(cmd);
}

void LCD_Data(unsigned int data)
{
     outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD 
     outl( 0x40000 ,HW_LCDIF_CTRL_SET); // SET data MODE 
     outl( 0x10000 ,HW_LCDIF_CTRL_SET); // run LCD 
     //outl( 0xffff ,HW_LCDIF_CTRL_SET); // SET Count 
     outl( 0x01 ,HW_LCDIF_CTRL_SET); // SET Count 

     lcd_write_reg(data);
}

void LCD_Datas(unsigned short * data,int counts)
{
    int i;
     outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD 
     outl( 0x40000 ,HW_LCDIF_CTRL_SET); // SET data MODE 
     outl( 0x10000 ,HW_LCDIF_CTRL_SET); // run LCD 
     //outl( 0xffff ,HW_LCDIF_CTRL_SET); // SET Count 
     outl( counts ,HW_LCDIF_CTRL_SET); // SET Count 

     for (i = 0; i < counts; i++) {
         while ((0x01000000 & inl(HW_LCDIF_CTRL)) ==0 )    { 
            // printf("FIFO is full,HW_LCDIF_CTRL = %x\n",inl(HW_LCDIF_CTRL)); 
         }
         outw( *(data++),HW_LCDIF_DATA);
     }
}


void LCD_static_Datas(unsigned short data,int counts)
{
    int i;
     outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD 
     outl( 0x40000 ,HW_LCDIF_CTRL_SET); // SET data MODE 
     outl( 0x10000 ,HW_LCDIF_CTRL_SET); // run LCD 
     //outl( 0xffff ,HW_LCDIF_CTRL_SET); // SET Count 
     outl( counts ,HW_LCDIF_CTRL_SET); // SET Count 

     for (i = 0; i < counts; i++) {
         while ((0x01000000 & inl(HW_LCDIF_CTRL)) ==0 )    { 
            // printf("FIFO is full,HW_LCDIF_CTRL = %x\n",inl(HW_LCDIF_CTRL)); 
         }
         outw( data,HW_LCDIF_DATA);
     }
}

void LCD_Reg_Set(unsigned int cmd,unsigned int data)
{
	LCD_Command(cmd);
	LCD_Data(data);
}

void LCD_SetDispAddr(unsigned int x,unsigned int y)
{
	LCD_Reg_Set(0x20, x);
	LCD_Reg_Set(0x21, y);
	LCD_Command(0x22);
}

void LCD_SetPixel(unsigned int x, unsigned int y, unsigned int color)
{
    	LCD_SetDispAddr(x,y);
	LCD_Data(color);	        //color表示RGB值, 这个函数的功能是往(x,y)写入color值
}

void LCDDEV_SetWindow(int x0, int y0, int x1, int y1)
{
	LCD_Reg_Set(0x50, y0);		// Horizontal GRAM Start Address-----HSA[7:0]
	LCD_Reg_Set(0x51, y1);		// Horizontal GRAM End Address-----HEA[7:0]
	LCD_Reg_Set(0x52, x0);		// Vertical GRAM Start Address-----VSA[8:0]
	LCD_Reg_Set(0x53, x1);		// Vertical GRAM Start Address-----VEA[8:0]
	LCD_SetDispAddr(x0, y0);
	//LCD_Command(0x22);
}


void  PowerOnLcdinit()
{
	lcd_delay(200);
	LCD_Reg_Set(0x00E5, 0x8000); 	 // Set the internal vcore voltage
	LCD_Reg_Set(0x0000, 0x0001); 	 // Start internal OSC.
	lcd_delay(40);
	LCD_Reg_Set(0x0001, 0x0000);	 // set SS and SM bit
	LCD_Reg_Set(0x0002, 0x0700);	 // set 1 line inversion
	LCD_Reg_Set(0x0003, 0x1238);	 // set GRAM write direction and BGR=1.
	LCD_Reg_Set(0x0004, 0x0000);	 // Resize register

	LCD_Reg_Set(0x0008, 0x0202);	 // set the back porch and front porch
	LCD_Reg_Set(0x0009, 0x0000);	 // set non-display area refresh cycle ISC[3:0]
	LCD_Reg_Set(0x000A, 0x0000);	 // FMARK function
	LCD_Reg_Set(0x000C, 0x0000);		 // RGB interface setting
	LCD_Reg_Set(0x000D, 0x0000);	 // Frame marker Position
	LCD_Reg_Set(0x000F, 0x0000);		 // RGB interface polarity
    LCD_Reg_Set(0x002b,0x0020);   //frame rate and color control(0x0000)

	//*************Power On sequence ****************
	LCD_Reg_Set(0x0010, 0x0000);		 // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_Reg_Set(0x0011, 0x0007);		 // DC1[2:0], DC0[2:0], VC[2:0]
	LCD_Reg_Set(0x0012, 0x0000);		 // VREG1OUT voltage
	LCD_Reg_Set(0x0013, 0x0000);		 // VDV[4:0] for VCOM amplitude
	lcd_delay(200);				// Dis-charge capacitor power voltage

	LCD_Reg_Set(0x0010, 0x17B0);		 // SAP, BT[3:0], AP, DSTB, SLP, STB
	LCD_Reg_Set(0x0011, 0x0007);		 // DC1[2:0], DC0[2:0], VC[2:0]
	lcd_delay(50);					 // Delay 50ms
	LCD_Reg_Set(0x0012, 0x013c);		 // VREG1OUT voltage
	lcd_delay(50);					 // Delay 50ms
	LCD_Reg_Set(0x0013, 0x1400);		 // VDV[4:0] for VCOM amplitude
	LCD_Reg_Set(0x0029, 0x0012);		 // VCM[4:0] for VCOMH
	lcd_delay(50);

	LCD_Reg_Set(0x0020, 0x0000);		 // GRAM horizontal Address
	LCD_Reg_Set(0x0021, 0x0000);		 // GRAM Vertical Address

	// ----------- Adjust the Gamma	Curve ----------//
	LCD_Reg_Set(0x0030, 0x0002);
	LCD_Reg_Set(0x0031, 0x0606);
	LCD_Reg_Set(0x0032, 0x0501);


	LCD_Reg_Set(0x0035, 0x0206);
	LCD_Reg_Set(0x0036, 0x0504);
	LCD_Reg_Set(0x0037, 0x0707);
	LCD_Reg_Set(0x0038, 0x0306);
	LCD_Reg_Set(0x0039, 0x0007);

	LCD_Reg_Set(0x003C, 0x0700);
	LCD_Reg_Set(0x003D, 0x0700);

	//------------------ Set GRAM area ---------------//
	LCD_Reg_Set(0x0050, 0x0000);		// Horizontal GRAM Start Address
	LCD_Reg_Set(0x0051, 0x00EF);		// Horizontal GRAM End Address
	LCD_Reg_Set(0x0052, 0x0000);		// Vertical GRAM Start Address
	LCD_Reg_Set(0x0053, 0x013F);		// Vertical GRAM Start Address


	LCD_Reg_Set(0x0060, 0x2700);		// Gate Scan Line
	LCD_Reg_Set(0x0061, 0x0001);		// NDL,VLE, REV
	LCD_Reg_Set(0x006A, 0x0000);		// set scrolling line

	//-------------- Partial Display Control ---------//
	LCD_Reg_Set(0x0080, 0x0000);
	LCD_Reg_Set(0x0081, 0x0000);
	LCD_Reg_Set(0x0082, 0x0000);
	LCD_Reg_Set(0x0083, 0x0000);
	LCD_Reg_Set(0x0084, 0x0000);
	LCD_Reg_Set(0x0085, 0x0000);

	//-------------- Panel Control -------------------//
	LCD_Reg_Set(0x0090, 0x0010);
	LCD_Reg_Set(0x0092, 0x0000);
	LCD_Reg_Set(0x0093, 0x0003);
	LCD_Reg_Set(0x0095, 0x0110);
	LCD_Reg_Set(0x0097, 0x0000);
	LCD_Reg_Set(0x0098, 0x0000);


	LCD_Reg_Set(0x0007, 0x0173);		// 262K color and display ON
	LCD_Command(0x0022);


}

int do_lcdif_init(cmd_tbl_t *cmdtp,int argc,char* argv[]){
	lcdif_init();
	PowerOnLcdinit();
    return 0;
}

void fill_rgb_buffer(){
 	unsigned int i,j;
	unsigned short *p_lcdif_framebuffer=(unsigned short*)lcdif_data_buffer;
	
 	for(i=0;i<(LCDIF_ROW/3);i++) //176RGB x 220_dot
  	{
    		for(j=0;j<LCDIF_COL;j++)
     		{
	 		*p_lcdif_framebuffer++=0xf800;
     		}
  	}  
	for(i=(LCDIF_ROW/3);i<(LCDIF_ROW/3)*2;i++) //176RGB x 220_dot
  	{
    		for(j=0;j<LCDIF_COL;j++)
     		{
	 		*p_lcdif_framebuffer++=0x07E0;
     		}
  	} 
	for(i=(LCDIF_ROW/3)*2;i<LCDIF_ROW;i++) //176RGB x 220_dot
  	{
    		for(j=0;j<LCDIF_COL;j++)
     		{
	 		*p_lcdif_framebuffer++=0x001f;
     		}
  	} 
}

int lcdif_display_RGB(cmd_tbl_t *cmdtp,int argc,char* argv[]){ 

	unsigned short *p_lcdif_framebuffer=(unsigned short*)lcdif_data_buffer;
    fill_rgb_buffer();
	LCDDEV_SetWindow(0, 0, LCD_MAX_XSIZE-1, LCD_MAX_YSIZE-1);
	LCD_Datas(p_lcdif_framebuffer,0x9600);
	LCD_Datas(p_lcdif_framebuffer+0x9600,0x9600);
    return 0;
}

void blank_buff(int val){
    int *b;
    int i;
    b = (int *)lcdif_data_buffer;

    //memset(lcdif_data_buffer,val,FRAME_BUFFER_LEN)
    for (i=0;i<(FRAME_BUFFER_LEN/4);i++) {
        *(b + i) = val;
    }
}
void refresh_dma(){
     LCDDEV_SetWindow(0, 0, LCD_MAX_XSIZE-1, LCD_MAX_YSIZE-1);     

     outl( 0x10000 ,HW_LCDIF_CTRL_CLR); // STOP LCD 
     outl( 0x40000 ,HW_LCDIF_CTRL_SET); // SET data MODE 
     outl( 0xffff,HW_LCDIF_CTRL_CLR); // clear Count 

     //lcdif_dma_pkg_init();
     lcdif_dma_start();	

}
int lcdif_refresh_dma(cmd_tbl_t *cmdtp,int argc,char* argv[]){ 

	printf("before semphore=0x%x\n",inl(HW_APBX_CH4_SEMA)&0x00ff0000);
    refresh_dma();

    return 0;
}

void lcdif_display_color(unsigned short color){
 	unsigned int i,j;
	LCDDEV_SetWindow(0, 0, LCD_MAX_XSIZE-1, LCD_MAX_YSIZE-1);
    LCD_static_Datas(color,0x9600);
    LCD_static_Datas(color,0x9600);
}

#ifdef CONFIG_ASC_DISPLAY

int lprintf(char *idata){
    int length=0;
    int val,c;
    if (col_now>=BYTES_COL) {
        col_now=0;
        blank_buff(0xeeeeeeee);
    }
    //while (idata[length] !='\0')length++;
    //if ((length) > (BYTES_ROW*BYTES_COL -((col_now-1)*BYTES_ROW+row_now))) {
    //    col_now=0;
    //    blank_buff();
    //    refresh_dma();
    //}
    //printf("row_now=%d,col_now=%d,idata=%d\n",row_now,col_now,idata);
    length = lcd_printf(row_now,col_now,idata);
    c = length/BYTES_ROW;
    if ((length % BYTES_ROW) == 0) {
        col_now += c;
        row_now =0;
    }else{
        col_now += (c+1);
        row_now =0;
    }
    return length;
}
 
#endif //CONFIG_ASC_DISPLAY

int do_lcdif_display_color(cmd_tbl_t *cmdtp,int argc,char* argv[])
{	
	ulong color;
	if (argc != 2) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}	

	color = TextToLong_TRL(argv[1]);
	
	lcdif_display_color((unsigned short )color);
	
    return 0;
}

int do_lcdif_refresh_fb(cmd_tbl_t *cmdtp,int argc,char* argv[]){	

	unsigned short *p_lcdif_framebuffer=(unsigned short*)lcdif_data_buffer;
	LCDDEV_SetWindow(0, 0, LCD_MAX_XSIZE-1, LCD_MAX_YSIZE-1);
	LCD_Datas(p_lcdif_framebuffer,0x9600);
	LCD_Datas(p_lcdif_framebuffer+0x9600,0x9600);
	
    return 0;
}

int do_lcdif_word(cmd_tbl_t *cmdtp,int argc,char* argv[]){

    long row_s,col_s,num;
    int val,i;

    if(argc == 1){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}
    //row_s = TextToLong_TRL(argv[1]);
    //col_s = TextToLong_TRL(argv[1]);
    //num = (long)*argv[3];
    //val = draw_asc(row_s,col_s,num);
    //draw_p(row_s,col_s);
    //val = draw_col_line(row_s);
    //val = lcd_printf(8,"asdfkjgjkl;dsjakl;jhjhj");
#ifdef CONFIG_ASC_DISPLAY
    val = lprintf(argv[1]);
#else //CONFIG_ASC_DISPLAY
    printf("no asc display support here,choose in make menuconfig\n");
#endif //CONFIG_ASC_DISPLAY

    refresh_dma();
    return 0;
}


BOOT_CMD(lcdif_init,do_lcdif_init,
         " #lcdif_init",
         "lcdif initial");
BOOT_CMD(lcdif_dis,do_lcdif_display_color,
         " #lcdif_dis <color>",
         "lcdif display a given color");
BOOT_CMD(lcdif_ref,do_lcdif_refresh_fb,
         " #lcdif_ref",
         "lcdif refresh framebuffer");
BOOT_CMD(lcdif_rgb,lcdif_display_RGB,
         " #lcdif_rgb",
         "lcdif draw a test RGB frame");
BOOT_CMD(lcdif_ref_dma,lcdif_refresh_dma,
         " #lcdif_ref_dma",
         "lcdif refresh framebuffer use dma");
BOOT_CMD(lcdif_asc,do_lcdif_word,
         " #lcdif_asc  string",
         "lcdif refresh framebuffer use dma");

//*****************************************************
#endif
