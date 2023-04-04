/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Lcd Source file


------------------- Version 1.0  ----------------------
Create File
 He Yong 2006-11-06

*/

#include <common.h>
#include <drivers/gpio_spi.h>
#include <drivers/lcd.h>

/* Debug option*/
#define dbg_lcd_printf(msg...)	if (LCD_DEBUG) { printf(msg); }

volatile AS3310_DMA_PKG NCached  dmapkg[LCD_PKG_NUM];
volatile AS3310_DMA_PKG NCached  dmapkg_y[2];
volatile AS3310_DMA_PKG NCached  dmapkg_u[1];
volatile AS3310_DMA_PKG NCached  dmapkg_v[1];

volatile char NCached lcd_framebuffer_4bpp[COL*ROW>>1];
volatile char NCached lcd_framebuffer[COL*ROW*2];
volatile int NCached lcd_table[8];


#ifdef LCD_YUV
AS3310_DMA_PKG * lcd_dma_table;
AS3310_DMA_PKG * lcd_dma_pkg_y[2];
AS3310_DMA_PKG * lcd_dma_pkg_u;
AS3310_DMA_PKG * lcd_dma_pkg_v;
#endif //LCD_YUV

static unsigned int point_a,point_b,counter;
static unsigned int shift_back;
static unsigned int shift_back_data_ready;
static ushort color,color_control;
static unsigned int puts_1;

static int is_4bpp=0;

void delayms(int ms){
    delay(ms<<10);
}


 void u_dma_start(ulong pkg_addr,int pkg_num){
    outl(pkg_addr ,HW_APBH_LCD_CH2_NXTCMDAR);
    outl(pkg_num ,HW_APBH_LCD_CH2_SEMA);
 }

 void v_dma_start(ulong pkg_addr,int pkg_num){
    outl(pkg_addr ,HW_APBH_LCD_CH1_NXTCMDAR);;
    outl(pkg_num ,HW_APBH_LCD_CH1_SEMA);
 }

 /* Start the DMA, dedicated channel for LCD DMA */ 
 void lcd_dma_start(ulong pkg_phy_addr,int pkg_num){
     outl(pkg_phy_addr ,HW_APBH_LCD_CH0_NXTCMDAR);
     outl(pkg_num ,HW_APBH_LCD_CH0_SEMA);
 }

#if CONFIG_ILI_9320RGB
void LCD_CtrlWrite_ILI9320(ulong reg, ulong data){
    clear_GPIO(CEN_PORT,CEN_PIN);
    gpio_spi_trans((0x70<<16) | reg);  // select reg
    set_GPIO(CEN_PORT,CEN_PIN);

    delayms(1);

    clear_GPIO(CEN_PORT,CEN_PIN);
    gpio_spi_trans((0x72<<16) | data);  // write reg
    set_GPIO(CEN_PORT,CEN_PIN);

    delayms(1);
}

void ili9320_init(void){
      delayms(50);      //this delay time is necessary
    LCD_CtrlWrite_ILI9320(0x00e5, 0x8000);
    LCD_CtrlWrite_ILI9320(0x0000, 0x0001); 	   // Start internal OSC.
    LCD_CtrlWrite_ILI9320(0x0001, 0x0000);	   // set SS and SM bit
    LCD_CtrlWrite_ILI9320(0x0002, 0x0700);	   // set 1 line inversion
    
  //  #if ILI9320_SCREEN_VERTICAL
    LCD_CtrlWrite_ILI9320(0x0003, 0x1200);	   // set GRAM write direction and BGR=1.
  //  #else
  //  LCD_CtrlWrite_ILI9320(0x0003, 0x1238);	   // set GRAM write direction and BGR=1.
  //  #endif //ILI9320_SCREEN_VERTICAL
                                               
    LCD_CtrlWrite_ILI9320(0x0004, 0x0000);	   // Resize register
    
    LCD_CtrlWrite_ILI9320(0x0007, 0x0133);	   // hoffer
                              
    LCD_CtrlWrite_ILI9320(0x0008, 0x0202);	   // set the back porch and front porch
    LCD_CtrlWrite_ILI9320(0x0009, 0x0000);	   // set non-display area refresh cycle ISC[3:0]
    LCD_CtrlWrite_ILI9320(0x000A, 0x0000);	   // FMARK function
    LCD_CtrlWrite_ILI9320(0x000C, 0x0111);         // RGB interface setting
    LCD_CtrlWrite_ILI9320(0x000D, 0x0000);	   // Frame marker Position
    LCD_CtrlWrite_ILI9320(0x000F, 0x0003);         // RGB interface polarity

    LCD_CtrlWrite_ILI9320(0x002b,0x0010);   //frame rate and color control(0x0000) hoffer

  //*************Power On sequence ****************
    LCD_CtrlWrite_ILI9320(0x0010, 0x0000);         // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_CtrlWrite_ILI9320(0x0011, 0x0007);         // DC1[2:0], DC0[2:0], VC[2:0]
    LCD_CtrlWrite_ILI9320(0x0012, 0x0000);         // VREG1OUT voltage
    LCD_CtrlWrite_ILI9320(0x0013, 0x0000);         // VDV[4:0] for VCOM amplitude
      delayms(20);                                // Dis-charge capacitor power voltage
  
    LCD_CtrlWrite_ILI9320(0x0010, 0x17b0);         // SAP, BT[3:0], AP, DSTB, SLP, STB
    LCD_CtrlWrite_ILI9320(0x0011, 0x0007);         // DC1[2:0], DC0[2:0], VC[2:0]
      delayms(20);			           // Delay 50ms  
    LCD_CtrlWrite_ILI9320(0x0012, 0x013c);         // VREG1OUT voltage 
      delayms(20);			           // Delay 50ms
	LCD_CtrlWrite_ILI9320(0x0013, 0x1400);         // VDV[4:0] for VCOM amplitude
	LCD_CtrlWrite_ILI9320(0x0029, 0x0012);         // VCM[4:0] for VCOMH
      delayms(30);    
    
    LCD_CtrlWrite_ILI9320(0x0020, 0x0000);         // GRAM horizontal Address
    LCD_CtrlWrite_ILI9320(0x0021, 0x0000);         // GRAM Vertical Address
  
  
  // ----------- Adjust the Gamma  Curve ----------//
//    LCD_CtrlWrite_ILI9320(0x0030, 0x0000);		
//    LCD_CtrlWrite_ILI9320(0x0031, 0x0505);		
//    LCD_CtrlWrite_ILI9320(0x0032, 0x0004);

    LCD_CtrlWrite_ILI9320(0x0030, 0x0002);		
    LCD_CtrlWrite_ILI9320(0x0031, 0x0606);		
    LCD_CtrlWrite_ILI9320(0x0032, 0x0501);// hoffer
    		
    LCD_CtrlWrite_ILI9320(0x0035, 0x0006);		
    LCD_CtrlWrite_ILI9320(0x0036, 0x0707);		
    LCD_CtrlWrite_ILI9320(0x0037, 0x0105);		
    LCD_CtrlWrite_ILI9320(0x0038, 0x0002);		
    LCD_CtrlWrite_ILI9320(0x0039, 0x0707);
    
    LCD_CtrlWrite_ILI9320(0x003C, 0x0704);		
    LCD_CtrlWrite_ILI9320(0x003D, 0x0807);	
  
  //------------------ Set GRAM area ---------------//
    LCD_CtrlWrite_ILI9320(0x0050, 0);	    // Horizontal GRAM Start Address
    LCD_CtrlWrite_ILI9320(0x0051, 240-1);	    // Horizontal GRAM End Address
    LCD_CtrlWrite_ILI9320(0x0052, 0);	    // Vertical GRAM Start Address
    LCD_CtrlWrite_ILI9320(0x0053, 320-1);	    // Vertical GRAM End Address
    
      
    LCD_CtrlWrite_ILI9320(0x0060, 0x2700);	    // Gate Scan Line
    LCD_CtrlWrite_ILI9320(0x0061, 0x0001);	    // NDL,VLE, REV
    LCD_CtrlWrite_ILI9320(0x006A, 0x0000);	    // set scrolling line
  
  //-------------- Partial Display Control ---------//
    LCD_CtrlWrite_ILI9320(0x0080, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0081, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0082, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0083, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0084, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0085, 0x0000);
    
  //-------------- Panel Control -------------------//
    LCD_CtrlWrite_ILI9320(0x0090, 0x0010);
    LCD_CtrlWrite_ILI9320(0x0092, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0093, 0x0003);
    LCD_CtrlWrite_ILI9320(0x0095, 0x0110);
    LCD_CtrlWrite_ILI9320(0x0097, 0x0000);
    LCD_CtrlWrite_ILI9320(0x0098, 0x0000);
  
  
    LCD_CtrlWrite_ILI9320(0x0007, 0x0173);	    // 262K color and display ON
}
#endif


