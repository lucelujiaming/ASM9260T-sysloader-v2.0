#include <common.h>
#include <drivers/nand_search.h>
#include <drivers/flash.h>
#include <drivers/nftl_mount.h>
#include <drivers/nftl_core.h>

void MediaHeader_dump(nftl_mediaheader * header)
{
	printf("NFTL2K MediaHeader dump:\n");
	printf("vol_id    %s\n",   header->DataOrgID);
	printf("FirstPhysicalEUN %d\n",   header->first_nftlblock);
	//printk("FormattedSize %Ld\n",   header->FormattedSize);
	printf("EUNNumInAll %d\n",   header->nftlblock_nums);
	printf("EUNNumForApp %d\n",   header->appblock_nums);
	//printf("UnitSizeFactor %d\n",   header->UnitSizeFactor);
}

void chains_dump(nftl_info * info){
    int log_block;
    u16 phy_block;

    printf("\nChains dump: total %d logical blocks\n",info->lblock_nums);
    for (log_block = 0; log_block < info->lblock_nums; log_block++) {
        phy_block = info->EUNtable[log_block];
        if (phy_block < info->tblock_nums){
            printf("\nVUN (%d): ",log_block);
            while(phy_block < info->tblock_nums) {
                printf("->%d[%d]",phy_block,info->ffptable[phy_block]);
                phy_block = info->reptable[phy_block];
            }
        }
        
    }
    printf("\n");
}

void reptable_dump(nftl_info * info){
    int block;
    u16 next_block;

    printf("\nRepl Table:\n");
    for (block = 0; block < info->tblock_nums; block++) { 
        next_block = info->reptable[block];
        if(next_block == BLOCK_TAIL){
            printf("%d->TAIL, ",block);
        }
        if(next_block < info->tblock_nums){
            printf("%d",block);
            printf("->%d, ",next_block);
        }
    }
    printf("\n");
}

void revtable_dump(nftl_info * info){
    int block;
    u16 pre_block;

    printf("\nReverse Table:\n");
    for (block = 0; block < info->tblock_nums; block++) { 
        pre_block = info->revtable[block];
        if(pre_block == BLOCK_HEAD){
            printf("%d->HEAD, ",block);
        }
        if(pre_block < info->tblock_nums){
            printf("%d",block);
            printf("->%d, ",pre_block);
        }
    }
    printf("\n");
}

void EUNtable_dump(nftl_info * info){
    int log_block;
    u16 phy_block;

    printf("\nEUN Table:\n");
    for (log_block = 0; log_block < info->lblock_nums; log_block++) { 
        phy_block = info->EUNtable[log_block];
        if(phy_block < info->tblock_nums){
            printf("%d",log_block);
            printf("->%d, ",phy_block);
        }
    }
    printf("\n");
}

void ffptable_dump(nftl_info * info){
    int block;
    int page;

    printf("\nFirst Free Page Table:\n");
    for (block = 0; block < info->tblock_nums; block++) { 
        page = info->ffptable[block];
        if(page>0){
            printf("%d",block);
            printf("->%d, ",page);
        }
    }
    printf("\n");
}

char NCached aftl_write_oob[128];
int is_badblock(device_t * nandbase_t,nftl_info * tlinfo,int block){
    struct nand_read_ops read_ops;
    //char oobbuf[64];

    read_ops.page = block<<(tlinfo->block_shift);
    read_ops.data = NULL;
    read_ops.oob = (char *)tlinfo->oob_buf;
    nandbase_t->ioctl(nandbase_t,NAND_IOCTL_READ_OOB,(unsigned long)&read_ops);

    int ret = 0;
    if (tlinfo->oob_buf[0] != 0xff) {// bad block
        ret = -1;
    }
    return ret;
}

int mark_badblock(device_t * nandbase_t,nftl_info * tlinfo,int block){
    struct nand_write_ops write_ops;
    //char oobbuf[64];

    memset((char *)aftl_write_oob,0,6);

    write_ops.page = block<<(tlinfo->block_shift);
    write_ops.data = tlinfo->move_buf;
    write_ops.oob = aftl_write_oob;

    nandbase_t->ioctl(nandbase_t,NAND_IOCTL_ERASE_FORCE,(block << tlinfo->block_shift)); // erase block
    nandbase_t->ioctl(nandbase_t,NAND_IOCTL_WRITE_OOB,(unsigned long)&write_ops); // mark bad block
    return 0;
}

