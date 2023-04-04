/*
Alpha Scale AS3310X Booter, H-BOOT
Zhao Haiyuan, AlpScale Software Engineering, zhaoy@alpscale.com.cn
 
------------------- Version 1.0  ----------------------
Create File, 
    Support Nand ECC 
    Zhao Haiyuan 2007-04-29

*/


#include <common.h>
#include <drivers/ecc.h>


volatile AS3310_DMA_PKG NCached ecc_pkg_table[ECC_MAX_PACKAGE];
int NCached error_code;

// void Ecc_Init()
// { //reset HWECC and APBH 
//     outl(0xc0000000,HW_HWECC_CTRL + 8);  //hwecc sftrst = 0         
//     outl(0xc0000000,HW_APBH_CTRL0 + 8);  //dma sftrst = 0   
//     outl(0x00000000,HW_APBH_CTRL1);  //dma ctrl1 irq=1;irq_en=1;
// }

//  /*
//  Ecc Encode. Encode the data from the address "datafrom" with the byte length of "Ecclth" eg 512.
//  Write the status code to "parityadd" and write the parity code to "parityadd+1" 
//  */
//  void EccEncode(char* datafrom, char* parityadd)
//  {
//  
//      int wait_dma;
//  
//      ecc_pkg_table[1].NEXT_PKG = (ulong)(&ecc_pkg_table[2].NEXT_PKG);
//      ecc_pkg_table[1].CTRL = 0x02001046; 
//      ecc_pkg_table[1].BUFFER = (ulong)datafrom;
//      ecc_pkg_table[1].CMD0 = 0x01c80042; //encode
//  
//      ecc_pkg_table[2].NEXT_PKG = (ulong)(&ecc_pkg_table[3].NEXT_PKG);
//      ecc_pkg_table[2].CTRL = 0x00040045;
//      ecc_pkg_table[2].BUFFER = (ulong)&error_code;
//  
//      ecc_pkg_table[3].NEXT_PKG = 0;
//      ecc_pkg_table[3].CTRL = 0x00090041; 
//      ecc_pkg_table[3].BUFFER = (ulong)(parityadd); //RsEncode+0x04; 512byte after 4 bytes of status adn 9 byte of parity
//  
//  /*begin the DMA and set sema*/
//  
//      outl(0x00000003,HW_APBH_CH0_SEMA); //3 
//      outl((ulong)&(ecc_pkg_table[1].NEXT_PKG),HW_APBH_CH0_NXTCMDAR);  //next cmd addr 3 cmd
//  
//      wait_dma = 0;
//      while ( (inl(HW_APBH_CH0_SEMA))&0x00ff0000 )
//      {
//          if ( wait_dma++ > MAX_DMA_WAIT )
//          {
//              puts("DMA Not Ready\n"); 
//              break;
//          }
//      }
//  
//  }


/*
Ecc Decode
decode the data from the address "datafrom" 
with the byte length of "Ecclth" eg 512.
based on the party code from parityadd
write the EccReport to "Report" 9*32 bit
*/
volatile int NCached Report[10];
int EccDecodeCheck( char* datafrom,char* parityadd)
{

    int index;
    int mask;
    int j;
    int wait_dma;


/*3 dma chain to Ecc decode. Produce Ecc Error Report*/
    ecc_pkg_table[4].NEXT_PKG = (ulong)(&ecc_pkg_table[5].NEXT_PKG);
    ecc_pkg_table[4].CTRL = 0x02001046; 
    ecc_pkg_table[4].BUFFER = (ulong)datafrom;
    ecc_pkg_table[4].CMD0 = 0x01c80053; //dncode

    ecc_pkg_table[5].NEXT_PKG = (ulong)(&ecc_pkg_table[6].NEXT_PKG);
    ecc_pkg_table[5].CTRL = 0x00090046;
    ecc_pkg_table[5].BUFFER = (ulong)(parityadd);//parity code;

    ecc_pkg_table[6].NEXT_PKG = 0;
    ecc_pkg_table[6].CTRL = 0x00240041; 
    ecc_pkg_table[6].BUFFER = (ulong)Report;

/*Start dma channel and sema*/
    outl(0x00000003,HW_APBH_CH0_SEMA);//sema
    outl((ulong)&(ecc_pkg_table[4].NEXT_PKG),HW_APBH_CH0_NXTCMDAR);  //next cmd addr     

    wait_dma = 0;
    while ( (inl(HW_APBH_CH0_SEMA))&0x00ff0000 )
    {
        if ( wait_dma++ > MAX_DMA_WAIT )
        {
            puts("DMA Not Ready\n"); 
            break;
        }
    }

/*
Ecc correct data. Based on the EccReport from "Report" Check the data from "datafrom". Check the parity code from "parityadd". 
*/
    if ( Report[0]==0 )
    {
        return(ECC_NOERROR); //no error
    }
    else if ( (Report[0] & (1<<31)) )
    { // All Ones
        as_putc('='); 
        return(ECC_ALLONES); 
    }
    else if ( (Report[0] & (1<<30)) )
    { // All Zero
        as_putc('-'); 
        return(ECC_ALLZEROES); 
    }
    else if ( (Report[0] & (1<<28)) != 0 )
    { // ==0x100403f7){
        as_putc('X'); // ecc error
        return(ECC_UNCORRECTABLE); //uncorrectable error. require irq to resend
    }
    else
    { // data error or parity error, correctable
        for ( j=1;j<9;j++ )
        {//9 dword of EccReport    
            if ( Report[j]==0 ) break;  // if Report==0, no error any more
            else
            {
                index = (Report[j]>>16);  //index locate the error
                mask = Report[j]&0x0000ffff; //mask correct the error

                if ( (index&0x8000)==0x8000 )
                { //parity error
                    index=index&0x0fff; //erase the fist bit of index and correct the parity
                    parityadd[(index<<1)-1] ^= ((char)(mask>>8));
                    parityadd[(index<<1)-2] ^= ((char)(mask));

                }
                else
                {//data error and correct them
                    datafrom[(index<<1)-1] ^= ((char)(mask>>8));
                    datafrom[(index<<1)-2] ^= ((char)(mask));
                }
            }   
        }
        return(ECC_CORRECTED); 
    }
}    