void lcd_fill_frame_16bpp_(ushort FrameBuffer[][COL])
{
    int i;
    int j;
    ushort color_table_4pp[16]={
                                0xf800,
                                0x07e0,
                                0x001f,
                                0xffe0,
                                0x07ff,
                                0xf81f,
                                0xffff,
                                0xf9e7,
                                0xf800,
                                0x07e0,
                                0x001f,
                                0xffe0,
                                0x07ff,
                                0xf81f,
                                0xffff,
                                0xf9e7};    
    for(i=0;i<ROW/4;i++)
    {
        for(j=0;j<COL;j++)
            FrameBuffer[i][j] = 0xf800;
    }
    for(i=ROW/4;i<ROW/2;i++)
    {
        for(j=0;j<COL;j++)
            FrameBuffer[i][j] = 0x07e0;
    }
    for(i=ROW/2;i<(3*ROW/4);i++)
    {
        for(j=0;j<COL;j++)
            FrameBuffer[i][j] = 0x001f;
    }
    for(i=(3*ROW/4);i<ROW;i++)
    {
        for(j=0;j<COL;j++)
            FrameBuffer[i][j] = 0xffe0;
    }

}

void lcd_fill_frame_16bpp()
{
    volatile ushort ** FrameBuffer;
    FrameBuffer = (volatile ushort **)lcd_framebuffer;
    lcd_fill_frame_16bpp_((LCDBIT (*)[COL])FrameBuffer);    
}


int do_irq_lcd(int priv){
    struct device * dev = (struct device *)priv;
    outl(0x02000000, AS3310_LCD_CTRL_BASE + 0x8); // Clear
#ifdef CONFIG_FB_AS3310_DEBUG
    putc('&');
#endif
    if (dev->status & LCD_CONTROL_RUN){
        if ((IS_DMA_LCD_COMPLETE())) {
            if (dev->status & LCD_CONTROL_MODE_RGB) {
                lcd_dma_start(dmapkg[0].NEXT_PKG ,4);
            }
            else{
                u_dma_start((ulong)(&(dmapkg_u[0].NEXT_PKG)),1);
                v_dma_start((ulong)(&(dmapkg_v[0].NEXT_PKG)),1);
                lcd_dma_start((ulong)(&(dmapkg_y[0].NEXT_PKG)),2);
            }
        }
        else { puts("<X>");}    
    }
    return 0;
}


int init_as3310_lcd_dma_pkg(){

    int i;
    ulong dma_phy_addr;
    ulong len,len_div;

    dma_phy_addr = (ulong)lcd_framebuffer;

/*  ================= for RGB Packages ==================*/

    len = COL*ROW*2;//16bpp
    len_div=(len>>2);
       /*      Magic Number     */

    memset((char *)lcd_table,0,sizeof(lcd_table));
    lcd_table[0] = 0x00004000;

    /*     Prepare DMA PKG */

    for (i=0;i<LCD_PKG_NUM-1;i++) {
        dmapkg[i].NEXT_PKG =  (ulong) ( &( dmapkg[i+1].NEXT_PKG ));
    }
           // load color table
    dmapkg[0].CTRL = 0x002000c2;//32bytes and no chain and to lcdctrl one pio write
    dmapkg[0].BUFFER = (ulong)lcd_table;
    dmapkg[0].CMD0 = 0x40000000; //clear the gate

    //dprintk("\ntable VP :0x%08x  PHY: 0x%08x\n",_lcd_table,_lcd_table->dma_addr);
    //dprintk("\nfirst table word :0x%08x\n",*((ulong volatile *)phys_to_virt(dmapkg[0].BUFFER)));
    //dprintk("\nFrameBuffer Physical Address: 0x%08x\n",dma_phy_addr);
   // dmapkg[1].CTRL = 0x960010c2;//0x9600  bytes and no chain
    #ifdef CONFIG_LCD_IRQ
   
        dmapkg[1].CTRL = 0x000010c6 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
        dmapkg[1].BUFFER = (ulong)dma_phy_addr;
        dmapkg[1].CMD0 = 0x2c000000 + (len_div>>1);//enrev and startf and en irq and 0x6*16bits
        
                // data 2
        dmapkg[2].CTRL = 0x000010c6 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
        dmapkg[2].BUFFER = (ulong)dma_phy_addr + (len_div);
        dmapkg[2].CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
        
                // data 3
        dmapkg[3].CTRL = 0x000010c6 + (len_div<<16);//0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
        dmapkg[3].BUFFER = (ulong)dma_phy_addr + (len_div*2);
        dmapkg[3].CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
        
                // data 4
        dmapkg[4].CTRL = 0x000010c2 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
        dmapkg[4].BUFFER = (ulong)dma_phy_addr + (len_div*3);
        dmapkg[4].CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits

    #endif // CONFIG_FB_AS3310_IRQ

    #ifdef CONFIG_LCD_CYCLED
    
        dmapkg[1].CTRL = 0x00000086 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone 
        dmapkg[1].BUFFER = (ulong)dma_phy_addr;                                                                                        
        dmapkg[1].CMD0 = 0x2C000000 + (len_div>>1);//no enrev and no startf and en irq and 0x6*16bits                                  
                                                                                                                                       
                                                                                                                                       
        dmapkg[2].CTRL = 0x00001086 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write                    
        dmapkg[2].BUFFER = (ulong)dma_phy_addr + (len_div);                                                                            
        dmapkg[2].CMD0 = 0x20000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits                                     
                                                                                                                                       
                                                                                                                                       
        dmapkg[3].CTRL = 0x00001086 + (len_div<<16);//0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone     
        dmapkg[3].BUFFER = (ulong)dma_phy_addr + (len_div*2);                                                                          
        dmapkg[3].CMD0 = 0x20000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits                                     
                                                                                                                                       
                                                                                                                                       
        dmapkg[4].CTRL = 0x00001086 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write                    
        dmapkg[4].BUFFER = (ulong)dma_phy_addr + (len_div*3);                                                                          
        dmapkg[4].CMD0 = 0x20000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits                                     
        
        dmapkg[4].NEXT_PKG = dmapkg[0].NEXT_PKG;  // CYCLE Mode
    
    #endif // CONFIG_FB_AS3310_CYCLED

/*  ================= for YUV Packages ==================*/

    len = COL*ROW;//8bpp
    len_div=(len>>1);

    /*     Prepare DMA PKG */
         // Y 0
    dmapkg_y[0].CTRL = 0x000010c6 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
    dmapkg_y[0].BUFFER = (ulong)dma_phy_addr;
    dmapkg_y[0].CMD0 = 0x2d800000 + (len_div>>2);//enrev and startf and en irq and 0x6*16bits
    dmapkg_y[0].NEXT_PKG = (ulong)(&(dmapkg_y[1].NEXT_PKG));
    
            // Y 1
    dmapkg_y[1].CTRL = 0x000010c2 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
    dmapkg_y[1].BUFFER = (ulong)dma_phy_addr + len_div;
    dmapkg_y[1].CMD0 = 0x25800000 + (len_div>>2);//enrev and no startf and en irq and 0x6*16bits
    
           // U
    dmapkg_u[0].CTRL = 0x000010c2 + (len_div<<15);//32bytes and no chain and to lcdctrl one pio write
    dmapkg_u[0].BUFFER =  (ulong)dma_phy_addr + (len_div<<1) ;
    dmapkg_u[0].CMD0 = 0x00000000 + (len_div>>3); //clear the gate
    
           // V
    dmapkg_v[0].CTRL = 0x000010c2 + (len_div<<15);//32bytes and no chain and to lcdctrl one pio write
    dmapkg_v[0].BUFFER =  (ulong)dma_phy_addr + (len_div<<1)+ (len_div>>1);
    dmapkg_v[0].CMD0 = 0x00000000 + (len_div>>3); //clear the gate


return 0;

}