u16 find_read_EUN(nftl_info *tlinfo, u16 logblock, char page){
    u16 thisEUN;
    u16 readEUN;
    //int silly;

    //printf("page:%d.",page);
    thisEUN = tlinfo->EUNtable[logblock];
    //printf("VUC-%d:find first EUN-%d.",logblock,thisEUN);
    readEUN = 0xffff;
	//silly = MAX_LOOPS;
    
	while (thisEUN <= tlinfo->tblock_nums) {
        //printf("EUN-%d:first free page-%d.",thisEUN,tlinfo->ffptable[thisEUN]);
        if (tlinfo->ffptable[thisEUN] > page) {
            //printf("EUN-%d:first free page-%d",thisEUN,tlinfo->ffptable[thisEUN]);
            readEUN = thisEUN;
        }

		thisEUN = tlinfo->reptable[thisEUN];
        //printf("next EUN %d.",thisEUN);

      // if (!silly--) {
      //     printk(KERN_WARNING "Infinite loop in Virtual Unit Chain 0x%x\n",
      //            thisVUC);
      //     return readEUN;
      // }
	}
    //printf("find read EUN:%d",readEUN);
    return readEUN;
}

void copy_back(device_t * nandbase_t,nftl_info * tlinfo,u16 thisVUC,u16 preblock,u16 srcblock,u16 tarblock,int beginpage,int endpage){
    int thispage;
    int ret;
    char * buf;
	nftl_oob oob;

    buf = tlinfo->move_buf;

    aftl_dbg("AFTL_CopyBack: VUC %d from EUN %d to EUN %d, page (%d - %d), phy page: (%d - %d) - > (%d - %d)\n",
           thisVUC,srcblock, tarblock, beginpage,endpage - 1,
            ((srcblock+tlinfo->boot_block_nums)<<tlinfo->block_shift) + beginpage,
            ((srcblock+tlinfo->boot_block_nums)<<tlinfo->block_shift) + endpage - 1,
            ((tarblock+tlinfo->boot_block_nums)<<tlinfo->block_shift) + beginpage,
            ((tarblock+tlinfo->boot_block_nums)<<tlinfo->block_shift) + endpage - 1
           );

    memset(&oob, 0xff, sizeof(nftl_oob));
    oob.Status = oob.Status1 = PAGE_USED;

    for (thispage = beginpage; thispage < endpage; thispage++) {// read the data 
    
        ret = nftl_read(nandbase_t,tlinfo,(srcblock) * tlinfo->page_per_block + thispage,buf);

        if (ret < 0) {
            ret = nftl_read(nandbase_t,tlinfo,(srcblock) * tlinfo->page_per_block + thispage,buf);

            if (ret < 0){
                printf("AFTL_CopyBack: Read Error Twice, Fail at EUN %d Page %d phy_page %d.\n"
                       , srcblock
                       , thispage
                       , (srcblock + tlinfo->boot_block_nums) * tlinfo->page_per_block + thispage
                       );
            }
        }

        oob.free_page_pos = oob.free_page_pos1 = thispage+1;            

        if ((thispage == 0)||(thispage == tlinfo->page_per_block - 1)){
            // === this page is the most important one, because it has many more info ===
            oob.revblocknum = oob.revblocknum1 = preblock;   
            oob.logblocknum = oob.logblocknum1 = thisVUC;
            //oob.u.b.EraseMark = oob.u.b.EraseMark1 = ERASE_MARK;// not wear-info here 
        }

        nftl_write(nandbase_t,tlinfo,(tarblock) * tlinfo->page_per_block + thispage,buf, (char*)&oob);

    }
    //return thispage;
}

void fold_page_data(device_t * nandbase_t,nftl_info * tlinfo,u16 thisVUC,u16 secondlastEUN,u16 targetEUN,char startpage,char endpage){
    u16 thisEUN;
    char thisffpage,thispage;
    nftl_oob oob;

    thisEUN = secondlastEUN; /* copy from the target EUN's previous EUN*/

    while(thisEUN!=BLOCK_HEAD){
        thisffpage = tlinfo->ffptable[thisEUN]; /* get page position of thisEUN */
        if(thisffpage > startpage){
            if(thisffpage >= endpage){ /* thisEUN's page data is enough */
                copy_back(nandbase_t,tlinfo,thisVUC,BLOCK_HEAD,thisEUN,targetEUN,startpage,endpage);
                tlinfo->ffptable[targetEUN] = endpage; /* set targetEUN page position */
                break;
            }
            else{   /* thisEUN's page data is not enough, continue searching for the previous one */
                copy_back(nandbase_t,tlinfo,thisVUC,BLOCK_HEAD,thisEUN,targetEUN,startpage,thisffpage);
                tlinfo->ffptable[targetEUN] = thisffpage; /* set targetEUN page position */
                startpage = thisffpage;
            }
        }
        thisEUN = tlinfo->revtable[thisEUN];
    }
    
    if (thisEUN == BLOCK_HEAD) {

        /* there's not enough valid data for the target page of targetEUN, we must fill ZEROES */
        memset(tlinfo->move_buf,0x0,(1<<tlinfo->page_shift));

        oob.Status = oob.Status1 = PAGE_USED; // this page do not content valid data
        oob.revblocknum = oob.revblocknum1 = BLOCK_HEAD;  // this is a HEAD block 
        oob.logblocknum = oob.logblocknum1 = thisVUC;

        for ( thispage = startpage ; thispage < endpage ; thispage++){

            oob.free_page_pos = oob.free_page_pos1 = thispage+1;  
            nftl_write(nandbase_t,tlinfo,(targetEUN) * tlinfo->page_per_block + thispage,tlinfo->move_buf, (char*)&oob);
        }
        tlinfo->ffptable[targetEUN] = endpage; /* set targetEUN page position */
    }
}

