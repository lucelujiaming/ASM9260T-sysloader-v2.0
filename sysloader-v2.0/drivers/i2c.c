

/*

Alpha Scale AS3310X IronPalms Console
zhangbo, AlpScale Software Engineering

*/

/*****************************
// standard i2c eeprom test progam 
*****************************/


#include <common.h>

#if CONFIG_I2C 
    
#define IS_DMA_I2C_COMPLETE() ((inl(HW_APBX_CH3_SEMA)&0x00ff0000)==0)

AS3310_DMA_PKG eepromr_dma_pkg[3];
AS3310_DMA_PKG eepromw_dma_pkg[3];
u32   i2c_head_w,i2c_head_r;


void Write_I2C_Head(ulong addr)
{
    int block;
    block = addr>>8;
	i2c_head_w = 0x000000a0 + 2*block + ((addr&0xff)<<8);   // 0x00,SUB1,SUB0,I2CADDR+W = 0xa0
	i2c_head_r = 0x000000a1 + 2*block;  //0x000000,I2CADDR+R = 0xa1
}

void RX_DMA_PACKET(ulong *buffer, int n_read)
{


    eepromr_dma_pkg[0].NEXT_PKG =  (ulong) ( &( eepromr_dma_pkg[1].NEXT_PKG ));
    eepromr_dma_pkg[1].NEXT_PKG =  (ulong) ( &( eepromr_dma_pkg[2].NEXT_PKG ));
    eepromr_dma_pkg[2].NEXT_PKG =  0;

    eepromr_dma_pkg[0].CTRL = 0x000210c6;
    eepromr_dma_pkg[0].BUFFER = (ulong)&i2c_head_w;
    eepromr_dma_pkg[0].CMD0 = 0x000b0002;
                 
    eepromr_dma_pkg[1].CTRL = 0x000110c6;
    eepromr_dma_pkg[1].BUFFER = (ulong)&i2c_head_r;
    eepromr_dma_pkg[1].CMD0 = 0x002b0001;
              
    eepromr_dma_pkg[2].CTRL = 0x000010c1 + (n_read<<16);
    eepromr_dma_pkg[2].BUFFER = (ulong)buffer;
    eepromr_dma_pkg[2].CMD0 = 0x02120000 + n_read;

		
}

void TX_DMA_PACKET(ulong *buffer,int n_write)
{

    eepromw_dma_pkg[0].NEXT_PKG =  (ulong) ( &( eepromw_dma_pkg[1].NEXT_PKG ));
                      
    eepromw_dma_pkg[0].CTRL = 0x000210c6;
    eepromw_dma_pkg[0].BUFFER = (ulong)&i2c_head_w;
    eepromw_dma_pkg[0].CMD0 = 0x002b0002;
                      
    eepromw_dma_pkg[1].CTRL = 0x000010c2 + (n_write<<16);
    eepromw_dma_pkg[1].BUFFER = (ulong)buffer;
    eepromw_dma_pkg[1].CMD0 = 0x00130000 + n_write;
}

void RX_DMA_START_I2C()
{
        
	outl(0x00000000,DMAX_CTRL0_ADDR);
	outl(0x00000003,DMAX_CH3_SEMA);
	outl((ulong) ( &( eepromr_dma_pkg[0].NEXT_PKG )),DMAX_CH3_NXTCMDAR);
	
}

void TX_DMA_START_I2C()
{
        
	outl(0x00000000,DMAX_CTRL0_ADDR);
	outl(0x2,DMAX_CH3_SEMA);
	outl((ulong) ( &( eepromw_dma_pkg[0].NEXT_PKG )),DMAX_CH3_NXTCMDAR);
	
}

void init_i2c(){
	outl(0xffffffc3,I2C_PIN);
    // I2C clear clock gate
	outl(0x00000000,I2C_CTRL0_ADDR);
    outl(0x40000000,HW_APBX_CTRL0_CLR);//clear the clk gate
    outl(0x80000000,HW_APBX_CTRL0_CLR);//clear the sft reset  
}


int do_eeprom_write(cmd_tbl_t *cmdtp,int argc,char * argv[])
{

	// ********** parameters ************
	ulong addr;
	ulong *buffer;
	int   nbyte;

    if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

	addr = TextToLong_TRL(argv[1]);
	buffer = (ulong *)TextToLong_TRL(argv[2]);
	nbyte = TextToLong_TRL(argv[3]);
	// **************************
	eeprom_write(addr,buffer,nbyte);
 
return 0;
}

int eeprom_write(ulong addr,ulong * buffer,int n_bytes){
int     retry;
int     n_write;

    n_write = n_bytes;

    while(n_bytes>0){
        retry = 0x100000;

        if (n_bytes > 16 ) {
            n_write = 16;
        }
        else n_write = n_bytes;

        Write_I2C_Head(addr);	   
        TX_DMA_PACKET(buffer,n_write);
        
        TX_DMA_START_I2C();

        addr = addr + 0x10;
        buffer = buffer + 0x4;
        n_bytes = n_bytes -16;

        delay(0x10000);

        while(IS_DMA_I2C_COMPLETE()==0){
            if (retry-- < 0) {
                puts("I2C Write time out\n");
                break;
            }
        }
        putc('#');
	}
    putc('\n');
    return 0;
}


int do_eeprom_read(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
	// *********  parameters  **********
	ulong addr;
	ulong *buffer;
	int   nbyte;

    if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}   

	addr = TextToLong_TRL(argv[1]);
	buffer = (ulong *)TextToLong_TRL(argv[2]);
	nbyte = TextToLong_TRL(argv[3]);
	// ************************************
	alp_eeprom_read(addr,buffer,nbyte);
return 0;
}


int alp_eeprom_read(ulong addr,ulong * buffer,ulong n_bytes){
int retry = 0x100000;

	Write_I2C_Head(addr);
	RX_DMA_PACKET(buffer, n_bytes);
	RX_DMA_START_I2C();

    while(IS_DMA_I2C_COMPLETE()==0){
        if (retry-- < 0) {
            puts("I2C Read time out\n");
            break;
        }
    }
return 0;
}


BOOT_CMD(eepromw,do_eeprom_write,
         " #eepromw ee_addr src_addr nbytes", 
         "write data to eeprom");
         
BOOT_CMD(eepromr,do_eeprom_read,
         " #eepromr ee_addr dest_addr nbytes",
         "read data from eeprom");  


#endif // CONFIG_I2C 