void lcd_init(struct device * dev){
 volatile AS3310_CLK * clk;
 volatile AS3310_LCD_CTRL * lcd_ctrl;
 ulong temp;
 int i;
 ulong * ptr;
 shift_back = 0;
 shift_back_data_ready = 0;
 irq_action_t lcd_irq;
 int ret;

    /*     Pin Assign      */
    for (i = GPIO_LCD_D_PIN_START; i <= GPIO_LCD_D_PIN_END;i++) {
        request_as3310_gpio(GPIO_LCD_D_PORT,i,LCD_PIN_TYPE);
    }
    request_as3310_gpio(GPIO_LCD_MISC_PORT,GPIO_LCD_DEN_PIN,LCD_PIN_TYPE);
    request_as3310_gpio(GPIO_LCD_MISC_PORT,GPIO_LCD_VSYNC_PIN,LCD_PIN_TYPE);
    request_as3310_gpio(GPIO_LCD_MISC_PORT,GPIO_LCD_HSYNC_PIN,LCD_PIN_TYPE);
    request_as3310_gpio(GPIO_LCD_MISC_PORT,GPIO_LCD_PIXCLK_PIN,LCD_PIN_TYPE);

    /*     Send Reset Signal      */
    request_as3310_gpio(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN,PIN_FUNCTION_GPIO);
    clear_GPIO(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN);
    set_GPIO(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN);

#ifdef CONFIG_ILI_9320RGB
    /*     Send Cen Signal      */
    request_as3310_gpio(LCD_CEN_GPIO_PROT,LCD_CEN_GPIO_PIN,PIN_FUNCTION_GPIO);
    clear_GPIO(LCD_CEN_GPIO_PROT,LCD_CEN_GPIO_PIN);
    set_GPIO(LCD_CEN_GPIO_PROT,LCD_CEN_GPIO_PIN);

    /*    Reset LCD     */
    set_GPIO(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN);
    delayms(1);
    clear_GPIO(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN);
    delayms(20);
    set_GPIO(LCD_RESETN_GPIO_PROT,LCD_RESETN_GPIO_PIN);
    delayms(20);

#ifndef CONFIG_PWM
    /* turn on back light */
    request_as3310_gpio(LCD_BACKLIGHT_GPIO_PROT,LCD_BACKLIGHT_GPIO_PIN,PIN_FUNCTION_GPIO);
    set_GPIO(LCD_BACKLIGHT_GPIO_PROT,LCD_BACKLIGHT_GPIO_PIN);
#endif //CONFIG_PWM

    gpio_spi_init();
    ili9320_init();
    gpio_spi_release();
#endif


    lcd_irq.irq = INT_AS3310_LCD;
    lcd_irq.irq_handler = do_irq_lcd;
    lcd_irq.clear = NULL;
    lcd_irq.priv_data = (int)dev;
    ret = request_irq(&lcd_irq);
    if (ret) {
        printf("LCD Request IRQ %d Error\n",INT_AS3310_LCD);
    }

    init_as3310_lcd_dma_pkg();

    dma_lcd_init();
    dma_start_lcd_apbh((ulong)&(dmapkg[0].NEXT_PKG),1,0);
    
    /*     LCD Init        */
    
    lcd_ctrl = AS3310_GetBase_LCD_CTRL();
    lcd_ctrl->CTRL0[2] = 0x80000000;//clear the gate used to sync rst
    lcd_ctrl->CTRL0[2] = 0x40000000;//clear the rst
    
    lcd_ctrl->TIMING0[0] = 0x02021000 + ((SCREEN_SIZE_COL - COL)<<16) + (COL - 1);//319  pixels/line (margin =0)
    lcd_ctrl->TIMING1[0] = 0x01010800 + ((SCREEN_SIZE_ROW - ROW)<<16) + (ROW - 1);//239  lines vsw =3
    lcd_ctrl->TIMING2[0] = 0xFF300000 + (LCD_PIXCLK_DIVIDER);//pixel clk = hclk/16....

#if CONFIG_ILI_9320RGB
    lcd_ctrl->TIMING0[0] = 0x02021000 + 240 - 1;// GRAM Her
    lcd_ctrl->TIMING1[0] = 0x01010800 + 320 - 1;// GRAM  Vert
	#ifdef CONFIG_BOARD_AS3310E_FPGA_QQ
	outl(239,0x80084120);//timing3 regiser
      #endif
#endif // ILI9320_RGB


 #ifdef RGB_CYCLE 
 lcd_ctrl->SECURE[1] = 0x0400000a; // auto start
 #endif //RGB_CYCLE

 lcd_ctrl->CTRL1[0] = 0xfc1004fd;//load p and lcd_en=1  tff
 lcd_ctrl->CTRL0[0] = 0x30000010;//en_receive and startp and receive 16*16bits
 //lcd_ctrl->LINEINT[0] = 0x00000002;

 dbg_lcd_printf("\nreceive 16*16bits...");

 		while(((lcd_ctrl->STAT[0])&0x00000040)==0);//wait for p is loaded
        /*  May die here !!!!!!*/

 printf("LCD inited.  Mode = %s %s\n",(dev->status & LCD_CONTROL_MODE_RGB) ?"RGB":"YUV",
#ifdef RGB_CYCLE
        "(DMA Cycled)"
#else
        "(IRQ)"
#endif
        );

 lcd_ctrl->CTRL1[0] = 0xfc2004fc;//load data and lcd_en=0 tff
 lcd_ctrl->CTRL1[0] = 0xfc2004fd;//load data and lcd_en=1 tff    

        if(dev->status & LCD_CONTROL_MODE_RGB)
            lcd_dma_start(dmapkg[0].NEXT_PKG ,4);
        else{
            u_dma_start((ulong)(&(dmapkg_u[0].NEXT_PKG)),1);
            v_dma_start((ulong)(&(dmapkg_v[0].NEXT_PKG)),1);
            lcd_dma_start((ulong)(&(dmapkg_y[0].NEXT_PKG)),2);
        }

 #ifdef RGB_CYCLE 
    lcd_ctrl->CTRL0[0] =  0x2c000000 + (len_div>>1);
 #endif //RGB_CYCLE
//dbg_lcd_printf("semphone: %x\n",inl(0x800040e0));
puts_1 =1;

}