u16 findfreeblock(nftl_info * tlinfo, int in_folding){
    u16 pot = tlinfo->LastFreeEUN;

	// === Normally, we force a fold to happen before we run out of free blocks completely 
	if ((!in_folding)&&(tlinfo->freeblock_nums < 4)) {
	    aftl_dbg("NFTL2K_findfreeblock: there are too few free EUNs\n");
		return 0xffff;
	}

	// === Scan for a free block
    do {
        if((tlinfo->revtable[pot] == BLOCK_FREE)&&(tlinfo->last_write_EUN != pot)){
            tlinfo->freeblock_nums -- ;
            tlinfo->LastFreeEUN = pot;
            return pot;
        }

		if (++pot >= tlinfo->tblock_nums)
			pot = 0;

    } while(pot != tlinfo->LastFreeEUN);

    return 0xffff;
}

u16 makefreeblock(device_t * nandbase_t,nftl_info * tlinfo,u16 * foldVUC){
	u16 longest_chain = 0;
	u16 chainlen = 0, thislen;
	u16 chain, EUN;

    *foldVUC = 0xffff;

	for (chain = 0; chain < tlinfo->lblock_nums; chain++) {
		EUN = tlinfo->EUNtable[chain];
		thislen = 0;

		while (EUN < tlinfo->tblock_nums) {
			thislen++;
			//printf("VUC %d reaches len %d with EUN %d\n", chain, thislen, EUN);
			EUN = tlinfo->reptable[EUN];
			if (thislen > 0xff00) {
				printf("Endless loop in Virtual Chain %d: Unit %x\n",chain, EUN);
			}
		}

		if (thislen > chainlen) {
			//printf("New longest chain is %d with length %d\n", chain, thislen);
			chainlen = thislen;
			longest_chain = chain;
		}
        if (thislen > CONFIG_AFTL_FOLD_THRESHOLD){// exceed the threshold break 
			longest_chain = chain;
            break;
        }
	}

	if (chainlen < 2) {
		printf("No Virtual Unit Chains available for folding.\n");
        chains_dump(tlinfo);
		return 0xffff;
	}

    *foldVUC =  longest_chain;
	return fold_chain (nandbase_t,tlinfo, longest_chain);
}

