
/*
Alpha Scale AS3310X BootLoader H-BOOT
He Yong, AlpScale Software Engineering, hoffer@sjtu.org
 
AS3310 Boot Loader Flash Source file

------------------- Version 1.2  ----------------------
Modify Nand Flash driver for DMA operation
 He Yong 2006-11-06

------------------- Version 1.1  ----------------------
Support Nor Flash
 Zhang Bo 2006-11-10

------------------- Version 1.0  ----------------------
Create File, Support Nand Flash
 He Yong 2006-11-06

*/

#include <common.h>
#include <drivers/nand_search.h>
#include <drivers/flash.h>
#include <drivers/ecc.h>


#define CONFIG_NAND_WRITE 0

volatile AS3310_DMA_PKG NCached nand_dma_pkg[NAND_PKG_NUM];

volatile int NCached nand_command;
char NCached nand_tmp_addr[8];
volatile uchar NCached ALIGN4 temp_ecc[12];
char NCached nand_ecc_tmp16[16];
volatile int NCached tmp_status;

char NCached ALIGN4 oob_buf_rw[128];

#if CONFIG_NAND_USE_OWN_BUFFER
volatile uchar NCached ALIGN32 nand_own_buf[4096];
volatile uchar NCached ALIGN32 nand_own_oob[128];
#endif //CONFIG_NAND_USE_OWN_BUFFER


void start_nand_dma_routine( nand_info * nand_i, ulong addr,int pkg_num){
int wait_dma;

    outl(addr ,HW_APBH_CH4_NXTCMDAR + (0x70*nand_i->chip));
    outl(pkg_num ,HW_APBH_CH4_SEMA + (0x70*nand_i->chip));

   // wait_dma = 0;
    while (IS_DMA_NAND_COMPLETE()==0) {
       // if (wait_dma++ > MAX_RBB_WAIT) {
       //     puts("DMA No Ready\n"); 
       //     break;
       // }
    }
}


int nand_send_cmd(nand_info * nand_i, char cmd){
    nand_command = cmd;
    /* === CMD 0x30 0x10 or 0xD0 === */
    nand_dma_pkg[0].CTRL = 0x000110c2;
    nand_dma_pkg[0].BUFFER = (ulong)(&nand_command);
    nand_dma_pkg[0].CMD0 = 0x20c20001 + (nand_i->chip<<20);

    start_nand_dma_routine( nand_i,(ulong)(&(nand_dma_pkg[0].NEXT_PKG)),1); // do the chain
    return 0;
}

int nand_wait_rbb(nand_info * nand_i){

   // delay(0x10000);
    /* === Wait rbb === */
    nand_dma_pkg[0].CTRL = 0x000010e0 ;
    nand_dma_pkg[0].BUFFER = (ulong)(&nand_command);
    nand_dma_pkg[0].CMD0 = 0x2bc00000 + (nand_i->chip<<20) ; 

    start_nand_dma_routine( nand_i,(ulong)(&(nand_dma_pkg[0].NEXT_PKG)),1); // do the chain
    return 0;
}


int nand_send_cmd_and_wait(nand_info * nand_i, char cmd){
    nand_command = cmd;
    /* === CMD 0x30 0x10 or 0xD0 === */
    nand_dma_pkg[0].CTRL = 0x000110c6;
    nand_dma_pkg[0].BUFFER = (ulong)(&nand_command);
    nand_dma_pkg[0].CMD0 = 0x20c20001 + (nand_i->chip<<20);
    nand_dma_pkg[0].NEXT_PKG =(ulong)(&(nand_dma_pkg[1].NEXT_PKG));

    /* === Wait rbb === */
    nand_dma_pkg[1].CTRL = 0x000010e0 ;
    nand_dma_pkg[1].BUFFER = (ulong)(&nand_command);
    nand_dma_pkg[1].CMD0 = 0x2bc00000 + (nand_i->chip<<20) ; 

    start_nand_dma_routine( nand_i,(ulong)(&(nand_dma_pkg[0].NEXT_PKG)),2); // do the chain
    return 0;
}

int nand_send_addr(nand_info * nand_i, char * addr,int cycles){

    /* === Send addr === */
    nand_dma_pkg[0].CTRL = 0x000010c2 + (cycles<<16);
    nand_dma_pkg[0].BUFFER = (ulong)addr;
    nand_dma_pkg[0].CMD0 = 0x20c40000 + (nand_i->chip<<20) + cycles ;  // n cycles addr
    
    start_nand_dma_routine( nand_i,(ulong)(&(nand_dma_pkg[0].NEXT_PKG)),1); // do the chain
    
    return 0;
}

int nand_read_buf(nand_info * nand_i, char * buf, int len){

    nand_dma_pkg[0].CTRL = 0x000010c1 + (len<<16);
    nand_dma_pkg[0].BUFFER = (ulong)buf;
    nand_dma_pkg[0].CMD0 = 0x21c00000 + (nand_i->chip<<20) + (len); // read xxxx bytes
    start_nand_dma_routine( nand_i,(ulong)(&(nand_dma_pkg[0].NEXT_PKG)),1);
    return len;
}



void Nand_Read_ID(nand_info * nand_i,ulong* dev_id,int n_byte,uchar cmd){

  /* cmd 0x90 or other */
    nand_send_cmd(nand_i,cmd);

  /* addr 0x00  */
    nand_tmp_addr[0] = 0;
    nand_send_addr(nand_i,nand_tmp_addr,1);

  /* read n bytes  */
    nand_read_buf(nand_i,(char *)dev_id, n_byte);
}

