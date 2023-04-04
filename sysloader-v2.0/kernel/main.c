/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Main Source file

Change log:

------------------- Version 1.0  ----------------------
He Yong 2006-03-18
    -- Copied code from console.c
zhangbo 2006-03-18
    -- transplant to alpos, add task management
*/

#include <common.h>
#include <drivers/lcdif.h>
#include <drivers/gpio_spi.h>
#include <drivers/usb.h>
#include <drivers/nand_search.h>
//#include <drivers/flash.h>
//#include <drivers/spi_flash.h>

#define ASM9260_SDRAM_SIP
//#undef	ASM9260_SDRAM_SIP

#define	ASM9260_LOAD_LINUX
//#undef	ASM9260_LOAD_LINUX

volatile char current_platform;

void set_pin_mux(int port,int pin,int mux_type)
{
    u32 val,addr;

    addr = (u32)(HW_IOCON_PIO_BASE + (port*32) + (pin*4));
    val = inl( addr );   // read org val

    val &= 0xFFFFFFF8; // clear mux feild
    val |= mux_type; // set mux feild with new value

    outl( val ,addr );   // Set new value
}

void set_pin_pullup(int port,int pin)
{
	do{
   		outl((1<<3),HW_IOCON_PIO0_0+port*0x20+pin*4 + 8);
   		outl((1<<4),HW_IOCON_PIO0_0+port*0x20+pin*4 + 4);
   } while(0);
}

void set_GPIO(int port,int pin){
int val,addr;
    val = (1<<pin);
    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (port*0x100) + 0x50 + 4;

    outl( val ,addr );   // Output data, set to 1
    addr += 0x20;
    outl( val ,addr );   // Output Enable
}

void clear_GPIO(int port,int pin){
int val,addr;
    val = (1<<pin);
    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (port*0x100) + 0x50 + 8;

    outl( val ,addr );   // Output data, set to 0
    addr += (0x20 - 4);
    outl( val ,addr );   // Output Enable
}

void Byte2HEX(uchar *tempstring,uchar data)
{
	uchar char_2_Hex;

	char_2_Hex=data>>4;
	if (char_2_Hex>9){
		char_2_Hex=char_2_Hex+'a'-10;
				}
	else char_2_Hex=char_2_Hex+'0';
	tempstring[0]=char_2_Hex;

	char_2_Hex=data%16;
	if (char_2_Hex>9){
		char_2_Hex=char_2_Hex+'a'-10;
				}
	else char_2_Hex=char_2_Hex+'0';
	tempstring[1]=char_2_Hex;

}


/*
Convert Ulong data to HEX String
*/
void Long2HEX(uchar *tempstring,ulong data)
{
	uchar temp;
	int i;
	for (i=0;i<4;i++){
		temp = data % 256;
		data = (data >> 8);
		Byte2HEX(&tempstring[6-2*i],temp);
		}

}

void puth(unsigned long  x){
    uchar long2hex[9];
    long2hex[8]=0;
    Long2HEX(long2hex,x);
    puts(long2hex);
}

void putb(uchar x){
    uchar long2hex[3];
    long2hex[2]=0;
    Byte2HEX(long2hex,x);
    puts(long2hex);
}

/*      Serial init     */
void serial_uart4_init(void)
{
    set_pin_mux(16,0,2);		//UART4-TXD
    set_pin_mux(16,1,2);		//UART4-RXD

    outl(1<<15,HW_AHBCLKCTRL0+4);			//UART4 ENABLE bit15---UART4
    outl(1,HW_UART4CLKDIV);					//UART4 devided by x from PLL	
	outl(0x1<<15,HW_AHBCLKCTRL0+8);			//UART4 clk gate  //eye check    
    outl(0x1<<15,HW_AHBCLKCTRL0+4);			//UART4 clk gate  //eye check	     
    outl(0x1<<15,HW_PRESETCTRL0+8);			//UART4 reset    //eye check    
    outl(0x1<<15,HW_PRESETCTRL0+4);			//UART4 reset    //eye check    
    outl(0xC0000000,HW_UART4_CTRL0+8);		//CLEAR UART4 SFTRST AND CLKGATE	
    outl(0x00062070,HW_UART4_LINECTRL);	// set bandrate 115200 12M	
    outl(0x301,HW_UART4_CTRL2+4);  			// enable Rx&Tx	
    outl(0xc000,HW_UART4_CTRL2+8); 			// clear hw control	
}


void serial_uart2_init(void)
{
    set_pin_mux(0,6,2);		//UART4-TXD
    set_pin_mux(0,7,2);		//UART4-RXD
    set_pin_mux(18,0,2);		//UART4-TXD
    set_pin_mux(18,1,2);		//UART4-RXD

    outl(1<<13,HW_AHBCLKCTRL0+4);			//UART4 ENABLE bit15---UART4
    outl(1,HW_UART2CLKDIV);					//UART4 devided by x from PLL	
	outl(0x1<<13,HW_AHBCLKCTRL0+8);			//UART4 clk gate  //eye check    
    outl(0x1<<13,HW_AHBCLKCTRL0+4);			//UART4 clk gate  //eye check	     
    outl(0x1<<13,HW_PRESETCTRL0+8);			//UART4 reset    //eye check    
    outl(0x1<<13,HW_PRESETCTRL0+4);			//UART4 reset    //eye check    
    outl(0xC0000000,HW_UART2_CTRL0+8);		//CLEAR UART4 SFTRST AND CLKGATE	
    outl(0x00062070,HW_UART2_LINECTRL);	// set bandrate 115200 12M	
    outl(0x301,HW_UART2_CTRL2+4);  			// enable Rx&Tx	
    outl(0xc000,HW_UART2_CTRL2+8); 			// clear hw control	
}


