//int Nint;

//#define delta 0x1
//#define des_bias 0x1

#define  MAC_CTRL  0x80038000
#define  ClkCtrl   0x80040004
#define  HclkDiv   0x80040030
#define  PLLCLK    0x800400d0
#define  Pinctrl   0x80018000
#define  Pinbank1up  0x80018120
#define  Pinbank3up  0x80018320
#define  Pinbank4up  0x800182f0
#define  Drive1      0x80018130
#define  Drive3      0x80018330


#define  MSB         0x80038020           //int *MSB=0x80001108;
#define  LSB	     0x80038030   //int *LSB	=0x8000110c;
#define  MDIO_CS     0x80038040       //int *MDIO_CS=0x80001110;
#define  TDP	     0x80038050   //int *TDP	=0x80001114;
#define  RDP	     0x80038060  
#define XCLK_GATE    0x80040050

#define DES_TX    0x40002000
#define	DES_TX_4  0x40002004
#define	DES_RX    0x40002400
#define	DES_RX_4  0x40002404

#include <common.h>


int init_mac_ctrl( )
{
	       // int *ClkCtrl = 0x80040004;  //set 180m
	       // int *HclkDiv = 0x80040030;
	       // int *PLLCLK = 0x800400d0;
	        
	        //int *Pinctrl = 0x80018000;
	       // 
	       // int *Pinbank1up = 0x80018120;
	       // int *Pinbank3up = 0x80018320;
	       // int *Pinbank4up = 0x800182f0;
	        
		//int num=0x0;
		//int *MACBase = 0x80038000;
           //     int *CR =0x80038000;           //int *CR =0x80001100;
                //int *CR_set =0x80038004; 
		//int *SIR	=0x80038010;   //int *SIR	=0x80001104;
		//int *MSB=0x80038020;           //int *MSB=0x80001108;
		//int *LSB	=0x80038030;   //int *LSB	=0x8000110c;
		//int *MDIO_CS=0x80038040;       //int *MDIO_CS=0x80001110;
		//int *TDP	=0x80038050;   //int *TDP	=0x80001114;
		//int *RDP	=0x80038060;   //int *RDP	=0x80001118;
		
		//int *XCLK_GATE  = 0x80040050;
   
//		int *data_a= 0x40003000;      
		//int *des_TX= 0x40002000;
		//int *des_TX_4 = 0x40002004;
		//int *des_RX= 0x40002400;
		//int *des_RX_4 = 0x40002404;
		

		//int data_bias=0x0;
		
		
		//int *data_rec =0x40003800;
                
		//int i;
		
	//pin -> mac	
	outl(0x00000000,(int*)Pinctrl );
	outl(0x00000000,(int*)Pinbank1up );
	outl(0x00000000,(int*)Pinbank3up );	
	outl(0x00000000,(int*)Pinbank4up );
	outl(0xffffffff,(int*)Drive1);
	outl(0xffffffff,(int*)Drive3);
	
	
        // clk ctrl
 //       RegWrite((int *)PLLCLK, 0x0000508f);	//180
 //       RegWrite((int *)HclkDiv, 0x00000002);	
 //       RegWrite((int *)ClkCtrl, 0x00008000);	
	//*XCLK_GATE = 0x40000000;
	outl(0x40000000,(int*)XCLK_GATE);
	
	//outl(0x00300001,(int*)MDIO_CS);
		
	
//2005/3/29 pm 18:19
	//computer ID 0x0030487019ec	DA
	//FPGA ID 0x006a27d5e3b1		SA

	//3-30
	/*
	*(data_a+data_bias)=0xe659e8d5; 
	data_bias+=delta;
	*(data_a+data_bias)=0x000a006a; 
	data_bias+=delta;
	*(data_a+data_bias)=0x27d5e3b1;  
	data_bias+=delta;
	*(data_a+data_bias)=0x00801234;
	for(num=0x0;num<0x3;num++){
		data_bias+=delta;
		*(data_a+data_bias)=0x12345678;
	}
*/
	//set up tramsmit descripter to ram from the address 0x40000000
	//*des_TX=0x3880;  //enable irq
	outl(0x0000384a,DES_TX);
	
	//*(des_TX+des_bias)=0x40003000;
	// *des_TX_4 = 0x40003000;   //SRAM
	// *des_TX_4 = 0x00000000;   //nor   
	// *des_TX_4 = 0x20000000;   //sdram  
	//*des_TX_4 = 0xffff0138;
	outl(0x40000000,DES_TX_4);
	// *(des_TX_4+des_bias)=0x00000000;
	// *(des_TX_4+des_bias+des_bias)=0x00000000;

	

	//set up receive descripter to ram from the address 0x40000200
	//*des_RX=0x3880;  //enable irq
	outl(0x0000384a,DES_RX);
	
	// *(des_RX+des_bias)=0x40003800;
	 //*des_RX_4=0x40003800;//sram
	 outl(0x40003000,DES_RX_4);
	// *des_RX_4=0x40000000;
	//*des_RX_4=0x00001000;//nor  
	//*des_RX_4=0x20001000;//sdram
	// *(des_RX_4+des_bias)=0x40003800;
	// *(des_RX_4+des_bias+des_bias)=0x40003800;


	
	/*
	31 - 10: Base address to the transmitter descriptor table.Not Reset.
	9 - 3: Pointer to individual descriptors. Automatically incremented by the Ethernet MAC.
	2 - 0: Reserved. Reads as zeroes.
	*/
	outl(0x40002000,(int*)TDP);
        
	outl(0x40002400,(int*)RDP);

	// provided the mac address is 006a27d5e3b1
	outl(0x00000060,(int*)MSB);
	outl(0x487019EC,(int*)LSB);
	
	outb(0xee,0x40003000);//for test
	
	
	return 0;

}

int mac_send(){    
	
	int test_ctrl;   
	
	outl(0x0000384a,DES_TX);
	
	
	outl(0x40000000,DES_TX_4);
	
	outl(0x0000384a,DES_RX);
	
	
	 outl(0x40003000,DES_RX_4);
	
	
	outl(0x40002000,(int*)TDP);
        
	outl(0x40002400,(int*)RDP);

	// provided the mac address is 006a27d5e3b1
	outl(0x00000060,(int*)MSB);
	outl(0x487019EC,(int*)LSB);
	
	outb(0xee,0x40003000);//for test
	
     // *CR=0x0000003e; //enable irq tx
     printf("1.\n");
      outl(0x00000035,(int*)MAC_CTRL);
      printf("start to send.\n");
      
  /*    
      //test test
      
      test_ctrl=inl((int*)MAC_CTRL);
      while(test_ctrl!=0x00000030){
      	test_ctrl=inl((int*)MAC_CTRL);
        printf("#");
        
}
     printf("\nready !\n");
     outl(0x00000032,(int*)MAC_CTRL);
*/
    
     //   while(1){}
	return 0;
}

void mac_isr()
{
	       int vector;
	       
	       
	       //outl(0x00000001,(int *)(MAC_CTRL+4));//enable irq rx
	       outl(0x00000032,(int *)MAC_CTRL);
	       printf("ready to receive.\n");
	       
	       
	       
	       outl(0x00400000,0x800001E4);   //54 pirq
	      vector = inl(0x80000000);
	       outl(0x00000000,0x800001E8);
	     //  outb(0xee,0x40003000);//for test
}

BOOT_CMD(macinit,init_mac_ctrl,
         " init mac ",
         "mactest"); 
         
BOOT_CMD(send,mac_send,
         " a ping test ",
         "mactest"); 
         