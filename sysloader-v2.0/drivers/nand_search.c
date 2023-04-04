
/*
Alpha Scale AS3310X BootLoader H-BOOT
He Yong, AlpScale Software Engineering, hoffer@sjtu.org
 
AS3310 Boot Loader Nand table Source file

------------------- Version 2.0  ----------------------
Support Big Capacity(>= 2Gb) Nand Flash by 4th DevID
 He Yong 2007-05-15
 
------------------- Version 1.0  ----------------------
Create File, Support 26 types of Nand Flash
 He Yong 2006-11-28

*/
#include <common.h>
#include <drivers/nand_search.h>
#include <drivers/flash.h>

nand_info alp_nandinfo;

/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct nand_flash_dev nand_flash_ids[] = {
//	{"16MiB 1,8V 8-bit",	0x33, 512, 16, 0x4000, 0},
//	{"16MiB 3,3V 8-bit",	0x73, 512, 16, 0x4000, 0},
//	{"16MiB 1,8V 16-bit",	0x43, 512, 16, 0x4000, NAND_BUSWIDTH_16},
//	{"16MiB 3,3V 16-bit",	0x53, 512, 16, 0x4000, NAND_BUSWIDTH_16},
//
//	{"32MiB 1,8V 8-bit",	0x35, 512, 32, 0x4000, 0},
//	{"32MiB 3,3V 8-bit",	0x75, 512, 32, 0x4000, 0},
//	{"32MiB 1,8V 16-bit",	0x45, 512, 32, 0x4000, NAND_BUSWIDTH_16},
//	{"32MiB 3,3V 16-bit",	0x55, 512, 32, 0x4000, NAND_BUSWIDTH_16},
//
//	{"64MiB 1,8V 8-bit",	0x36, 512, 64, 0x4000, 0},
//	{"64MiB 3,3V 8-bit",	0x76, 512, 64, 0x4000, 0},
//	{"64MiB 1,8V 16-bit",	0x46, 512, 64, 0x4000, NAND_BUSWIDTH_16},
//	{"64MiB 3,3V 16-bit",	0x56, 512, 64, 0x4000, NAND_BUSWIDTH_16},
//
//	{"128MiB 1,8V 8-bit",	0x78, 512, 128, 0x4000, 0},
//	{"128MiB 1,8V 8-bit",	0x39, 512, 128, 0x4000, 0},
//	{"128MiB 3,3V 8-bit",	0x79, 512, 128, 0x4000, 0},
//	{"128MiB 1,8V 16-bit",	0x72, 512, 128, 0x4000, NAND_BUSWIDTH_16},
//	{"128MiB 1,8V 16-bit",	0x49, 512, 128, 0x4000, NAND_BUSWIDTH_16},
//	{"128MiB 3,3V 16-bit",	0x74, 512, 128, 0x4000, NAND_BUSWIDTH_16},
//	{"128MiB 3,3V 16-bit",	0x59, 512, 128, 0x4000, NAND_BUSWIDTH_16},
//
//	{"256MiB 3,3V 8-bit",	0x71, 512, 256, 0x4000, 0},
//	{"256MiB 3,3V 8-bit",	0x31, 512, 256, 0x4000, 0},
//
//	/*
//	 * These are the new chips with large page size. The pagesize and the
//	 * erasesize is determined from the extended id bytes
//	 */
//
//	/*512 Megabit */
//	{"64MiB 1,8V 8-bit",	0xA2, 0,  64, 0, LP_OPTIONS},
	{"64MiB 3,3V 8-bit",	0xF2, 0,  64, 0, LP_OPTIONS},
//	{"64MiB 1,8V 16-bit",	0xB2, 0,  64, 0, LP_OPTIONS16},
//	{"64MiB 3,3V 16-bit",	0xC2, 0,  64, 0, LP_OPTIONS16},

	/* 1 Gigabit */
//	{"128MiB 1,8V 8-bit",	0xA1, 0, 128, 0, LP_OPTIONS},
	{"128MiB 3,3V 8-bit",	0xF1, 0, 128, 0, LP_OPTIONS},
//	{"128MiB 1,8V 16-bit",	0xB1, 0, 128, 0, LP_OPTIONS16},
//	{"128MiB 3,3V 16-bit",	0xC1, 0, 128, 0, LP_OPTIONS16},

	/* 2 Gigabit */
//	{"256MiB 1,8V 8-bit",	0xAA, 0, 256, 0, LP_OPTIONS},
	{"256MiB 3,3V 8-bit",	0xDA, 0, 256, 0, LP_OPTIONS},
//	{"256MiB 1,8V 16-bit",	0xBA, 0, 256, 0, LP_OPTIONS16},
//	{"256MiB 3,3V 16-bit",	0xCA, 0, 256, 0, LP_OPTIONS16},
//
//	/* 4 Gigabit */
//	{"512MiB 1,8V 8-bit",	0xAC, 0, 512, 0, LP_OPTIONS},
	{"512MiB 3,3V 8-bit",	0xDC, 0, 512, 0, LP_OPTIONS},
//	{"512MiB 1,8V 16-bit",	0xBC, 0, 512, 0, LP_OPTIONS16},
//	{"512MiB 3,3V 16-bit",	0xCC, 0, 512, 0, LP_OPTIONS16},
//
//	/* 8 Gigabit */
//	{"1GiB 1,8V 8-bit",	    0xA3, 0, 1024, 0, LP_OPTIONS},
    {"1GB 3,3V 8-bit",	    0xD3, 0, 1024, 0, LP_OPTIONS},
//	{"1GiB 1,8V 16-bit",	0xB3, 0, 1024, 0, LP_OPTIONS16},
//	{"1GiB 3,3V 16-bit",	0xC3, 0, 1024, 0, LP_OPTIONS16},
//
//	/* 16 Gigabit */
//	{"2GiB 1,8V 8-bit",	    0xA5, 0, 2048, 0, LP_OPTIONS},
	{"2GB 3,3V 8-bit",	    0xD5, 0, 2048, 0, LP_OPTIONS},
//	{"2GiB 1,8V 16-bit",	0xB5, 0, 2048, 0, LP_OPTIONS16},
//	{"2GiB 3,3V 16-bit",	0xC5, 0, 2048, 0, LP_OPTIONS16},
// 
    /* 32 Gigabit */
     {"4GB 3,3V 8-bit",     0xD7, 0, 4096, 0, LP_OPTIONS},

     /* 64 Gigabit */        
     {"8GB 3,3V 8-bit",     0xD9, 0, 8192, 0, LP_OPTIONS},

	/*
	 * Renesas AND 1 Gigabit. Those chips do not support extended id and
	 * have a strange page/block layout !  The chosen minimum erasesize is
	 * 4 * 2 * 2048 = 16384 Byte, as those chips have an array of 4 page
	 * planes 1 block = 2 pages, but due to plane arrangement the blocks
	 * 0-3 consists of page 0 + 4,1 + 5, 2 + 6, 3 + 7 Anyway JFFS2 would
	 * increase the eraseblock size so we chose a combined one which can be
	 * erased in one go There are more speed improvements for reads and
	 * writes possible, but not implemented now
	 */
   // {"128MiB 3,3V 8-bit",	0x01, 2048, 128, 0x4000,0},

	{NULL,0,0,0,0,0},
};