void EMI_init()
{
#ifdef	ASM9260_SDRAM_SIP

	outl(1<<6,HW_AHBCLKCTRL0+4);     //enable EMI CLK

	outl((1 << 3) | (12 << 5) | (8 << 9) | (0 << 13),0x80700000 );
	outl((inl(0x8004034C) & 0xFFF1E0FF) | (5 << 8) | (1 << 17) | (2 << 18),0x8004034C);
	outl(0x20000000,0x80700014 + 4*2);
	outl(0xc,0x80700054 + 4*2);  
	outl(0x024996d9,0x80700004);
	outl((inl(0x80700004) & 0xFFFFFFFC) | (1 << 0),0x80700004);
	outl(0x00542b4f,0x80700094);
	outl(inl(0x8070000C) | (1 << 9) ,0x8070000C);	
#else

	/* pin assign */
	int i,j;

	set_pin_mux(11, 7, 6);

	for(i=13;i<=17;i++)
	{
		for(j=0;j<8;j++)
		{
			set_pin_mux(i,j,6);
		}
	}

	outl(1<<6,HW_AHBCLKCTRL0+4);     //enable EMI CLK

	outl((1 << 3) | (12 << 5) | (8 << 9) | (0 << 13),0x80700000 );
	outl((inl(0x8004034C) & 0xFFF1E0FF) | (5 << 8) | (0 << 17) | (2 << 18),0x8004034C);
	outl(0x20000000,0x80700014 + 4*2);
	outl(0xc,0x80700054 + 4*2);  
	outl(0x024996d9,0x80700004);
	outl((inl(0x80700004) & 0xFFFFFFFC) | (1 << 0),0x80700004);
	outl(0x00542b4f,0x80700094);
	outl(inl(0x8070000C) | (1 << 9) ,0x8070000C);	
#endif
}

void base_init(){

	  EMI_init();

}
void delay(int n){
    int i;
    while (n-->0) {
        for(i=0;i<1000;i++);
    }
}
/*---------------Main --------------------------------*/

void clk_init(void)
{
	int i, wait_lock=0;

	delay(1000);//wait 100us
	outl(1<<2,HW_AHBCLKCTRL0+4); //open sram clk
	outl(1<<8,HW_AHBCLKCTRL1+4);//open icoll clk
	outl(3<<9,HW_AHBCLKCTRL0+4); //open DMA0,1 clk
	outl(inl(HW_PDRUNCFG)&0xFFFFFFFB,HW_PDRUNCFG);//open syspll power
	outl(0x2,HW_CPUCLKDIV); //CPU=PLLCLK/1
	outl(0x2,HW_SYSAHBCLKDIV); //HCK=CPUCLK/2
	outl(480,HW_SYSPLLCTRL); //480MHz
	while((inl(HW_SYSPLLSTAT)&0x1)==0x0)//wait syspll lock
	{
		if (wait_lock++ > 0x100000)         
			{
				break;
			}
	}
	for(i=0;i<0x10000;i++);
	outl(0x1,HW_MAINCLKSEL); //select syspll to main clk
	outl(0x0,HW_MAINCLKUEN);
	outl(0x1,HW_MAINCLKUEN);

}

int main(){
	int i;
	clk_init();
	base_init();

/* RD:byzsma on: Mon, 18 May 2015 13:14:25 +0800
 */
    set_pin_mux(5,0,0);
    set_pin_mux(5,1,0);
    set_pin_mux(5,2,0);
    set_pin_mux(5,3,0);
    set_GPIO(5,0);
    set_GPIO(5,1);
    set_GPIO(5,2);
    set_GPIO(5,3);
// End of RD:byzsma
	serial_uart4_init();
	//puts("\nFor  NAND Loading System Loader ...\n");
	extern struct device dev_nand;

	nand_probe(&dev_nand);

    // detect rootfs partition
    unsigned int rootfs_partition_idx = 0;
    unsigned int rootfs_timestamp = 0;
    unsigned int *rootfs_addr;
    int rootfs_cnt;
    for (i=0; i<2; i++) {
        nand_read(get_nand_info(),3,0x20800000,i == 0 ? 0x200000 : 0x900000,0x100000);
        rootfs_addr = 0x20800000;
        for (rootfs_cnt = 0; rootfs_cnt < 8; rootfs_cnt++) {
            if (*rootfs_addr == 0x28cd3d45) {
                if (*(rootfs_addr + 15) > rootfs_timestamp) {
                    rootfs_partition_idx = i;
                    rootfs_timestamp = *(rootfs_addr + 15);
                }
                break;
            }
            rootfs_addr += 0x20000;
        }
    }

#ifdef ASM9260_LOAD_LINUX
	//puts ("\nNAND Loading Linux .... \n");
	nand_read(get_nand_info(),3,0x20680000,0x20000,0x40000);//load Uboot to 0x20680000 in nand 0x20000
	nand_read(get_nand_info(),3,0x20800000,0x60000,0x1A0000);//load linux to 0x20800000 in nand 0x100000
	*(unsigned int*)0x206C0000 = rootfs_partition_idx;
	//puts("Jumping to UBoot...\n");
	((ulong (*)(int, char *[]))0x20680000) (0, NULL);//0x20680000 is begin of UBOOT in sdram
#else
	puts ("\nNAND Loading Alpos .... \n");
	nand_read(get_nand_info(),3,0x20008000,0x10000,0x80000);//load alpos to 0x20008000 in nand 0x10000
	puts("Jumping to Alpos...\n");
	((ulong (*)(int, char *[]))0x20008000) (0, NULL);//0x20008000 is begin of Alpos in sdram
#endif
}