int do_lcd_ioctl(cmd_tbl_t * cmdtp,int argc, char *argv[]){

    struct device * dev;
    int dev_index;

	if(argc!=2){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

    dev = device_get("lcdcontrol",&dev_index);
    dev->ioctl(dev,TextToLong_TRL(argv[1]),0);
     
    return 0;
}

BOOT_CMD(lcd_ioctl,do_lcd_ioctl,
        "#lcd_ioctl cmd\n"
         "0: stop\n"
         "1: run\n"
         "2: rgb mode\n"
         "3: yuv mode\n"
         , "LCD IO Control");


int lcd_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){
    switch (cmd) {
    case LCD_CONTROL_IOCTL_STOP:
        dev->status &= (~LCD_CONTROL_RUN);
        break;
    case LCD_CONTROL_IOCTL_RUN:
        if (!(dev->status & LCD_CONTROL_RUN)) {
            dev->status |= LCD_CONTROL_RUN;            
            if (dev->status & LCD_CONTROL_MODE_RGB) {
                lcd_dma_start(dmapkg[0].NEXT_PKG ,4);
            }
            else{
                u_dma_start((ulong)(&(dmapkg_u[0].NEXT_PKG)),1);
                v_dma_start((ulong)(&(dmapkg_v[0].NEXT_PKG)),1);
                lcd_dma_start((ulong)(&(dmapkg_y[0].NEXT_PKG)),2);
            }
        }
        break;
    case LCD_CONTROL_IOCTL_RGB:
        dev->status |= LCD_CONTROL_MODE_RGB;
        dev->status &= (~LCD_CONTROL_MODE_YUV);
        break;
    case LCD_CONTROL_IOCTL_YUV:
        dev->status |= LCD_CONTROL_MODE_YUV;
        dev->status &= (~LCD_CONTROL_MODE_RGB);
        break;
    }
    return 0;
}


int lcd_probe(struct device * dev){

    lcd_init(dev);
    return 0;
}

struct device dev_lcd = {
    .name       = "lcdcontrol",
    .dev_id     = 0,
    .probe      = lcd_probe,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = NULL,  
    .write      = NULL, 
    .ioctl      = lcd_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .status     = LCD_CONTROL_MODE_RGB, 
    .priv_data  = NULL, 
};

__add_device(dev_lcd);