/*
*	Manufacturer ID list
*/
#define NAND_MFD_ID_TYPES 9
struct nand_manufacturers nand_manuf_ids[] = {
 //   {NAND_MFR_TOSHIBA, "Toshiba"},
	{NAND_MFR_SAMSUNG, "Samsung"},
 //   {NAND_MFR_FUJITSU, "Fujitsu"},
 //   {NAND_MFR_NATIONAL, "National"},
 //   {NAND_MFR_RENESAS, "Renesas"},
 //   {NAND_MFR_STMICRO, "ST Micro"},
	{NAND_MFR_HYNIX, "Hynix"},
    {NAND_MFR_MICRON, "Micron"},
 //   {NAND_MFR_SANDISK, "San Disk"},
};
// 

char NCached dev_id[4];
char NCached dev_id_91[4];
/* Before this func , NAND_init() must be executed */
int NandSearch(nand_info * nand_i){
   int i;
   int found_91,found_maker,found_dev;
   nand_general_id4 fourth_id;
   u32 temp1,temp2;

   found_91 = 0;
   found_maker = 0;
   found_dev = 0;
   nand_i->maker_code = 0;

   Nand_Read_ID(nand_i,(ulong *)dev_id,4,0x90);
  // Nand_Read_ID(nand_i,(ulong *)dev_id_91,4,0x91);

   nand_i->maker_code = dev_id[0];
   nand_i->dev_id = dev_id[1];
 //  nand_i->third_id = dev_id[2];
   nand_i->fourth_id = dev_id[3];  // the 4th ID
 //  nand_i->extra_3 = dev_id_91[0];  // the 0x91 ID

   /* ============= Seach For Manufacture ID ============= */

  // as_puts(" Nand : ");

   if (nand_i->maker_code == 0) {
       /* 0x00 Maf code, Error! */
      // as_puts("No Nand\n");
       return -1;
   }

   for (i = 0; i < NAND_MFD_ID_TYPES ; i++) {
       if (nand_manuf_ids[i].id == nand_i->maker_code) {
           /* we found known maker id*/
           as_puts(nand_manuf_ids[i].name);
           found_maker = 1;
           break;
       }
   }

 //  if (!found_maker) {
 //      as_puts("Unknow Maker");
 //  }

  // as_putc('\t');
   /* ============= Seach For extra 0x91 ID ============= */

 //  for (i = 0; i < NAND_ID_91_TYPES ; i++) {
 //      if (id_91_table[i] == nand_i->extra_3) {
 //          /* we found 0x91 id*/
 //          found_91 = 1;
 //          as_puts("(0x91)\t");
 //      }
 //  }

   /* ============= Seach For Device ID & Decide chip size ============= */

 for (i = 0; ; i++) {
     if (nand_flash_ids[i].id == 0) break; /* seach to the end of table */
     if (nand_flash_ids[i].id == nand_i->dev_id) {
         /* we found known dev id*/
         as_puts(nand_flash_ids[i].name);
         /* decide chip size*/
         nand_i->chip_size = nand_flash_ids[i].chipsize;
      //   nand_i->page_size = nand_flash_ids[i].pagesize;
      //   nand_i->block_size = nand_flash_ids[i].erasesize;
        // nand_i->options = nand_flash_ids[i].options;
         found_dev = 1;
         break;
     }
 }
 
   /* ============= Decide page cycles & col cycles ============= */

 //  if ((found_dev)&&((nand_i->page_size == 512)||(found_91))) {
 //      /*  found a small page chip */
 //
 //      nand_i->page_size = 512;
 //      nand_i->page_shift = 9;
 //      nand_i->col_cycles = 1;
 //
 //      if (nand_i->block_size == 0) {
 //          /* a small page but big chip size */
 //          nand_i->block_size = 0x4000;
 //      }
 //
 //      if (found_91) {
 //          /* Toshiba 0x91 chip has double block size */
 //          nand_i->block_size <<= 1;
 //          nand_i->chip_size >>= 1;
 //          as_puts("\tHalf Actual Chip Size\t");
 //      }
 //
 //      nand_i->block_shift = get_shift( nand_i->block_size / nand_i->page_size  );
 //
 //      if ((nand_i->chip_size >>(nand_i->options & NAND_BUSWIDTH_16) ) > 32){
 //          /* if chip size is larger than 32 MB, we must use 3 row cycles */
 //          nand_i->row_cycles = 3;
 //      }
 //      else{
 //          nand_i->row_cycles = 2;
 //      }
 //  }
 //  else{
       /*   found a Large page chip, or the device is not in table.
            we decide the parameter from the 4th ID 
       */
       fourth_id.value = nand_i->fourth_id;

       nand_i->col_cycles = (u32)2;
       nand_i->page_shift = fourth_id.field.PageSize + 10;
       nand_i->block_shift = 6 + fourth_id.field.BlockSize - fourth_id.field.PageSize;

       if(!found_dev){ // not in table
        //   if (fourth_id.field.Orgniz == NAND_GENERAL_ORG_X8) { 
        //       nand_i->options = LP_OPTIONS;  // x8 Nand
        //   }
        //   else{   
        //       nand_i->options = LP_OPTIONS16; // x16 Nand
        //   }
           /* we assume big chip size */
           nand_i->row_cycles = (u32)3;
       }
       else{
           /* determine row_cycles by chip size */
           if ((nand_i->chip_size/* >>(nand_i->options & NAND_BUSWIDTH_16)*/ ) > 128) {
               /* big chip size */
               nand_i->row_cycles = (u32)3;
           }
           else{
               /* small chip size */
               nand_i->row_cycles = 2;
           }
       }
  // }
   nand_i->row_cycles = (nand_i->row_cycles) & 0xff;
   nand_i->col_cycles = (nand_i->col_cycles) & 0xff;
   temp1 = (nand_i->row_cycles);
   temp2 = (nand_i->col_cycles);

   nand_i->addr_cycles = (temp1+temp2) ;

   nand_i->page_size = (1<<nand_i->page_shift);
   nand_i->block_size = (1<<(nand_i->block_shift+nand_i->page_shift));
   nand_i->sector_per_page = (nand_i->page_size >> 9); // 512 Byte / Sector
   nand_i->page_per_block = (1<<nand_i->block_shift);
   nand_i->oob_size =  (nand_i->page_size>>5);
   nand_i->oob_buf = NULL;// nand_oob_buf;

   //as_puts("\t Row: 0x");as_puth(nand_i->row_cycles);
   //as_puts("\t Col: 0x");as_puth(nand_i->col_cycles);
   //as_puts("\t Addr: 0x");as_puth(nand_i->addr_cycles);

   //as_puts("Pagesize: 0x");as_puth(nand_i->page_size);
   //as_puts("\t Blocksize: 0x");as_puth(nand_i->block_size);

   return 0;
}