/*
AS3310 Nand Read Page
Using Ecc Check
type 0: raw read data               ret=2048
type 1: raw read oob                ret=64
type 2: data + oob + ecc (boot mode)        ecc byte = spare[(2+16*i)-(10+16*i)]    ret=2048
type 3: data + oob + ecc (kernel mode)      ecc byte = spare[28-63]                 ret=2048
*/

int AS3310_Nand_Read_Page(nand_info * nand_i,int page,char * buffer,char * oob,int type,int len){
    int data_read,oob_read;
    int i,j;
    int ret;
    char * buf_ptr,* oob_ptr,* oob_buf2;
    int sector;    // sector num for big page
    int ecc_error_code;
    int error;    
    int oob_512_ex;
    ulong cmd_read;  

#if CONFIG_NAND_USE_OWN_BUFFER
    char * org_buffer;
    org_buffer = buffer;
    buffer = (char *)nand_own_buf;
#endif //CONFIG_NAND_USE_OWN_BUFFER

    if (len > nand_i->page_size) {   len = nand_i->page_size;  }
    //printf("Nand Read page %d, buffer 0x%08x, oob 0x%08x, type %d\n",page, buffer, oob, type);

   // oob_read = 0; data_read = 0; ret = nand_i->page_size;
   // if (type == NAND_READ_RAW_DATA) {data_read = len; }
   // else if (type == NAND_READ_RAW_OOB) {oob_read = nand_i->oob_size; ret = nand_i->oob_size;}
   // else  {
        data_read = nand_i->page_size; oob_read = nand_i->oob_size;
   //     }
     
    error = 0;
    /* === Read CMD 0x00 === */

    cmd_read = 0;
    if ((nand_i->page_size == 512)&&(type == NAND_READ_RAW_OOB))  {   cmd_read = 0x50;  }

    if (nand_i->page_size == 512) {nand_send_cmd(nand_i,0xff);}

    nand_send_cmd(nand_i,cmd_read);

    /* === Col addr === */
    i = 0;
    nand_tmp_addr[i++] = 0;  // 1st col addr    
    if (nand_i->col_cycles > 1) {
        if (type == NAND_READ_RAW_OOB)  {nand_tmp_addr[i++] = (nand_i->page_size>>8) ; }
        else { nand_tmp_addr[i++] = 0;}  // 2nd col addr
    }    
    /* === Row addr === */
    for (j=0;i<nand_i->addr_cycles;) { // 1st,2nd,3rd row addr
        nand_tmp_addr[i++] = ((page>>((j++)*8)) & 0xff);
    }
    nand_send_addr(nand_i,nand_tmp_addr,nand_i->addr_cycles);

    /* === Send Read cmd and wait  === */
    if (nand_i->page_size > 512) {   nand_send_cmd_and_wait(nand_i,0x30);  }
    else{    nand_wait_rbb(nand_i);  }

    /* === Read Buf === */
    if (data_read) { 
        nand_read_buf(nand_i,buffer, data_read);
#if CONFIG_NAND_USE_OWN_BUFFER
#else
      //  IvalidDCacheRegion(buffer, data_read);
#endif// CONFIG_NAND_USE_OWN_BUFFER
    }
    
    /* === Read OOB Buf === */
    if (oob_read) { 
        //oob_buf2 = oob_buf_rw;
        oob_buf2 = oob;
        nand_read_buf(nand_i,oob_buf2, oob_read);  
     //   IvalidDCacheRegion(oob, oob_read);
    }

    /* ===== Do ECC Check ===== */
    if ( type == NAND_READ_PAGE_OS )
     {

         buf_ptr = buffer;
         oob_ptr = oob;

         // do each sector / one page
         for ( sector = 0; sector < nand_i->sector_per_page ; sector++ )
         {
           //  memcpy(nand_ecc_tmp16,oob + (nand_i->oob_size - nand_i->sector_per_page*9) + (sector*9),9);
           //  oob_ptr = nand_ecc_tmp16;

             ecc_error_code = EccDecodeCheck(buf_ptr,oob + (nand_i->oob_size - nand_i->sector_per_page*9) + (sector*9)); 
             buf_ptr += 512; // Sector size

             if ( (ecc_error_code >= 0 )&&(ecc_error_code <= 64 ) )
             { /*  as_putc('0'+ ecc_error_code); */
             }
             else
             {
                 // as_putc('X'); // ecc error
                 error--;
             }
             // puts("\n");
         }
     }


__nand_read_page_finish:

    //memcpy(oob,oob_buf2,nand_i->oob_size);

#if CONFIG_NAND_USE_OWN_BUFFER
    memcpy(org_buffer,buffer,data_read);
#endif// CONFIG_NAND_USE_OWN_BUFFER

    if (error) { return error;}
    return ret;
}
   


int NAND_init(nand_info * nand_i){
  //  AS3310_GPMI * gpmi_ptr;
  //
  //  /* get pointer */
  //  gpmi_ptr = AS3310_GetBase_GPMI();
  //
  //  gpmi_ptr->CTRL0[0] = 0;  // Gate
  //  gpmi_ptr->CTRL1[0] = 4;  // choose nand mode
  //  gpmi_ptr->TIMING1[0] = 0xffff0000;  // wait max
  //
  //  outl((nand_i->chip +1)*0x00100000 ,HW_APBH_CTRL0);// nand dma channel X reset 

    return NandSearch(nand_i);

}