//    
//    int lcd_monitor()
//    {
//        AS3310_LCD_CTRL * lcd_ctrl = AS3310_GetBase_LCD_CTRL();
//        puts("i am in monitor\n");
//        lcd_ctrl->MONITOR[0] = 0xc0800040;//en the monitor and not mask the exceed int
//                                          //not mask the finish int and en int and 16bpp monitor
//        puts("start the monitor\n");
//        return 0;
//    }
//    void lcd_monitor_exceed_irq()
//    {
//        puts("i am coming the exceed_irq\n");
//    
//        puts("the pixels exceed the 16bpps\n");
//        AS3310_LCD_CTRL * lcd_ctrl;
//        lcd_ctrl    =  AS3310_GetBase_LCD_CTRL();
//        lcd_ctrl->MONITOR[2] = 0x04000000;//clear the int
//                                          
//        outl(0x00000001, INT_IRQCLEAR1 ); // Clear
//        inl(AS3310_INT_BASE);
//        outl(0x0, INT_IRQCLEAR1 ); // Clear   
//    }
//    
//    void lcd_fill_fram_test(ulong FrameBuffer[][COL/8],ulong c1,ulong c2,ulong c3,ulong c4)
//    {
//        int row_u32 = COL/8;
//        int i;
//        int j;
//    
//        for(i=0;i<ROW;i++)
//        {
//            for(j=0;j<row_u32/4;j++)
//                FrameBuffer[i][j] = (c1&0x0000000f)|((c1<<4)&0x000000f0)|((c1<<8)&0x00000f00)|((c1<<12)&0x0000f000)|((c1<<16)&0x000f0000)|((c1<<20)&0x00f00000)|((c1<<24)&0x0f000000)|((c1<<28)&0xf0000000);
//            for(j=row_u32/4;j<row_u32/2;j++)
//                FrameBuffer[i][j] = (c2&0x0000000f)|((c2<<4)&0x000000f0)|((c2<<8)&0x00000f00)|((c2<<12)&0x0000f000)|((c2<<16)&0x000f0000)|((c2<<20)&0x00f00000)|((c2<<24)&0x0f000000)|((c2<<28)&0xf0000000);
//            for(j=row_u32/2;j<(3*row_u32/4);j++)
//                FrameBuffer[i][j] = (c3&0x0000000f)|((c3<<4)&0x000000f0)|((c3<<8)&0x00000f00)|((c3<<12)&0x0000f000)|((c3<<16)&0x000f0000)|((c3<<20)&0x00f00000)|((c3<<24)&0x0f000000)|((c3<<28)&0xf0000000);
//            for(j=(3*row_u32/4);j<row_u32;j++)
//                FrameBuffer[i][j] = (c4&0x0000000f)|((c4<<4)&0x000000f0)|((c4<<8)&0x00000f00)|((c4<<12)&0x0000f000)|((c4<<16)&0x000f0000)|((c4<<20)&0x00f00000)|((c4<<24)&0x0f000000)|((c4<<28)&0xf0000000);
//        }     
//    }
//    
//    void lcd_monitor_finish_irq()
//    {
//        puts("the pixels monitor is end into int process\n");
//        AS3310_LCD_CTRL * lcd_ctrl;
//        lcd_ctrl    =  AS3310_GetBase_LCD_CTRL();
//    
//        ulong monitor_r;
//        ulong pixel_num;
//    
//    	monitor_r	=  lcd_ctrl->MONITOR[0];
//    	pixel_num	=	monitor_r & 0x1f;
//        puts("monitor the pixel is ");
//        puth(pixel_num);
//        puts("\n");
//    
//    
//        ulong len = COL*ROW/2;//4bpp
//        ulong len_div=len>>2;
//    
//        ushort  p_ ;
//        ulong i;
//        ulong   p_addr_   =   (ulong)COLOR_TABLE_4BPP_ADDR;
//        ulong   pixel_check;
//        ulong   pixel_index1;
//        ulong   pixel_index2;
//        ulong   pixel_index3;
//        ulong   pixel_index4;
//        volatile ulong ** FrameBuffer;
//        FrameBuffer = (volatile ulong **)FRAME_BUFFER_4BPP_ADDR;
//        
//        if(pixel_num<=16)// can use the 4bpp
//        {
//            /*generate the dma structure*/
//            /*Prepare DMA PKG fro p and d*/
//    
//            for (i=0;i<LCD_PKG_NUM;i++) {
//                lcd_dma_pkg_4bpp[i] = (AS3310_DMA_PKG *)(LCD_PKG_4BPP_BASE + i*sizeof(AS3310_DMA_PKG));
//            }
//    
//            for (i=0;i<LCD_PKG_NUM-1;i++) {
//                lcd_dma_pkg_4bpp[i]->NEXT_PKG =  (ulong) ( &( lcd_dma_pkg_4bpp[i+1]->NEXT_PKG ));
//                //puts("\nptr 0x");
//               // puth((ulong)&(lcd_dma_pkg_4bpp[i]->NEXT_PKG));
//               // puts("   value  ");
//               // puth(lcd_dma_pkg_4bpp[i]->NEXT_PKG);
//            }
//           // puts("\n");
//    
//                   // load color table
//            lcd_dma_pkg_4bpp[0]->CTRL = 0x002010c6;//32bytes and one chain and one pio write
//            lcd_dma_pkg_4bpp[0]->BUFFER = (ulong)COLOR_TABLE_4BPP_ADDR;
//            lcd_dma_pkg_4bpp[0]->CMD0 = 0x3C000010; //enrev and startf and startp and en irq
//    
//                    // data 1
//            lcd_dma_pkg_4bpp[1]->CTRL = 0x000010c6 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//            lcd_dma_pkg_4bpp[1]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR;
//            lcd_dma_pkg_4bpp[1]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                    // data 2
//            lcd_dma_pkg_4bpp[2]->CTRL = 0x000010c6 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//            lcd_dma_pkg_4bpp[2]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div);
//            lcd_dma_pkg_4bpp[2]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                    // data 3
//            lcd_dma_pkg_4bpp[3]->CTRL = 0x000010c6 + (len_div<<16);//0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//            lcd_dma_pkg_4bpp[3]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div*2);
//            lcd_dma_pkg_4bpp[3]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                    // data 4
//            lcd_dma_pkg_4bpp[4]->CTRL = 0x000010c2 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//            lcd_dma_pkg_4bpp[4]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div*3);
//            lcd_dma_pkg_4bpp[4]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//            /*     Prepare DMA PKG fro d only */
//            for (i=0;i<(LCD_PKG_NUM-1);i++) {
//                lcd_dma_pkg_4bpp_data[i] = (AS3310_DMA_PKG *)( LCD_PKG_4BPP_DATA_BASE+ i*sizeof(AS3310_DMA_PKG));
//            }
//            for (i=0;i<(LCD_PKG_NUM-2);i++) {
//                lcd_dma_pkg_4bpp_data[i]->NEXT_PKG =  (ulong) ( &( lcd_dma_pkg_4bpp_data[i+1]->NEXT_PKG ));
//            }
//                    // data 1
//            lcd_dma_pkg_4bpp_data[0]->CTRL = 0x000010c6 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//            lcd_dma_pkg_4bpp_data[0]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR;
//            lcd_dma_pkg_4bpp_data[0]->CMD0 = 0x2C000000 + (len_div>>1);//enrev and startf and en irq and 0x6*16bits
//    
//                    // data 2
//            lcd_dma_pkg_4bpp_data[1]->CTRL = 0x000010c6 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//            lcd_dma_pkg_4bpp_data[1]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div);
//            lcd_dma_pkg_4bpp_data[1]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                    // data 3
//            lcd_dma_pkg_4bpp_data[2]->CTRL = 0x000010c6 + (len_div<<16);//0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//            lcd_dma_pkg_4bpp_data[2]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div*2);
//            lcd_dma_pkg_4bpp_data[2]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                    // data 4
//            lcd_dma_pkg_4bpp_data[3]->CTRL = 0x000010c2 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//            lcd_dma_pkg_4bpp_data[3]->BUFFER = (ulong)FRAME_BUFFER_4BPP_ADDR + (len_div*3);
//            lcd_dma_pkg_4bpp_data[3]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//            ///////////////////////////////////////////
//            /*fill the p to COLOR_TABLE_ADDR */
//            puts("p table:\n");
//            for(i=0;i<16;i++)
//            {
//                lcd_ctrl->CHECK[0] = (0x00000060 | i);
//                p_ = (ushort)(((lcd_ctrl->CHECK[0])&0xffff0000)>>16);
//                puth((ulong)p_);
//                puts(":");
//                p_ = ((p_>>1)&0x000f)|((p_>>3)&0x00f0)|((p_>>4)&0x0f00);//conver to 12bpp to table
//                outw(p_,p_addr_);
//                p_addr_ = p_addr_ + 2;
//            }
//                puts("\n");
//    
//            //     temp =  inl(color_table)&0xffff0fff;
//           outl((inl(COLOR_TABLE_4BPP_ADDR)&0xffff0fff)|0x00002000 ,COLOR_TABLE_4BPP_ADDR);//mark the 4bpp       
//            
//    		//////////////////////////////////////////////////////
//    		lcd_ctrl->SWITCH[1]=0x00000008;//set the shift_4bpp_ready
//    		//////////////////////////////////////////////////////
//            // above code must be done!!!!!!!!!!!!//
//            ///////////////////////////////////////////////////////////////////////////////
//            /*fill the first data*/
//    //              lcd_ctrl->CHECK[0] = 0xf7000070;//the color is not in the table
//    //      		while(((pixel_check=lcd_ctrl->CHECK[0]) & 0x80) == 0);//finished 
//    //              puth(pixel_check);
//    //              if ((pixel_check&0x100) != 0) {
//    //                  puts("   color check failed\n");            
//    //                  pixel_index1 = 3;
//    //              }
//    //              else pixel_index1 = pixel_check&0x0000000f;
//    //      /*        puts("pixel_index1:");
//    //              puth(pixel_index1);
//    //              puts("\n");*/
//    //      
//    //              lcd_ctrl->CHECK[0] = 0x07e00070;//green
//    //      		while(((pixel_check=lcd_ctrl->CHECK[0]) & 0x80) == 0);//finished       
//    //              pixel_index2 = pixel_check&0x0000000f;
//    //      /*        puts("pixel_index2:");
//    //              puth(pixel_index2);
//    //              puts("\n");*/
//    //      
//    //              lcd_ctrl->CHECK[0] = 0x001f0070;//blue
//    //      		while(((pixel_check=lcd_ctrl->CHECK[0]) & 0x80) == 0);//finished       
//    //              pixel_index3 = pixel_check&0x0000000f;
//    //      /*        puts("pixel_index3:");
//    //              puth(pixel_index3);
//    //              puts("\n");*/
//    //      
//    //      
//    //              lcd_ctrl->CHECK[0] = 0xffe00070;//r+g=orrign
//    //              while(((pixel_check=lcd_ctrl->CHECK[0]) & 0x80) == 0);//finished       
//    //              pixel_index4 = pixel_check&0x0000000f;
//    //      /*        puts("pixel_index4:");
//    //              puth(pixel_index4);
//    //              puts("\n");*/
//    //      
//    //              lcd_fill_fram_test((ulong (*)[COL/8])FrameBuffer,pixel_index4,pixel_index3,pixel_index2,pixel_index1);
//    //              ///////////////////////////////////////////////////////
//    //      		//////////////////////////////////////////////////////
//    //      		lcd_ctrl->SWITCH[1]=0x00000002;//set the shift_4bpp
//    //      		//////////////////////////////////////////////////////        
//        }
//        
//        lcd_ctrl->MONITOR[2] = 0x08000000;//clear the int                                  
//        outl(0x00000002, INT_IRQCLEAR1 ); // Clear
//        inl(AS3310_INT_BASE);
//        outl(0x0, INT_IRQCLEAR1 ); // Clear
//    }
//    
//    #ifdef LCD_YUV
//    void do_irq_lcd(){
//        AS3310_LCD_CTRL * lcd_ctrl;
//        lcd_ctrl = AS3310_GetBase_LCD_CTRL();
//      
//       //dma_apbh_reset_ch(1);
//       //dma_apbh_reset_ch(2);
//       //dma_apbh_reset_ch(3);
//    //     ulong semphone_ch1 = inl(HW_APBH_CH1_SEMA);
//    //     //puth(semphone_ch1);
//    //
//    //     semphone_ch1 = (semphone_ch1>>20)&0xf;
//    //     ulong semphone_ch2 = inl(HW_APBH_CH2_SEMA);
//    //     //puth(semphone_ch2);
//    //     semphone_ch2 = (semphone_ch2>>20)&0xf;
//    //     ulong semphone_ch3 = inl(HW_APBH_CH3_SEMA);
//    //     //puth(semphone_ch3);
//    //     semphone_ch3 = (semphone_ch3>>20)&0xf;
//    //     if((semphone_ch1 == 0)&(semphone_ch2 == 0)&(semphone_ch3 == 0))
//    //     {
//            u_dma_start(&(lcd_dma_pkg_u->NEXT_PKG),1);
//            v_dma_start(&(lcd_dma_pkg_v->NEXT_PKG),1);
//            lcd_dma_start(&(lcd_dma_pkg_y[0]->NEXT_PKG),2);
//          //   putc('L');
//    //    }
//    
//    
//        lcd_ctrl->CTRL0[2] = 0x02000000;
//        outl(0x00000008, INT_IRQCLEAR0 + 4); // Clear
//        inl(AS3310_INT_BASE);
//        outl(0x00000008, INT_IRQCLEAR0 + 8); // Clear
//    }
//    #endif// LCD_YUV
//    
//    #ifdef LCD_RGB
//    void do_irq_lcd(){
//    	unsigned long old,temp,old1,temp1;
//        AS3310_LCD_CTRL * lcd_ctrl;
//        lcd_ctrl = AS3310_GetBase_LCD_CTRL();
//        dbg_lcd_printf("r");
//    
//        #ifndef RGB_CYCLE
//        ulong   check_shift = lcd_ctrl->SWITCH[0];
//        if(check_shift & 0x1){//set the shift mode
//            if(puts_1) {
//                puts("enter the 4bpp loade the data only\n");
//                   puts_1 = 0;
//            }
//             
//            lcd_ctrl->CTRL1[0] = 0xfca004fd;//4bpp tft map//only load the data
//                dma_start_lcd_apbh((ulong)&(lcd_dma_pkg_4bpp_data[0]->NEXT_PKG),4,0);
//        }
//        else if((check_shift & 0x2)|(check_shift & 0x4)) {//shift_4bpp or shift_2bpp is ok
//            puts("enter the 4bpp loade table and data only\nSWITCH:");
//            lcd_ctrl->CTRL1[0] = 0xfc8004fd;//4bpp tft map//set the transfer mode is loading the palette and loading data
//            dma_start_lcd_apbh((ulong)&(lcd_dma_pkg_4bpp[0]->NEXT_PKG),5,0);
//            lcd_ctrl->SWITCH[1] = 0x00000001;//set shift mode for next transfer
//            puth(inl(lcd_ctrl->SWITCH));
//            putc('\n');
//        }
//        else if(shift_back_data_ready & shift_back)    {
//            shift_back_data_ready = 0;
//            shift_back = 0;
//            lcd_ctrl->CTRL1[0] = 0xfc0004fd;//16bpp tft no map//set the transfer mode is loading the palette and loading data
//            dma_start_lcd_apbh((ulong)&(lcd_dma_back_pkg[0]->NEXT_PKG),5,0);
//        }
//        else  {
//            lcd_ctrl->CTRL1[0] = 0xfc2004fd;//16bpp tft no map //load data only
//            dma_start_lcd_apbh((ulong)lcd_dma_pkg[0]->NEXT_PKG,4,0);
//        }
//    
//        #else 
//        #endif // RGB_CYCLE
//        
//        lcd_ctrl->CTRL0[2] = 0x02000000;
//        outl(0x00000008, INT_IRQCLEAR0 + 4); // set Clear
//        
//        //puts("i out of the lcd irq\n");
//    }
//    #endif// LCD_RGB
//    
//    
//    int  lcd_shift_back()
//    {
//        puts("i want back!!\n");
//        AS3310_LCD_CTRL * lcd_ctrl;
//        int i;
//        lcd_ctrl = AS3310_GetBase_LCD_CTRL();
//        ulong len = COL*ROW*2;//16bpp
//        ulong len_div=len>>2;
//    
//        for (i=0;i<LCD_PKG_NUM;i++) {
//            lcd_dma_back_pkg[i] = (AS3310_DMA_PKG *)(LCD_PKG_BACK_BASE + i*sizeof(AS3310_DMA_PKG));
//        }
//    
//        for (i=0;i<LCD_PKG_NUM-1;i++) {
//            lcd_dma_back_pkg[i]->NEXT_PKG =  (ulong) ( &( lcd_dma_back_pkg[i+1]->NEXT_PKG ));
//            //puts("\nptr 0x");
//           // puth((ulong)&(lcd_dma_pkg_4bpp[i]->NEXT_PKG));
//           // puts("   value  ");
//           // puth(lcd_dma_pkg_4bpp[i]->NEXT_PKG);
//        }
//       // puts("\n");
//        /*Prepare DMA PKG fro p and d*/
//               // load color table
//        lcd_dma_back_pkg[0]->CTRL = 0x002010c6;//32bytes and one chain and one pio write
//        lcd_dma_back_pkg[0]->BUFFER = (ulong)COLOR_TABLE_ADDR;
//        lcd_dma_back_pkg[0]->CMD0 = 0x3C000010; //enrev and startf and startp and en irq
//    
//                // data 1
//        lcd_dma_back_pkg[1]->CTRL = 0x000010c6 + (len_div<<16);    //0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//        lcd_dma_back_pkg[1]->BUFFER = (ulong)lcd_framebuffer;
//        lcd_dma_back_pkg[1]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                // data 2
//        lcd_dma_back_pkg[2]->CTRL = 0x000010c6 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//        lcd_dma_back_pkg[2]->BUFFER = (ulong)lcd_framebuffer + (len_div);
//        lcd_dma_back_pkg[2]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                // data 3
//        lcd_dma_back_pkg[3]->CTRL = 0x000010c6 + (len_div<<16);//0x9600  bytes and one chain and to lcdctrl one pio write and no dec semphone
//        lcd_dma_back_pkg[3]->BUFFER = (ulong)lcd_framebuffer + (len_div*2);
//        lcd_dma_back_pkg[3]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//                // data 4
//        lcd_dma_back_pkg[4]->CTRL = 0x000010c2 + (len_div<<16);//0x9600 bytes and one cycle chain and to lcdctrl one pio write
//        lcd_dma_back_pkg[4]->BUFFER = (ulong)lcd_framebuffer + (len_div*3);
//        lcd_dma_back_pkg[4]->CMD0 = 0x24000000 + (len_div>>1);//enrev and no startf and en irq and 0x6*16bits
//    
//        shift_back = 1;//sign i come back you can prepare the data 
//        
//        /*Prepare First frame d*/
//        outl((inl(COLOR_TABLE_ADDR)&0xffff0fff)|0x00004000 ,COLOR_TABLE_4BPP_ADDR);//mark the 16bpp 
//        lcd_fill_frame_16bpp();
//        shift_back_data_ready = 1;
//    
//        lcd_ctrl->SWITCH[0] = 0x00000000;//clear all and shift back
//        return 0;
//    }
//    
//    /*
//    void rect_move(){
//        volatile ushort **FrameBuffer;
//    
//        FrameBuffer = (volatile LCDBIT **)FRAME_BUFFER_ADDR;    
//    
//        if (((counter++)&0xff) == 0) {
//            color = 0xffff - color;
//        }
//       // bzero((ulong * )FrameBuffer);
//        point_a++;point_b--;
//        //change_color(&color,&color_control);
//        Draw_Rect(point_a%320,point_a%240,(point_b+80)%320,point_b%240,FrameBuffer,&color);
//     }*/
//    
//    
//    BOOT_CMD(lcd_monitor,lcd_monitor,
//             " #lcd_monitor",
//             "monitor the lcd");
//    BOOT_CMD(lcd_shift_back,lcd_shift_back,
//             " #lcd_shift_back",
//             "lcd back");
//    
//    