volatile uchar NCached ALIGN32 tmp_oob[128];
int nand_read(nand_info * nand_i,int read_type,char *target_addr,ulong addr,ulong length){
   // uchar oob[128];
    int length_left;
    int error;
    int correct = 0;
    char *target_oob;
    int n_read;

   // if (read_type == NAND_READ_RAW_OOB) { target_oob = target_addr;}
   // else 
   //     {
        target_oob = (char *)tmp_oob;
    //    }

    error = 0;
    /* Begin Read */
    length_left = length;
    while (length_left > 0) {
		if (((addr & ((1<<(nand_i->block_shift+nand_i->page_shift)) - 1)) == 0)&&(read_type!=NAND_READ_RAW_OOB)) { // a new block
			if (nand_block_is_bad(nand_i,addr>>(nand_i->block_shift+nand_i->page_shift)) < 0) {
				as_puts("Block ");
				as_puth(addr>>(nand_i->block_shift+nand_i->page_shift));
				as_puts(" is bad! skip.\n");
				
				addr += (1<<(nand_i->block_shift+nand_i->page_shift));
				continue;
			}
		}

      //  as_putc('=');
        n_read = AS3310_Nand_Read_Page(nand_i,addr>>nand_i->page_shift,target_addr,target_oob,read_type,length_left);
        if (n_read <= 0)
             {   n_read = nand_i->page_size;   error--;} 
        else
        {
                
           ;// correct++;//as_putc('a');
        }

        // ECC error
        addr += nand_i->page_size; //next page
        length_left -= n_read;
        target_addr += n_read; 
    }
    //puth(correct);
	return error;	
}



int nand_probe(struct device * dev){
    nand_info * nand_i;

    nand_i = get_nand_info();
    dev->priv_data =  nand_i;

    nand_i->chip = 0;
    nand_i->mode = NAND_MODE_16P512;
    return NAND_init(nand_i);
}

struct device dev_nand = {
    .name       = "nand",
    .dev_id     = 0,
    .probe      = nand_probe,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
  //  .read       = dev_nand_read,  
  //  .write      = dev_nand_write, 
  //  .ioctl      = dev_nand_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = NULL,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device(dev_nand);