u16 find_write_EUN(device_t * nandbase_t,nftl_info *tlinfo, u16 logblock, char page, u16 * LastBlockNum){
    u16 lastEUN,secondlastEUN;
	unsigned int writeEUN;
    nftl_oob oob;
	//int silly, silly2 = 3;
    int chain_length;
    int write_page;
    u16 foldedVUC, folded_targetEUN;
    int is_fill_zero;

    foldedVUC = 0xffff;
    secondlastEUN = BLOCK_HEAD;
    * LastBlockNum = lastEUN = BLOCK_HEAD;
    chain_length = 0; // count for folding , if length exceed limits we do folding 
    
    memset(tlinfo->move_buf,0x0,(1<<tlinfo->page_shift));

	// === Scan the media to find a unit in the VUC which has a free space for the block in question.
	// ===This condition catches the 0x[7f]fff cases, as well as being a sanity check for past-end-of-media access
	writeEUN = tlinfo->EUNtable[logblock];
	//silly = MAX_LOOPS;
	while (writeEUN < tlinfo->tblock_nums) {
        //=== save last used EUN No.
        * LastBlockNum = writeEUN;
        secondlastEUN = lastEUN;
        lastEUN = writeEUN;
        if (tlinfo->ffptable[lastEUN] <= page) break;
		//=== Skip to next block in chain 
		writeEUN = tlinfo->reptable[writeEUN];
        chain_length++;
	}

    if (lastEUN < tlinfo->tblock_nums) {// not an empty chain 
        if (tlinfo->ffptable[lastEUN] <= page) {
            if (tlinfo->ffptable[lastEUN] == page) { // we can write directly to the lastEUN
                return lastEUN;
            }
            //=== we need to do some copys from pre pages
            if (secondlastEUN < tlinfo->tblock_nums) {// has valid data 
              //  is_fill_zero = 0;
              //  while (tlinfo->ffptable[secondlastEUN] <= page) {
              //
              //      if (tlinfo->revtable[secondlastEUN] > tlinfo->tblock_nums) {
              //          /* reach the head , fill ZERO */
              //          is_fill_zero = 1;
              //          lastEUN = secondlastEUN;
              //          oob.Status = oob.Status1 = PAGE_USED;
              //          for (write_page = tlinfo->ffptable[lastEUN]; write_page < page; write_page++) {
              //              // === write the data 
              //              oob.free_page_pos = oob.free_page_pos1 = write_page+1;    
              //              nftl_write(nandbase_t,tlinfo,(lastEUN) * tlinfo->page_per_block + write_page,tlinfo->move_buf, (char*)&oob);
              //          }
              //          break;
              //      }
              //      secondlastEUN = tlinfo->revtable[secondlastEUN];
              //  }
              //  if (!is_fill_zero) {
              //      lastEUN = tlinfo->reptable[secondlastEUN];
                    copy_back(nandbase_t,tlinfo,logblock,secondlastEUN,secondlastEUN,lastEUN,tlinfo->ffptable[lastEUN], page);
              //  }
            }
            else{// we have only one data block, so we need to fill ZERO 
                oob.Status = oob.Status1 = PAGE_USED;

                for (write_page = tlinfo->ffptable[lastEUN]; write_page < page; write_page++) {
                    // === write the data 
                    oob.free_page_pos = oob.free_page_pos1 = write_page+1;    
                    nftl_write(nandbase_t,tlinfo,(lastEUN) * tlinfo->page_per_block + write_page,tlinfo->move_buf, (char*)&oob);
                }
            }
            // === update free page table 
            tlinfo->ffptable[lastEUN] = page;
            return lastEUN;
        }
        else{
            // we need to copy all data from lastEUN to a new target block;
        }
    }

	//===We didn't find one in the existing chain, or there is no existing chain. 

	writeEUN = findfreeblock(tlinfo,0);

	if (writeEUN >= tlinfo->tblock_nums) {
		// === No free blocks,so we have to fold a chain to make room ===
		folded_targetEUN = makefreeblock(nandbase_t,tlinfo,&foldedVUC);

        if (foldedVUC == logblock){
            /* 
            if the folded chain is what we are now writing , 
            the lastEUN should changed to the folding target EUN
            */
            lastEUN = folded_targetEUN;
        }

		if (folded_targetEUN >= tlinfo->tblock_nums) {
			/* Ouch. This should never happen - we should
			   always be able to make some room somehow.
			   If we get here, we've allocated more storage
			   space than actual media, or our makefreeblock
			   routine is missing something.
			*/
			printf("AFTL: Cannot make free space! We're DEAD!! \n");
			return BLOCK_TAIL;
		}
        
        writeEUN = findfreeblock(tlinfo,0);
	}

    aftl_dbg("VUC %d find a new block -->%d to write\n",logblock,writeEUN);

	// === Insert the new block into the chain === 
    if (lastEUN < tlinfo->tblock_nums) {
        // ===has valid data 
        /* (do NOT termilate)
        // ===we have to termilate lastEUN's free page status
        if (tlinfo->ffptable[lastEUN] < tlinfo->page_per_block) {
            // === there's still space on that block
            // === write to the last page's oob
            //aftl_dbg("last page's oob\n");
            oob.Status = oob.Status1 = PAGE_IGNORE; // this last page do not content valid data
            oob.revblocknum = oob.revblocknum1 = BLOCK_FREE;  // this is NOT a HEAD block 
            oob.free_page_pos = oob.free_page_pos1 = tlinfo->ffptable[lastEUN]; 
            write_page = (lastEUN+1) * (tlinfo->page_per_block) - 1;
            nftl_write(nandbase_t,tlinfo,write_page ,tlinfo->move_buf, (char*)&oob);

        }
        */
        // === Copy page0 - page data to the new block 
        if (page > 0) copy_back(nandbase_t,tlinfo, logblock, lastEUN, lastEUN, writeEUN,  0, page );

        // === update links 
        tlinfo->reptable[lastEUN] = writeEUN;
        tlinfo->reptable[writeEUN] = BLOCK_TAIL;
        tlinfo->revtable[writeEUN] = lastEUN;

    }
    else{ // this is a new EUN in a new VUC 
        //aftl_dbg("this is a new EUN in a new VUC\n");
        int current_page;
        if (page != 0) {// make the link at page 0 
            oob.Status = oob.Status1 = PAGE_USED; // this page do not content valid data
            oob.revblocknum = oob.revblocknum1 = BLOCK_HEAD;  // this is a HEAD block 
            oob.logblocknum = oob.logblocknum1 = logblock;

            for (write_page = 0; write_page < page; write_page++) {// write the data 
                oob.free_page_pos = oob.free_page_pos1 = write_page+1; 
                aftl_dbg("%d_",write_page);

                current_page = (writeEUN) * (tlinfo->page_per_block) + write_page;
                nftl_write(nandbase_t,tlinfo,current_page ,tlinfo->move_buf, (char*)&oob);
            }
        }

        // === update links 
        tlinfo->EUNtable[logblock] = writeEUN;
        tlinfo->reptable[writeEUN] = BLOCK_TAIL;
        tlinfo->revtable[writeEUN] = BLOCK_HEAD;
    }

    // === update free page table 
    tlinfo->ffptable[writeEUN] = page;
	return writeEUN;

}