#define LINE_WIDTH 0


//int bzero(ulong * buffer)
//{
//long i;
//volatile ulong * addr;
//ulong length;
//
//    addr = buffer;
//    length = (ROW*COL*sizeof(LCDBIT))>>2;
//    for (i=0;i<length;i++)
//        *(addr++)=0;
//    
//}

void draw_point(char * buffer,int row,int col,int val,int depth){
ulong offset = (row*COL + col);

//printf("(%d,%d)",col,row);

    if (depth ==16 ) {
        offset = (depth>>3)*offset;
        *((unsigned short *)(buffer + offset)) = val;
        return ;
    }
    if (depth == 4) {
        char temp_val;

        temp_val = ((val&0x8000)>>12) | ((val&0x180)>>6) | (val&0x1) ;

        buffer = buffer + (offset>>1);
        if (offset & 1){
            // low 4 bit
            *buffer &= ~0xf;
            *buffer |= (0xf&temp_val);
        }
        else{
            // high 4 bit
            *buffer &= ~0xf0;
            *buffer |= (0xf0&(temp_val<<4));
        }
        return ;
    }

}

void Draw_Line(long start_x,long start_y,long end_x,long end_y,LCDBIT FrameBuffer[][COL],LCDBIT *color){
char direction;
int step,i,j,m,l;
long start_endx,start_endy;
int color_depth;

if (is_4bpp == 1) color_depth = 4;
else color_depth = 16;

start_endx = start_x-end_x;
if(start_endx == 0) start_endx=1;
start_endy = start_y-end_y;
if(start_endy == 0) start_endy=1;

if (abs(start_x-end_x)>abs(start_y - end_y)) {
    direction='x';
	if (start_x > end_x) step=-1;
	else step=1;
	for(i=start_x; (((i<=end_x)&&(i>=start_x))  ||  ((i>=end_x)&&(i<=start_x)));i=i+step){
			j = ((long)((i-start_x)*(start_y - end_y))/start_endx)+start_y;
         //   putb(j);putc('\t');
			for(l=-LINE_WIDTH;l<LINE_WIDTH+1;l++){
				if ((i+l<COL)&&(i+l >=0)) 
					for(m=-LINE_WIDTH;m<LINE_WIDTH+1;m++)
					if ((j+m<ROW)&&(j+m >=0)) {
                        draw_point((char *)(&FrameBuffer[0][0]),j+m,i+l,(int)(*color),color_depth);
                        //FrameBuffer[j+m][i+l]=*color;
                    }
			}
  //  *color++;	
	}
}

else {
	direction='y';
	if (start_y > end_y) step=-1;
	else step=1;
	for(j=start_y; (((j<=end_y)&&(j>=start_y))  ||  ((j>=end_y)&&(j<=start_y))) ;j=j+step){
			i = ((long)((j-start_y)*(start_x-end_x))/start_endy)+start_x;
			for(l=-LINE_WIDTH;l<LINE_WIDTH+1;l++){
				if ((i+l<COL)&&(i+l >=0)) 
					for(m=-LINE_WIDTH;m<LINE_WIDTH+1;m++)
					if ((j+m<ROW)&&(j+m >=0)){
                        draw_point((char *)(&FrameBuffer[0][0]),j+m,i+l,(int)(*color),color_depth);
                        //FrameBuffer[j+m][i+l]=*color;
                    }
			}
		}	
  //  *color++;
	}

}