int writeback_lastbuf(device_t * nandbase_t,nftl_info * nftl_i){
    nftl_oob oob;
    int ret;
    u16 readEUN,thisEUN,tempEUN;
    int page;

    //aftl_dbg("wl");

    memset((char *)&oob, 0xff, sizeof(nftl_oob));
    oob.Status = oob.Status1 = PAGE_USED;
    oob.free_page_pos = oob.free_page_pos1 = nftl_i->last_write_page+1;
    
    // === check if we need read back the org page data
    if (nftl_i->write_buf_status != 0xf) {// yes we need do that 
        readEUN = find_read_EUN(nftl_i,nftl_i->last_write_VUC,nftl_i->last_write_page);

        if (readEUN < nftl_i->tblock_nums) {

            page = (readEUN * nftl_i->page_per_block) + nftl_i->last_write_page;
            ret = nftl_read(nandbase_t,nftl_i,page,nftl_i->move_buf);

            if (ret < 0){
                ret = nftl_read(nandbase_t,nftl_i,page,nftl_i->move_buf);
                if (ret < 0 ){
                printf("writeback_lastbuf: Cannot read page %d in VUC %d, EUN %d\n",
                       nftl_i->last_write_page,nftl_i->last_write_VUC,readEUN);
                return -1;
                }
            }
        }
        else{
            memset(nftl_i->move_buf,0x0,2048);
        }

        if ((nftl_i->write_buf_status & PAGE_SECTOR_0) == 0) { // sector0 none cached
            memcpy(nftl_i->write_buf,nftl_i->move_buf,512);
        }
        if ((nftl_i->write_buf_status & PAGE_SECTOR_1) == 0) { // sector1 none cached
            memcpy(nftl_i->write_buf+512,nftl_i->move_buf+512,512);  
        }
        if ((nftl_i->write_buf_status & PAGE_SECTOR_2) == 0) { // sector2 none cached
            memcpy(nftl_i->write_buf+1024,nftl_i->move_buf+1024,512);  
        }
        if ((nftl_i->write_buf_status & PAGE_SECTOR_3) == 0) { // sector3 none cached
            memcpy(nftl_i->write_buf+1536,nftl_i->move_buf+1536,512);  
        }

    }

    if (nftl_i->last_write_page == 0) {
        //=== the cached page is page-0, so we must write other info 
        oob.revblocknum = oob.revblocknum1 = nftl_i->last_pre_block;
        oob.logblocknum = oob.logblocknum1 = nftl_i->last_write_VUC;
        //oob.u.b.EraseMark = oob.u.b.EraseMark1 = ERASE_MARK;
    }

    /* if the block is written full, free the previous blocks */
    if (nftl_i->last_write_page == nftl_i->page_per_block - 1) {

        thisEUN = nftl_i->revtable[nftl_i->last_write_EUN];

        while ( thisEUN < nftl_i->tblock_nums ) {
            tempEUN = nftl_i->revtable[thisEUN];
            aftl_dbg("(%d) ", thisEUN);
            if (format_block(nandbase_t,nftl_i, thisEUN) < 0) {// could not erase : mark block as reserved
                nftl_i->reptable[thisEUN] = BLOCK_RESERVED;
                nftl_i->revtable[thisEUN] = BLOCK_RESERVED;
            } 
            else {// correctly erased : mark it as free 
                nftl_i->reptable[thisEUN] = BLOCK_FREE;
                nftl_i->revtable[thisEUN] = BLOCK_FREE;
                nftl_i->freeblock_nums++;
                nftl_i->ffptable[thisEUN] = 0;
            }
            thisEUN = tempEUN;
        }

        // === Make this the new start of chain for thisVUC 
      //  nftl_i->reptable[nftl_i->last_write_EUN] = BLOCK_TAIL;
        nftl_i->revtable[nftl_i->last_write_EUN] = BLOCK_HEAD;
        nftl_i->EUNtable[nftl_i->last_write_VUC] = nftl_i->last_write_EUN;

        oob.revblocknum = oob.revblocknum1 = BLOCK_HEAD;
    }

    // === just write that page
    nftl_write( nandbase_t,nftl_i,((nftl_i->last_write_EUN) * nftl_i->page_per_block) + nftl_i->last_write_page,
            nftl_i->write_buf,( char *) &oob);

//    if (ret < 0) { 
//        printf("Writing page failed,and we will mark this block as bad!");
//        //mark_badblock(nftl_i->nbase_info,nftl_i->last_write_EUN);
//        return -1;
//    }

    nftl_i->ffptable[nftl_i->last_write_EUN] = nftl_i->last_write_page+1;
    nftl_i->write_buf_status = 0;

    if (nftl_i->last_write_page == 0) {
        //chains_dump(nftl_i);
    }
    
    return 0;
}