void Draw_Rect(long start_x,long start_y,long end_x,long end_y,LCDBIT FrameBuffer[][COL],LCDBIT *color){
int i,j;
int x1,x2,y1,y2;

if (start_x <= end_x) { x1 = start_x ; x2 = end_x;}
else  { x2 = start_x ; x1 = end_x;}

if (start_y <= end_y) { y1 = start_y ; y2 = end_y;}
else  { y2 = start_y ; y1 = end_y;}



    for (j = y1; j <= y2; j++) {
        FrameBuffer[j][x1] = *color;
        FrameBuffer[j][x2] = *color;

        }



    for (i = x1; i<=x2; i++) {
        FrameBuffer[y1][i] = *color;
        FrameBuffer[y2][i] = *color;
        }

}
/*
void Draw_Line(long start_x,long start_y,long end_x,long end_y,LCDBIT FrameBuffer[][COL],LCDBIT *color){
int i,j;
if (start_y == 0) {
    for (j = 0;j< ROW;j++) {
        FrameBuffer[j][80] = *color;
        FrameBuffer[j][240] = *color;
        }
    }
else {
    for (i = 0;i< COL;i++) {
        FrameBuffer[60][i] = *color;
        FrameBuffer[180][i] = *color;
        }
    }
}
*/


void Move_Edge(int *Ax,int *Ay,int step){
		if (*Ax==0){
			if(*Ay >= ROW - step) {*Ay=ROW;*Ax=1;}
			else (*Ay)+=step;		
		}
		else if(*Ay==ROW){
			if(*Ax >= COL - step) {*Ax=COL;*Ay=ROW-1;}
			else (*Ax)+=step;			
		}
		else if(*Ax==COL){
			if(*Ay <= step) {*Ax=COL-1;*Ay=0;}
			else (*Ay)-=step;			
		}
		else if(*Ay==0){
			if(*Ax <= step) {*Ax=0;*Ay=1;}
			else (*Ax)-=step;			
		}
}


void change_color(LCDBIT *color,int *color_control){
if(*color_control ==0){
*color = *color - 1 + (1<<6) ;
    if (((*color) & 0x1f) <2) {
        *color_control = 1;
        *color = 0x07c0;
    }
}

else if(*color_control ==1){
*color = *color - (1<<6) + (1<<11);
    if ((((*color)>>6) & 0x1f) <2) {
        *color_control = 2;
        *color = 0xf800;
    }
}

else {
*color = *color - (1<<11) + 1;
    if (((*color)>>11) <2) {
        *color_control = 0;
        *color = 0x1f;
    }
}

}


void Triangle_2(LCDBIT FrameBuffer[][COL],int tri_count,int A_step,int B_step,int C_step){
LCDBIT **FrameBuffer_temp;
int Ax,Ay,Bx,By,Cx,Cy;
int Ax0,Ay0,Bx0,By0,Cx0,Cy0;
uchar temp;
LCDBIT black,color,color0,color_control0;
int i;
int color_control;
int init_tri_count;

AS3310_LCD_CTRL * lcd_ctrl;
lcd_ctrl = AS3310_GetBase_LCD_CTRL();

	Ax=0,Ay=0,Bx=(COL>>1),By=ROW,Cx=COL,Cy=0;
	color=0x1f;
    color_control = 0;
    black=0;

    FrameBuffer_temp = (LCDBIT **)FrameBuffer;
    memset((void * )FrameBuffer_temp,0,(ROW*COL*sizeof(LCDBIT)));


while(getc(&temp)==0){

   if ( (lcd_ctrl->SWITCH[0]&(1<<3)) !=0 ) {
       if (is_4bpp == 0) {
           // entered 4bpp
           FrameBuffer_temp = (LCDBIT **)lcd_framebuffer_4bpp;
           memset((void * )FrameBuffer_temp,0,(ROW*(COL>>1)));
           lcd_ctrl->SWITCH[1]=0x00000002;//set the shift_4bpp
           printf("Entered 4 BPP Mode\n");

       }
       is_4bpp = 1;
   }
   else {
       if (is_4bpp == 1) {
           // entered 16bpp
           FrameBuffer_temp = (LCDBIT **)FrameBuffer;
           memset((void * )FrameBuffer_temp,0,(ROW*COL*sizeof(LCDBIT)));
       }
       is_4bpp = 0;
   }

for (i=0;i<TRI_STEP;i++) {
	Move_Edge(&Ax,&Ay,A_step);
	Move_Edge(&Bx,&By,B_step);
	Move_Edge(&Cx,&Cy,C_step);
}

    change_color(&color,&color_control);

Ax0=Ax;Ay0=Ay;Bx0=Bx;By0=By;Cx0=Cx;Cy0=Cy;color0=color;color_control0=color_control;

	Draw_Line(Ax,Ay,Bx,By,(LCDBIT (*)[COL])FrameBuffer_temp,&black);
	Draw_Line(Bx,By,Cx,Cy,(LCDBIT (*)[COL])FrameBuffer_temp,&black);
	Draw_Line(Cx,Cy,Ax,Ay,(LCDBIT (*)[COL])FrameBuffer_temp,&black);


for (i=0;i<TRI_STEP*tri_count;i++) {
	Move_Edge(&Ax,&Ay,A_step);
	Move_Edge(&Bx,&By,B_step);
	Move_Edge(&Cx,&Cy,C_step);
}
    Draw_Line(Ax,Ay,Bx,By,(LCDBIT (*)[COL])FrameBuffer_temp,&color);
	Draw_Line(Bx,By,Cx,Cy,(LCDBIT (*)[COL])FrameBuffer_temp,&color);
	Draw_Line(Cx,Cy,Ax,Ay,(LCDBIT (*)[COL])FrameBuffer_temp,&color);

 //   puth((Ax<<16)+Ay);putc(' ');puth((Bx<<16)+By);putc(' ');puth((Cx<<16)+Cy);
 //   putc('\n');

    Ax=Ax0;Ay=Ay0;Bx=Bx0;By=By0;Cx=Cx0;Cy=Cy0;color=color0;color_control=color_control0;


//    memcpy(0x20300000,0x20008000,0x100);

	}

}

    #if TEXT_LIB