int read_sector(nftl_info * nftl_i, u32 sector, char * buffer){
	u16 phyblock;
	u16 logblock = (sector >> (nftl_i->block_shift + 2)); // / ((int)nftl_i->page_per_block << 2); // logical block number
	int pageofs = (sector>>2)  & (nftl_i->page_per_block -1); //page number in a block
    int do_cache_ops = 0;
    device_t * nandbase_t;
    int dev_index;
    int ret,page;

    read_lock = 1;
    nandbase_t = device_get("nand",&dev_index);

    //printf("page in a chip:%d.page in a block:%d.page per block:%d.",(sector>>2),pageofs,nftl_i->page_per_block);

    /* we needn't check for write buffer , because the read buf has already beed updated by write_sector()*/
    // === read cache if this page is cached in write buf
    if ((nftl_i->last_write_VUC == logblock)&&(nftl_i->last_write_page ==pageofs)){
        if ((1<<(sector&0x3)) & nftl_i->write_buf_status) {
            // === if the target page is already cached in write_buf 
         //   printf("Cached read in write buffer.VUC: %d. The %dth sector number in the block.\n",
         //          logblock,(sector & ((nftl_i->page_per_block << 2)-1)));
            memcpy(buffer,nftl_i->write_buf + ((sector&0x3)*512),512);
            return 0;
        }
    }

     // === check if this page is already cached 
    if (nftl_i->read_buf_status > 0) {// already cached 
        if ((nftl_i->last_read_VUC == logblock)&&( nftl_i->last_read_page == pageofs)){
            //=== we visit the same page, so do cached write
            do_cache_ops = 1;
        }
    }

    // === read cache if this page is cached in read buf
    if (do_cache_ops) {
     //   printf("Cached Read in read buffer.VUC: %d. The %dth sector number in the block.\n",
     //            logblock,(sector & ((nftl_i->page_per_block << 2)-1)));
        memcpy(buffer,nftl_i->read_buf + ((sector&0x3)*512),512);
        return 0;
    }

    

    //=== update current page 
    nftl_i->last_read_VUC = logblock;
    nftl_i->last_read_page = pageofs;
    nftl_i->read_buf_status = 1;

    // === do read ===
    phyblock = find_read_EUN(nftl_i, logblock, nftl_i->last_read_page);

	if (phyblock >= nftl_i->tblock_nums) {
		// === the requested block is not on the media, return all 0x00 
        aftl_dbg("Read a non-data VUC: %d. The %dth sector in the block.\n", 
                logblock,(sector&((nftl_i->page_per_block << 2)-1)) );
		memset(buffer, 0, 512);
	} else {
        //printf("Non-Cached Read VUC %d in ENU %d. The %dth sector in the block.\n", 
        //        logblock,phyblock,(sector&((nftl_i->page_per_block << 2)-1)));

		page = (phyblock * nftl_i->page_per_block) + pageofs;
        ret = nftl_read(nandbase_t,nftl_i,page,nftl_i->read_buf);

        if (ret < 0){
            ret = nftl_read(nandbase_t,nftl_i,page,nftl_i->read_buf);
            if (ret < 0 ){
                printf("Error: Cannot read page %d in VUC %d, EUN %d\n",pageofs,logblock,phyblock);
                return -1;
            }
        }
        memcpy(buffer,nftl_i->read_buf + ((sector&0x3)*512),512);
	}
    read_lock = 0;
	return 0;
}

int write_sector(nftl_info * nftl_i, u32 sector, const char * buffer){
	u16 phyblock;
	u16 logblock = sector / ((int)nftl_i->page_per_block << 2); // logical block number
	int pageofs = (sector>>2)  & (nftl_i->page_per_block -1); //page number in a block
    u16 lastblocknum;
    int do_cache_ops = 0;
    device_t * nandbase_t;
    int dev_index;

    write_lock = 1;
    nandbase_t = device_get("nand",&dev_index);

    // === check if this page is already cached 
    if (nftl_i->write_buf_status > 0) {
        //=== already cached 
        if ((nftl_i->last_write_VUC == logblock)&&( nftl_i->last_write_page == pageofs)){
            //=== we visit the same page, so do cached write
            do_cache_ops = 1;
    //        printf("Cached write: ");
        }
        else{
            //=== we visit the different page, see if we need to write back the write_buf 
            writeback_lastbuf(nandbase_t,nftl_i);
        }
    }

    if(!do_cache_ops){
   //     printf("Non-cached write: ");
        // === it is not a cached write, so find which block should we write to 
        phyblock = find_write_EUN(nandbase_t,nftl_i, logblock, pageofs,&lastblocknum);

        // dprintk("Not a cached write dev_block %d, find writeEUN = %d\n", block,writeEUN);

        if (phyblock >= nftl_i->tblock_nums) {
            printf("NFTL2K_writeblock(): Cannot find block to write to\n");
            //=== If we still haven't got a block to use, we're screwed 
            return 1;
        }
        
        nftl_i->last_write_EUN = phyblock;
        nftl_i->last_pre_block = lastblocknum;

       // if(nftl2k->Last_Write_VUC == thisVUC){
       //     nftl2k->swtich_wrtie_vuc_target = 0;
       // }
       // else{
       //     nftl2k->swtich_wrtie_vuc_target = 1;
       // }

        nftl_i->last_write_VUC = logblock;
        nftl_i->last_write_page = pageofs;
  //      printf("Non-cached write: ");
   //     printf("Write VUC %d in EUN %d. The %dth sector in the block.\n",
   //              logblock,phyblock,(sector&((nftl_i->page_per_block << 2)-1))  );  
    }
    
    //=== write target data in write buffer, we don't handle cache write now === 
    memcpy(nftl_i->write_buf + ((sector&0x3)*512),buffer,512);

    //=== mark this buffer as write cached ===
    nftl_i->write_buf_status |= (1<<(sector&0x3));

    nftl_i->write_jif = jiffies;
    write_lock = 0;
   // if ((nftl_i->last_read_VUC == logblock)&&( nftl_i->last_read_page == nftl_i->last_write_page)){
   //     //=== we visit the same page, so update the read cache 
   //     memcpy(nftl_i->read_buf + ((sector&0x3)*512),buffer,512);
   //     return 0;
   // }
   // else return 0;
    return 0;
}

int do_aftlws(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    u32 sector = 0;
    char * datafrom;
    int index;
    int len;
    struct device * aftl;

    if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

    aftl = device_get("aftl",&index);
    if (aftl == NULL) {
        printf("No such device : aftl\n");
        return -1;
    }

    sector = TextToLong_TRL(argv[1]);
    datafrom = (char *)TextToLong_TRL(argv[2]);
    len = TextToLong_TRL(argv[3]);

    while(len>0) {
        aftl->write(aftl,sector,datafrom,1);
        sector ++;
        datafrom += 512;
        len -= 512;
    }
    //MemDisp_TRL(datafrom,32,4);
    //reptable_dump(&alp_nftlinfo);
    //revtable_dump(&alp_nftlinfo);
    //EUNtable_dump(&alp_nftlinfo);
    //ffptable_dump(&alp_nftlinfo);
    chains_dump((nftl_info * )aftl->priv_data);
    return 0;
}

int do_aftlrs(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    u32 sector = 0;
    uchar buffer[512];
    int index;
    char * tarbuf;
    struct device * aftl;
    nftl_info * nftl_i;

    if ((argc < 2)||(argc > 3)) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
    sector = TextToLong_TRL(argv[1]);
    if(argc==3){
        tarbuf = (char *)TextToLong_TRL(argv[2]);
    }
    else{
        tarbuf = buffer;
    }

    aftl = device_get("aftl",&index);
    if (aftl == NULL) {
        printf("No such device : aftl\n");
        return -1;
    }

    aftl->read( aftl , sector, tarbuf, 1);

    nftl_i = (nftl_info * )aftl->priv_data;
    printf("info address: 0x%08x\n",nftl_i);
    printf("total lblocks: %d\n",nftl_i->lblock_nums);

    MemDisp_TRL(tarbuf,512,4);
    //reptable_dump(&alp_nftlinfo);
    //revtable_dump(&alp_nftlinfo);
    //EUNtable_dump(&alp_nftlinfo);
    //ffptable_dump(&alp_nftlinfo);
   chains_dump(nftl_i);
    
    return 0;
}