void Draw_word(char * buffer,Point * start_p,HzDotStruct * word_s,LCDBIT color,int depth){
int i,j,index;

/* i COL index; j Row index*/
    for (j=0;j<WORD_HIGHT;j++) {
        for (i=0;i<WORD_WIDTH;i++) {
          index = i + (j*(WORD_WIDTH));
          if ( ( word_s->HzDot[index>>3] & (1<<(7-(i&0x7)))  ) !=0 ){
             // printf("(%d:%2d,%2d)",(index>>3),i,j);
              draw_point(buffer,j+start_p->y,i+start_p->x,color,depth);
          }
        }
    }

}



void word_display(){
char * FrameBuffer_temp;
uchar temp;
LCDBIT black,color,color0,color_control0;
int i;
int color_control;
int init_tri_count;
AS3310_LCD_CTRL * lcd_ctrl;
Point screen_size;
Point pnt;
//HzDotStruct * word_s;
int wd_index;
int xx,yy;

    screen_size.x = COL;
    screen_size.y = ROW;

    lcd_ctrl = AS3310_GetBase_LCD_CTRL();
    //word_s = got_Hzlib_ptr(wd);

	color=0x1f;
    color_control = 0;
    black=0;

    FrameBuffer_temp = (char *)lcd_framebuffer;
    memset((void * )FrameBuffer_temp,0,(ROW*COL*sizeof(LCDBIT)));


while(getc(&temp)==0){

//   if ( (lcd_ctrl->SWITCH[0]&(1<<3)) !=0 ) {
//       if (is_4bpp == 0) {
//           // entered 4bpp
//           FrameBuffer_temp = (LCDBIT **)FRAME_BUFFER_4BPP_ADDR;
//           memset((void * )FrameBuffer_temp,0,(ROW*(COL>>1)));
//           lcd_ctrl->SWITCH[1]=0x00000002;//set the shift_4bpp
//           printf("Entered 4 BPP Mode\n");
//
//       }
//       is_4bpp = 1;
//   }
//   else {
//       if (is_4bpp == 1) {
//           // entered 16bpp
//           FrameBuffer_temp = (LCDBIT **)FrameBuffer;
//           memset((void * )FrameBuffer_temp,0,(ROW*COL*sizeof(LCDBIT)));
//       }
//       is_4bpp = 0;
//   }
 
//    for (xx=0;xx<4;xx++) {
//        yy = 0;
//        for (i = 0 ; i<7; i++) {
//            pnt.x = xx*WORD_WIDTH*3;
//            pnt.y = yy;
//            Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(i+(7*xx)),color,16);
//            yy += WORD_HIGHT;
//            change_color(&color,&color_control);
//        }
//    }
//
//    pnt.x=48;pnt.y=80;
//    Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(28),color,16);
//    pnt.x=48;pnt.y=112;
//    Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(29),color,16);
//    change_color(&color,&color_control);
//    pnt.x=240;pnt.y=80;
//    Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(30),color,16);
//    pnt.x=240;pnt.y=112;
//    Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(31),color,16);

    yy = 16;
    for (i = 0 ; i<6; i++) {
        pnt.x = 144;
        pnt.y = yy;
        Draw_word(FrameBuffer_temp,&pnt,got_Hzlib_ptr(i+32),0xffff,16);
        yy += WORD_HIGHT;     
    }

	}

}
#endif

int do_cmd_lcd(cmd_tbl_t * cmdtp,int argc, char *argv[]){
 //   volatile LCDBIT *FrameBuffer[COL];
    volatile ushort **FrameBuffer,**FrameBuffer2;
    int i,row,col;
    ushort *ptr;
    ushort color;
    color = 0xffff;
    FrameBuffer = (volatile LCDBIT **)lcd_framebuffer;
    FrameBuffer2 = (volatile LCDBIT **)lcd_framebuffer_4bpp;

    int t = 0;
    Point temp;
    temp.x = 0;
    temp.y = 0;

	if(argc==1){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

//    if (!((argv[1][0]=='F')||(argv[1][0]=='f'))) 
//{

    if ((argv[1][0]=='L')||(argv[1][0]=='l')) {
        row = TextToLong_TRL(argv[2]);
        Draw_Line(0,row,COL,row,(LCDBIT (*)[COL])FrameBuffer,&color);
    }else  if((argv[1][0]=='C')||(argv[1][0]=='c')){
        col = TextToLong_TRL(argv[2]);
        Draw_Line(col,0,col,ROW,(LCDBIT (*)[COL])FrameBuffer,&color);
    }        
    else if((argv[1][0]=='T')||(argv[1][0]=='t')){
        printf("AlpScale ScreenSaver V1.0 (Press Any Key to Return..)\n");
        Triangle_2((LCDBIT (*)[COL])lcd_framebuffer,15,1,1,1);
    }
    else if((argv[1][0]=='B')||(argv[1][0]=='b')){
        printf("Screen Cleared\n");
        memset((void * )FrameBuffer,0,(ROW*COL*sizeof(LCDBIT)));
    }
    #if TEXT_LIB
    else if((argv[1][0]=='W')||(argv[1][0]=='w')){
            word_display();
         //  temp2 = /*got_Hzlib_ptr(0);*/ &HzLib_temp;
         //  for ( t=0 ; t<128 ; t++) {          
         //  printf(" %x",temp2->HzDot[t]);
         //  }
          //Draw_word(FRAME_BUFFER_ADDR,&temp,got_Hzlib_ptr(0),0xffff,16);
    }
    #endif //TEXT_LIB
//}
//  else  {
//              lcd_dma_start((ulong * )lcd_dma_pkg[0]->NEXT_PKG,4);
//  }
return 0;
}

BOOT_CMD(lcd,do_cmd_lcd,
        "#lcd [options]\n\
         L N\t(Draw Nth Row)\n\
         C N\t(Draw Nth Col)\n\
         T\t(ScreenSaver)\n\
         B\t(Clear Screen)",
        "Test LCD");

// void test_mode()
// {
//     //#ifdef TEST_MODE
//     ulong addr_sdram = 0x20000000;
//     
//     //while(1)
//     //{
//         for(addr_sdram=0x20000000;addr_sdram<0x24000000;addr_sdram = addr_sdram+4)
//         inl(addr_sdram);
//     //}
//     //#endif
// }