int do_aftlformat(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    nftl_info * nftl_i;
    nftl_i = get_nftl_info();

    if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

    aftl_format(nftl_i);
    nftl_init(nftl_i);
    return 0;
}

int do_aftlinfo(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    nftl_info * nftl_i;

    nftl_i = get_nftl_info();

    if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

    puts(" Nand Info :\n");
    printf("Page size:%d bytes. ",(1<<nftl_i->page_shift));
    printf("Block size:%d pages. ",(1<<nftl_i->block_shift));
   // printf("Chip size:%d blocks. ",(1<<nftl_i->chip_shift));
    //printf("Chip number:%d chips\n",nandinfo->chip_nums);
    printf("\nTotal VUC %d blocks.",nftl_i->lblock_nums);
    printf("\tTotal EUN %d blocks.",nftl_i->tblock_nums);
    printf("\tSector for use:%d (0x%x) sectors.\n",
           (nftl_i->lblock_nums)*(1<<nftl_i->block_shift)*(1<<(nftl_i->page_shift - 9)),
           (nftl_i->lblock_nums)*(1<<nftl_i->block_shift)*(1<<(nftl_i->page_shift - 9)));

    chains_dump(nftl_i);
    EUNtable_dump(nftl_i);
    reptable_dump(nftl_i);
    revtable_dump(nftl_i);
}


int do_aftloff(cmd_tbl_t *cmdtp,int argc,char* argv[]){

    if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

    aftl_write_back();

    puts("AFTL Flushed.\n");
    return 0;
}

/*
int do_aftl_tstw(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    nftl_info * nftl_i;
    device_t * nandbase_t;
    int dev_index;
    char oobf[64];

    nandbase_t = device_get("nand",&dev_index);
    nftl_i = get_nftl_info();

    if (argc != 3) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

   // oobf.bad[0] = 0xff;
    memset((char *) oobf,0xff,64);
    nftl_write(nandbase_t , nftl_i ,TextToLong_TRL(argv[1]),(char * )TextToLong_TRL(argv[2]),(char *) oobf);
}

int do_aftl_tstr(cmd_tbl_t *cmdtp,int argc,char* argv[]){
    nftl_info * nftl_i;
    device_t * nandbase_t;
    int dev_index;
    int page;
    char * buffer;

    nandbase_t = device_get("nand",&dev_index);
    nftl_i = get_nftl_info();

    if (argc != 3) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 

    page = TextToLong_TRL(argv[1]);
    buffer = (char * )TextToLong_TRL(argv[2]);

    printf("do_aftl_tstr: page %d, buffer 0x%08x\n",page, buffer);
    nftl_read(nandbase_t , nftl_i ,page,buffer);
}
*/

BOOT_CMD(aftlws,do_aftlws,
         "#aftlws sector_num mem_addr len",
         "copy from mem to aftl");

BOOT_CMD(aftlrs,do_aftlrs,
         "#aftlrs sector_num [mem_addr]",
         "read one sector (512 bytes) from aftl");

BOOT_CMD(aftlinfo,do_aftlinfo,
         "#aftlinfo",
         "get aftl info");

BOOT_CMD(aftlformat,do_aftlformat,
         "#aftlformat",
         "format aftl chip");

BOOT_CMD(aftloff,do_aftloff,
         "#aftloff",
         "off aftl use");
/*
BOOT_CMD(aftl_tstw,do_aftl_tstw,
         "#aftl_tstw page buf",
         "aftl_tstw page buf");

BOOT_CMD(aftl_tstr,do_aftl_tstr,
         "#aftl_tstr page buf",
         "aftl_tstr page buf");
*/


int  nftl_ioctl(struct device * dev,unsigned int cmd,unsigned long arg){

    nftl_info * nftl_i = (nftl_info *) dev->priv_data;  
    device_t * nandbase_t;
    int dev_index;

    nandbase_t = device_get("nand",&dev_index);

    switch (cmd) {    
    case NFTL_IOCTL_FORMAT: 
        aftl_format(nftl_i);
        break;
    case NFTL_IOCTL_OFF: 
        return writeback_lastbuf(nandbase_t,nftl_i);
        break;

    case NFTL_IOCTL_GET_SECTOR_SHIFT: /* return sector size shift */
        return 9;
        break;
    case NFTL_IOCTL_GET_SECTOR_NUM: /* return number of sectors for use */
        return (nftl_i->lblock_nums)*(1<<nftl_i->block_shift)*(1<<(nftl_i->page_shift - 9));
        break;
    
    }
    return 0;
}





