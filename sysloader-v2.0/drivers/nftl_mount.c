#include <common.h>
#include <drivers/nand_search.h>
#include <drivers/flash.h>
#include <drivers/nftl_mount.h>
#include <drivers/nftl_core.h>

nftl_info ALIGN32 alp_nftlinfo;

static struct timer_list aftl_timer;

int write_lock = 0;
int read_lock = 0;

unsigned int get_chip_shift(unsigned int val){
    int i;

    if (val == 0) {return -1; }

    for (i = 0; ; i++) {
        if ( (val>>i) == 1 ) return i;
    }
}

void aftl_format(nftl_info * nftl_i){
    int total_block;
    nftl_mediaheader mediaheader;
    nftl_oob oob;
    int reserved_block,spare_block;
    char * databuf;
    device_t * nandbase_t;
    int dev_index;
    int i,erase_block;

    // === Erase all Good Blocks in this Partition ===
    /*for(block = 0;block < total_block;block++){
        if(NAND_Erase(nftl_i->nbase_info,block)==1){ // erase failed
            printf("block %d erase failed\n",block);
            mark_badblock(nftl_i->nbase_info,block);
        }
    }*/
    nandbase_t = device_get("nand",&dev_index);

    databuf = nftl_i->move_buf;
    // ===== Prepare media header ======= //
    total_block = (nftl_i->chip_size <<( 20 - (nftl_i->block_shift+nftl_i->page_shift))) ;

    reserved_block = (CONFIG_AFTL_RESERVED_SIZE_MB<<(20 - (nftl_i->block_shift+nftl_i->page_shift))) + 1;
    spare_block = (total_block>>7) * CONFIG_AFTL_SPARE_PRECENTAGE;
    strcpy(mediaheader.DataOrgID,"ANAND");
    mediaheader.first_nftlblock = (u16)reserved_block;
    mediaheader.nftlblock_nums = (u16)(total_block - reserved_block);
    mediaheader.appblock_nums = (u16)(total_block - reserved_block - spare_block);
    MediaHeader_dump(&mediaheader);

    // === erase all blocks ===//
    erase_block = reserved_block - 1;
    for (i = 0; i < total_block;i++) {
        nandbase_t->ioctl(nandbase_t,NAND_IOCTL_ERASE_SKIPBAD,(erase_block<<nftl_i->block_shift)); 
        erase_block++;
    }


    // === write to boot record block, block 0 ===
    // memset((char *)nftl_i->write_buf,0,2048);
    // memcpy(nftl_i->write_buf,(char *)&mediaheader,sizeof(nftl_mediaheader));
    // //MemDisp_TRL(&mediaheader,32,4);
    // MemDisp_TRL(nftl_i->write_buf,32,4);
    
    memset(databuf,0xff,2048);
    memset(&oob, 0xff, sizeof(nftl_oob));
    oob.Status = oob.Status1 = PAGE_USED;
    oob.revblocknum = oob.revblocknum1 = BLOCK_RESERVED;
    oob.logblocknum = oob.logblocknum1 = BLOCK_RESERVED;
    memcpy(databuf,&mediaheader,sizeof(nftl_mediaheader));
    //MediaHeader_dump(&mediaheader);
    
    nftl_write(nandbase_t , nftl_i ,-(1<<nftl_i->block_shift) ,(char * )databuf,(char *) &oob);
}

int find_media_header(nftl_info * nftl_i,device_t * nandbase_t){
    int i;
    char * databuf,* databuf_empty;
    int aftl_formated;

    databuf = nftl_i->move_buf;
    databuf_empty = nftl_i->write_buf;
    aftl_formated = 0;

__find_media_header_again:

    nftl_read(nandbase_t , nftl_i ,-(1<<nftl_i->block_shift),databuf);
    MemDisp_TRL(databuf,12,4);
    if(memcmp(databuf,"ANAND",6)){
        /* no valid header */
        memset (databuf_empty, 0xff , (1<<nftl_i->page_shift));
        if (memcmp(databuf,databuf_empty,(1<<nftl_i->page_shift))) {
            /* no valid header */
            return -1;
        }
        else{
            /* empty header, we format it */
            if (aftl_formated == 1) {
                printf("AFTL Can not Format AFTL\n");
                return -1;
            }
            aftl_format(nftl_i);
            aftl_formated = 1;
            goto __find_media_header_again;
        }
    }
//while(1);
    memcpy(&(nftl_i->nftl_header), databuf, sizeof(nftl_mediaheader));
    MediaHeader_dump(&(nftl_i->nftl_header));

    nftl_i->page_per_block = 1<<(nftl_i->block_shift);
   // nftl_i->boot_block_nums = nftl_i->nftl_header.first_nftlblock;
    nftl_i->tblock_nums = nftl_i->nftl_header.nftlblock_nums;
    nftl_i->lblock_nums = nftl_i->nftl_header.appblock_nums ; 

    /* for  block interface */
    blk_dev_aftl.lba = (nftl_i->lblock_nums << (nftl_i->block_shift + nftl_i->page_shift - 9));

    printf("nftl nftl_i:\n");
    printf("PagePerBlock:%d ",nftl_i->page_per_block);
    printf("BootBlockNums:%d ",nftl_i->boot_block_nums);
   // printf("TotalPhyBlockNums:%d ",nftl_i->pblock_nums);
    printf("NftlBlockNums:%d ",nftl_i->tblock_nums);
    printf("LogBlockNums:%d",nftl_i->lblock_nums);
    as_putc('\n');
    
    nftl_i->read_buf_status = 0;
    nftl_i->write_buf_status = 0;
    nftl_i->last_read_page = 0xff;
    nftl_i->last_read_VUC = 0xffff;
    nftl_i->last_write_page = 0xff;
    nftl_i->last_write_VUC = 0xffff;
    nftl_i->last_write_EUN = 0xffff;
    nftl_i->last_pre_block = 0xffff;

   // for (i = 0; i < nftl_i->boot_block_nums; i++){
   // 		nftl_i->reptable[i] = BLOCK_RESERVED;
   // 		nftl_i->revtable[i] = BLOCK_RESERVED;
   // }
	// === mark all remaining blocks as potentially containing data 
    nftl_i->reptable = (u16 *)c_malloc(2*(nftl_i->tblock_nums));
    if(!nftl_i->reptable){
        asfree(nftl_i->reptable);
        printf("allocation of reptable failed\n");
        return -1;
    }

    nftl_i->revtable = (u16 *)c_malloc(2*(nftl_i->tblock_nums));
    if(!nftl_i->revtable){
        asfree(nftl_i->revtable);
        printf("allocation of reptable failed\n");
        return -1;
    }

    nftl_i->EUNtable = (u16 *)c_malloc(2*(nftl_i->lblock_nums));
    if(!nftl_i->EUNtable){
        asfree(nftl_i->EUNtable);
        printf("allocation of EUNtable failed\n");
        return -1;
    }

    nftl_i->ffptable = (char *)c_malloc(2*(nftl_i->tblock_nums));
    if(!nftl_i->ffptable){
        asfree(nftl_i->ffptable);
        printf("allocation of ffptable failed\n");
        return -1;
    }
    //nftl_i->reptable = reparray;
    //nftl_i->revtable = revarray;
    //memset(nftl_i->reptable,0xff,(nftl_i->tblock_nums)*2);
    //nftl_i->reptable = c_malloc(2 * nftl_i->tblock_nums);
    //if(!nftl_i->reptable){
    //    asfree(nftl_i->reptable);
    //    printf("Allocation of RepTable failed\n");
    //    return -1;
    //}
    //nftl_i->revtable = c_malloc(2 * nftl_i->tblock_nums);
	for (i = 0; i < nftl_i->tblock_nums; i++) {
		nftl_i->reptable[i] = BLOCK_NOMARK;
		nftl_i->revtable[i] = BLOCK_NOMARK;
	}

    //=== check bad block === 
    printf("NFTL2K: Scan for Bad Blocks: ");
    for (i = 0; i < nftl_i->tblock_nums; i++) {
   	    if (is_badblock(nandbase_t, nftl_i, (i + nftl_i->boot_block_nums))){
   		   nftl_i->reptable[i] = BLOCK_RESERVED;
   		   nftl_i->revtable[i] = BLOCK_RESERVED;
           printf("(%d)\t",i);
        }
    }
    printf("\n");
    //revtable_dump(nftl_i);
    //reptable_dump(nftl_i);
    //puts("rep & revtable init\n");

    //u16 EUNarray[nftl_i->lblock_nums];
    //nftl_i->EUNtable = EUNarray;
    for (i = 0; i < nftl_i->lblock_nums; i++) {
    	nftl_i->EUNtable[i] = BLOCK_NOMARK;
    }
    //EUNtable_dump(nftl_i);
    //puts("EUNtable init\n");

    //char ffparray[nftl_i->tblock_nums];
    //nftl_i->ffptable = ffparray;
    for(i = 0;i < nftl_i->tblock_nums; i++){
        nftl_i->ffptable[i] = 0x00;
    }
    //ffptable_dump(nftl_i);
    //puts("ffptable init\n");

    return 0;
    
}

int get_chain_length(nftl_info *nftl_i, unsigned int first_block)
{
	unsigned int length = 0, block = first_block;

	for (;;) {
		length++;
		// ===check to avoid infinite loops === 
		if (length >= nftl_i->tblock_nums) {
			printf("nftl2k: length too long %d !\n", length);
			break;
		}

		block = nftl_i->reptable[block];
		if (!(block == BLOCK_TAIL || block == BLOCK_NOMARK || block < nftl_i->tblock_nums))
			printf("Incorrect ReplUnitTable[] : %d\n", block);
		if (block == BLOCK_TAIL || block == BLOCK_NOMARK || block >= nftl_i->tblock_nums)
			break;
	}
	return length;
}

int format_block(device_t * nandbase_t, nftl_info *nftl_i, int block)
{
    int ret = 0;
    int phy_block = block + nftl_i->boot_block_nums;
    //=== check bad block === 
	if(is_badblock(nandbase_t,nftl_i,phy_block)) {
		printf("nftl2k Warning: try to format a bad block %d\n", block);
        return 0;
    }

    ret = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_ERASE_FORCE,(phy_block << nftl_i->block_shift));

	if(ret){
        printf("Error while formatting block %d. We will mark it as bad!\n", block);
		goto fail;
    }
	return 0;
fail:
	// ===could not format, mark it as bad block ===
	mark_badblock(nandbase_t,nftl_i,phy_block);
	return -1;
}

void format_chain(device_t * nandbase_t, nftl_info *nftl_i, unsigned int first_block)
{
	unsigned int block = first_block, block1;

	printf("Formatting chain at block %d\n", first_block);

	for (;;) {
		block1 = nftl_i->reptable[block];

		printf("(%d) ", block);
		if (format_block(nandbase_t,nftl_i, block) < 0) {
			//=== cannot format !!!! Mark it as Bad Unit 
			nftl_i->reptable[block] = BLOCK_RESERVED;
			nftl_i->revtable[block] = BLOCK_RESERVED;
		} else {
			nftl_i->reptable[block] = BLOCK_FREE;
			nftl_i->revtable[block] = BLOCK_FREE;
			nftl_i->freeblock_nums++;
            nftl_i->ffptable[block] = 0;
		}

		// === goto next block on the chain === 
		block = block1;

		if (block == BLOCK_TAIL || block == BLOCK_NOMARK || block >= nftl_i->tblock_nums)
			break;
	}
}
char NCached aftl_oob_buf[128];
void nftl_read_oob(device_t * nandbase_t,nftl_info * nftl_i,int page,char * buffer)
{
    struct nand_read_ops read_ops;

    read_ops.page = page + (nftl_i->boot_block_nums) * (nftl_i->page_per_block);
    read_ops.data = NULL;
    read_ops.oob = aftl_oob_buf;
    nandbase_t->ioctl(nandbase_t,NAND_IOCTL_READ_OOB,(unsigned long)&read_ops);

    //printf("phy_page:%d",phy_page);
    //AS3310_Nand_Read_2k_Page(nftl_i->nbase_info,phy_page,NULL,(char *)oob,NAND_READ_RAW_OOB);
    //MemDisp_TRL(oob,64,4);
    memcpy(buffer,&aftl_oob_buf[2],sizeof(nftl_oob));
    //MemDisp_TRL(buffer,16,4);
}

int nftl_read(device_t * nandbase_t,nftl_info * nftl_i,int page,char * buffer)
{
    struct nand_read_ops read_ops;
    //char oob[64];
    int ret;

    read_ops.page = page + (nftl_i->boot_block_nums) * (nftl_i->page_per_block);
    //read_ops.page = page + (0x04000000>>11);
    read_ops.data = buffer;
    read_ops.oob = aftl_oob_buf;

//    printf("nftl_read: page %d, buffer 0x%08x, boot_block_nums 0x%08x, phy page %d\n",
//           page, buffer,nftl_i->boot_block_nums,read_ops.page);

    ret = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_READ_PAGE_ECCOS,(unsigned long)&read_ops);

    return ret;
}


/*
 * Write data and oob to flash
 */
int nftl_write(device_t * nandbase_t,nftl_info * nftl_i,int page,char * buffer, char * oob)
{
    struct nand_write_ops write_ops;
	int res;

    /* handle ecc layout */
    aftl_oob_buf[0] = 0xff;
    memcpy(aftl_oob_buf + 2, oob , 24);

    /* handle reserve page offset */
    write_ops.page = page + (nftl_i->boot_block_nums) * (nftl_i->page_per_block);
    //write_ops.page = page + (0x04000000>>11);
    write_ops.data = buffer;
    write_ops.oob = aftl_oob_buf;

//    printf("nftl_write: page %d, buffer 0x%08x, boot_block_nums 0x%08x, phy page %d\n",
//           page, buffer,nftl_i->boot_block_nums,write_ops.page);

    res = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_WRITE_PAGE_ECCOS,(unsigned long)&write_ops);

//    if ((offs & ((loff_t)mtd->erasesize - 1)) == 0) {
//        /* first page of block, so we need do ecc for uci */ 
//        /* bci status 2 byte + size of uci 16 byte = 18 byte , generate 3 byte of ecc code */
//        ftl_ECCCalculate((unsigned char *)&oob_struct->b.Status,
//                    sizeof(struct nftl2k_uci)+2,
//                    &oob_struct->b.ecc);
//    }
//
//	res = mtd->write_oob(mtd, offs & ~(mtd->writesize - 1), &ops);
//	*retlen = ops.retlen;

	return res;
}

u16 fold_chain(device_t * nandbase_t,nftl_info * nftl_i,u16 logblock){
    u16 thisEUN,tailEUN,targetEUN,tempEUN;
    nftl_oob oob;
    int chain_len;

    // === write back last package if needed 
    if (nftl_i->write_buf_status > 0) {
        writeback_lastbuf(nandbase_t,nftl_i);
    }

	thisEUN = nftl_i->EUNtable[logblock];
    tailEUN = thisEUN;

	if (thisEUN >= nftl_i->tblock_nums) {
		printf("Trying to fold non-existent :Virtual Unit Chain %d!\n", logblock);
		return 0xffff;
	}

    chain_len = 0;
    while (thisEUN < nftl_i->tblock_nums){
        chain_len++;
        tailEUN = thisEUN;
        thisEUN = nftl_i->reptable[thisEUN];
    }

	thisEUN = nftl_i->EUNtable[logblock]; // restore thisEUN
    
	if (chain_len < 2 ) {
		printf("Trying to fold a single EUN chain.\n");
		return thisEUN;
	}

    /* ==== check for inplace folding if possible ======= */
    if (nftl_i->ffptable[tailEUN] < nftl_i->page_per_block) {
        /* ===== in place folding ========== */
        targetEUN = tailEUN;
        aftl_dbg("Folding chain %d into unit %d\n",logblock, targetEUN);
        fold_page_data(nandbase_t,nftl_i,logblock,nftl_i->revtable[tailEUN],targetEUN, nftl_i->ffptable[tailEUN], nftl_i->page_per_block);

    }
    else{
        /* ===== out place folding ========== */
        // === We need to find a targetblock to fold into. 
        targetEUN = findfreeblock(nftl_i,1);
        if (targetEUN >= nftl_i->tblock_nums) {
            printf("findfreeblock failed! returns 0xffff.\n");
            return 0xffff;
        }
        aftl_dbg("Folding chain %d into unit %d\n",logblock, targetEUN);
        fold_page_data(nandbase_t,nftl_i,logblock,tailEUN,targetEUN, 0, nftl_i->page_per_block);
    }

    aftl_dbg("format block chain :\n");
	// === Free each block in the old chain for future use
	while ((thisEUN < nftl_i->tblock_nums) && (thisEUN != targetEUN)) {
		aftl_dbg("(%d) ", thisEUN);
        tempEUN = nftl_i->reptable[thisEUN];
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
	nftl_i->reptable[targetEUN] = BLOCK_TAIL;
	nftl_i->revtable[targetEUN] = BLOCK_HEAD;
	nftl_i->EUNtable[logblock] = targetEUN;

	return targetEUN;
}

void aftl_write_back(){
    nftl_info * nftl_i;
    device_t * nandbase_t;
    int dev_index;

    nandbase_t = device_get("nand",&dev_index);
    nftl_i = get_nftl_info();

    if(nftl_i->write_buf_status > 0){
        writeback_lastbuf(nandbase_t,nftl_i);
    //    putc('^');
    }
    //aftl_dbg("~");
 //   puts("~");
}

static void aftl_time_out(unsigned long arg){
    nftl_info * nftl_i;

    nftl_i = get_nftl_info();
    if(!(read_lock + write_lock )){
        if((jiffies - nftl_i->write_jif) < 200){
            aftl_write_back();            
        }
        
    }
    aftl_timer.expires = jiffies + AFTL_TIME_OUT ;
    aftl_timer.data = aftl_timer.expires;
    add_timer(&aftl_timer);
    
}

static void as3310_aftl_init_timer(void){

    aftl_timer.function = aftl_time_out;
    aftl_timer.expires = jiffies + AFTL_TIME_OUT;
    aftl_timer.data = aftl_timer.expires;
    aftl_timer.next = aftl_timer.prev = NULL;
    aftl_timer.handle_later = 0;
    add_timer(&aftl_timer);
}


int nftl_init(nftl_info * nftl_i)
{
    //nftl_info *nftl_i;
    device_t * nandbase_t;
    int dev_index;
	unsigned int first_logical_block, logical_block, pre_block;
	unsigned int block, first_block,format_block;
	int chain_length, do_format_chain;
    nftl_oob oob;

	unsigned int writeEUN;
    int lastEUN;
    int thisVUC;
    char status;

    int silly2 ;
    int tmp_block;

	// === search for NFTL2K MediaHeader ===
    //nftl_i->nbase_info = &nand_dev_info;
    nandbase_t = device_get("nand",&dev_index);
    nftl_i->page_shift = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_GET_PAGE_SHIFT,0);
    nftl_i->block_shift = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_GET_BLOCKPAGE_SHIFT,0);
    nftl_i->chip_shift = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_GET_CHIP_SHIFT,0);
    nftl_i->chip_size = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_GET_CHIP_SIZE,0);

    nftl_i->write_jif = jiffies;

#ifndef CONFIG_AFTL_SIZE_ALL
    nftl_i->chip_size = min(CONFIG_AFTL_SIZE_MB + CONFIG_AFTL_RESERVED_SIZE_MB,nftl_i->chip_size);
#endif// CONFIG_AFTL_SIZE_ALL

    nftl_i->boot_block_nums = (CONFIG_AFTL_RESERVED_SIZE_MB<<(20 - (nftl_i->block_shift+nftl_i->page_shift))) + 1;
    nftl_i->page_per_block = (1<<nftl_i->block_shift);
    //chipsize = nandbase_t->ioctl(nandbase_t,NAND_IOCTL_GET_CHIP_SIZE,0);
    //nftl_i->chip_shift = get_chip_shift((chipsize << 20) >> (nftl_i->page_shift + nftl_i->block_shift));

    printf("nand base nftl_i:\n");
    //as_puts("DevID:0x");as_putb(nftl_i->nbase_info->dev_id);
    //printf("chipsize:%d MByte",chipsize);
    as_puts(" PageShift:0x");as_putb(nftl_i->page_shift);
    as_puts(" BlockShift:0x");as_putb(nftl_i->block_shift);
    as_puts(" boot_block_nums:0x");as_puth(nftl_i->boot_block_nums);
    //as_puts(" Chipnums:0x");as_putb(nftl_i->nbase_info->chip);
    printf("Total Blocks: %d \n",(nftl_i->chip_size << (20 - (nftl_i->block_shift+nftl_i->page_shift)))  );

    //nand_format(nftl_i);
    
    if(find_media_header(nftl_i,nandbase_t) <0 ){
        printf("Could not find valid boot record\n");
        return -1;
    }


	// === init the logical to physical table and inverse table 
   // for (i = 0; i < s->nb_blocks; i++) {
   // 	s->EUNtable[i] = BLOCK_NIL;
   //}

    printf("Start Mounting...\n");
	// === first, explore each block to restore the chains ===
    memset(&oob,0xff,sizeof(nftl_oob));
    //printf(" 1st block\n");
	first_logical_block = 0;
	for (first_block = 0; first_block < nftl_i->tblock_nums; first_block++) {
		// === if the block was not already explored, we can look at it ===
		if (nftl_i->revtable[first_block] == BLOCK_NOMARK) {
			block = first_block;
			chain_length = 0;
			do_format_chain = 0;

			for (;;) {
				//=== read the block header. If error, we format the chain 
                nftl_read_oob(nandbase_t,nftl_i,block*nftl_i->page_per_block,(char *)&oob);

                if (oob.logblocknum != oob.logblocknum1) {
                    printf("EUN %d OOB data logblocknum confused, (%d != %d)\n",block,oob.logblocknum,oob.logblocknum1);
                }
                if (oob.revblocknum != oob.revblocknum1) {
                    printf("EUN %d OOB data revblocknum confused, (%d != %d)\n",block,oob.revblocknum,oob.revblocknum1);
                }
				logical_block = oob.logblocknum;
				pre_block = oob.revblocknum;

				// ===invalid/free block test 
				if (logical_block >= nftl_i->tblock_nums) {
					if (chain_length == 0) {// ===if not currently in a chain, we can handle it safely 
						//=== free block: mark it 
						nftl_i->reptable[block] = BLOCK_FREE;
						nftl_i->revtable[block] = BLOCK_FREE;
                        nftl_i->LastFreeEUN = block;
                        // printf("(%d) ",block);
						// === directly examine the next block. 
						goto examine_reptable;
					} else {
						// === the block was in a chain : this is bad. We must format all the chain 
						printf("Block %d: free but referenced in chain %d\n",
						       block, first_block);
                        format_block = nftl_i->reptable[block];
						nftl_i->reptable[block] = BLOCK_TAIL;
						nftl_i->revtable[block] = BLOCK_TAIL;
						do_format_chain = 1;
						break;
					}
				}

                aftl_dbg("VUN(%d):(%d)<-(%d)\n",logical_block,pre_block,block);

				//=== we accept only first blocks here 
				if (chain_length == 0) {
			   // === this block is not the first block in chain :
			   // === ignore it, it will be included in a chain later, or marked as not explored 
					first_logical_block = logical_block;
				} else {
					if (logical_block != first_logical_block) {
						printf("Block %d: incorrect logical block: %d expected: %d\n",
						       block, logical_block, first_logical_block);
						// === the chain is incorrect : we must format it,
						// === but we need to read it completly 
                        format_block = nftl_i->reptable[block]; 
                        nftl_i->reptable[block] = BLOCK_NOMARK;
						do_format_chain = 1;
						break;
					}
				}

				chain_length++;

				if (pre_block == BLOCK_HEAD) { // ok, this block is the head of the chain 

                    // === check if the EUNtable has already locate a physical block 
                    if (nftl_i->EUNtable[logical_block] < nftl_i->tblock_nums){
                        //=== already has a valid block, this is bad
                        //=== maybe caused by power failure in folding
                        //=== need refold the chain
                        
                        printf("NFTL:HEAD Block %d's logical block number %d already has an EUNblock %d\n",
                               block,logical_block,nftl_i->EUNtable[logical_block]);
                        format_block = block; 
                        do_format_chain = ONE_LOGBLOCK_IN_TWO_CHAINS;
                    }
                    else{
                        // === ok, it is good, we mark the EUNtable
                        nftl_i->EUNtable[logical_block] = block;
                        nftl_i->revtable[block] = BLOCK_HEAD;
                    }
					break;
				} 
                else if (pre_block >= nftl_i->tblock_nums) {
					printf("Block %d: referencing invalid block %d\n",block, pre_block);
                    format_block = block; 
					do_format_chain = 1;
					//nftl_i->revtable[block] = BLOCK_TAIL;
					//nftl_i->reptable[block] = BLOCK_TAIL;
					break;
				} 
                else {

                    // read the last page oob of the block to check if this block is a inplace folding target block
                    nftl_read_oob(nandbase_t,nftl_i,((block+1)*nftl_i->page_per_block) - 1,(char *)&oob);
                    if (oob.revblocknum ==  BLOCK_HEAD){
                        //finish this chain
                        if (nftl_i->EUNtable[logical_block] < nftl_i->tblock_nums) {
                            printf("Late HEAD Block %d's in VUC %d already has an EUNblock %d\n"
                                   ,block, logical_block, nftl_i->EUNtable[logical_block] );
                            do_format_chain = 1;
                            nftl_i->revtable[block] = BLOCK_TAIL;
                            break;
                        }
                        printf("EUN(%d) is a Late HEAD Block VUC(%d)\n", block,logical_block);
                        nftl_i->EUNtable[logical_block] = block;
                        nftl_i->revtable[block] = BLOCK_HEAD;
                        break;                                
                    }

                    if (nftl_i->revtable[pre_block] != BLOCK_NOMARK) {
					    // === the previous block is already in a chain
                        // === so we must check if it is a valid one                        
					    if ((nftl_i->revtable[pre_block] != BLOCK_HEAD) &&
                            (nftl_i->revtable[pre_block] > nftl_i->tblock_nums)){
					    	printf("Block %d: previous block %d is a invalid one, which referring %d (0x%04x)\n",
					    	       block, pre_block,nftl_i->revtable[pre_block],nftl_i->revtable[pre_block]);
                            format_block = block; 
					    	do_format_chain = 1;
					    	//nftl_i->reptable[block] = BLOCK_TAIL;
                            //nftl_i->revtable[block] = BLOCK_TAIL;
					    } else {
                            //=== check if the ReplUnitTable has already locate a physical block 
                            if (nftl_i->reptable[pre_block] < nftl_i->tblock_nums){
                                //=== already has a valid block, this is bad
                                //=== maybe caused by power failure in folding
                                //=== need refold the chain
                                printf("NFTL:Block %d's previous block %d already has a replace-block %d\n",
                                       block,pre_block,nftl_i->reptable[pre_block]);
                                format_block = block; 
                                do_format_chain = ONE_BLOCK_IN_TWO_CHAINS;
                            }
                            else{
                            //=== ok, it is good, we found another part of a chain
                                nftl_i->revtable[block] = pre_block;
                                nftl_i->reptable[pre_block] = block;
                            }
					    }
					    break;
				    } else {
                        /* we may found another part of a chain */
                        /* check if the chain can reach head */
                        silly2 = 1000;
                        tmp_block = pre_block;
                        while ((tmp_block < nftl_i->tblock_nums)&&(silly2 > 0)) {
                            printf("->%d",tmp_block);
                            tmp_block = nftl_i->revtable[tmp_block] ;
                            silly2--;
                        }

                        if (silly2 == 0) {
                            printf("NFTL:Infinite Loop for EUN %d in VUC %d\n",
                                   block,logical_block);
                            do_format_chain = 1;
                            break;
                        }
                        /* ok, it is good, we found another part of a chain */
				    	nftl_i->revtable[block] = pre_block;
				    	nftl_i->reptable[pre_block] = block;
				    	block = pre_block;
				    }

                }
			}

			// === the chain was completely explored. 
            // === Now we can handle those faulty chains 

            if (do_format_chain == ONE_BLOCK_IN_TWO_CHAINS) {
                //=== two chain pointed to one block, format the shorter one 
				unsigned int first_block0,first_block1, chain_to_format, chain_length0,chain_length1;
                first_block0 = block;
                first_block1 = nftl_i->reptable[pre_block];

                //=== XXX: what to do if same length ? 
                chain_length0 = get_chain_length(nftl_i, first_block0);
                chain_length1 = get_chain_length(nftl_i, first_block1);

                if (chain_length0 > chain_length1) {
                    //=== format the previous one, change link to this one 
                    chain_to_format = first_block1;
                    nftl_i->reptable[pre_block] = block;
                    nftl_i->revtable[block] = pre_block;
                }
                else{
                    //=== format the this one 
                    chain_to_format = first_block0;
                }
                format_chain(nandbase_t,nftl_i, chain_to_format);

            }
            else if (do_format_chain == ONE_LOGBLOCK_IN_TWO_CHAINS) {
                //=== two chain pointed to one logical entry, format the shorter one 
				unsigned int first_block0,first_block1, chain_to_format, chain_length0,chain_length1;
                first_block0 = block;
                first_block1 = nftl_i->EUNtable[logical_block];

                chain_length0 = get_chain_length(nftl_i, first_block0);
                chain_length1 = get_chain_length(nftl_i, first_block1);

                if (chain_length0 > chain_length1) {
                    //=== format the previous one, change link to this one 
                    chain_to_format = first_block1;
                    nftl_i->EUNtable[logical_block] = block;
                    nftl_i->revtable[block] = BLOCK_HEAD;
                }
                else{ // format this one 
                    chain_to_format = first_block0;
                }
                format_chain(nandbase_t,nftl_i, chain_to_format);
            }
			else if (do_format_chain) {
				//=== invalid chain : format it.
                //=== because it is a reverse search, 
                //=== so format chain start from the current block 
				format_chain(nandbase_t,nftl_i, format_block);
			} 
		}
	examine_reptable:;
	}

   // printf("init reptable[]\n");
	//=== init reptable[] of those tail blocks === 
    nftl_i->freeblock_nums = 0 ;
	for (block = 0; block < nftl_i->tblock_nums; block++) {
		if (nftl_i->reptable[block] == BLOCK_NOMARK) {
            //=== it must be the Tail of one valid chain 
            nftl_i->reptable[block] = BLOCK_TAIL;
		}
		if (nftl_i->reptable[block] == BLOCK_FREE) {
            nftl_i->freeblock_nums++ ;
		}
	}
/*
   // printf("init ffptable[]\n");
    // === init ffptable[] , and search and fold all vuc that exceed limits ===
	for (thisVUC = 0; thisVUC < nftl_i->lblock_nums; thisVUC++) {
        lastEUN = BLOCK_TAIL;
        writeEUN = nftl_i->EUNtable[thisVUC];
        chain_length = 0;
        while (writeEUN < nftl_i->tblock_nums) {
           // if (!silly--) {
           //     printk(KERN_WARNING
           //            "Infinite loop in Virtual Unit Chain 0x%x\n",
           //            thisVUC);
           //     break;
           // }

            lastEUN = writeEUN;
            //=== Skip to next block in chain 
            writeEUN = nftl_i->reptable[writeEUN];

            if (writeEUN < nftl_i->tblock_nums) {
               // ===lastEUN is not the tail of a chain
               // ===so read the free page position
                nftl_read_oob(nandbase_t,nftl_i,((lastEUN+1) * nftl_i->page_per_block - 1),(char *)&oob);
                if (oob.free_page_pos != oob.free_page_pos1) {
                    printf("EUN %d free_page_pos confused, (%d != %d)\n",block,oob.free_page_pos,oob.free_page_pos1);
                }
                nftl_i->ffptable[lastEUN] = oob.free_page_pos;
            }

            chain_length++;
        }
        
        //=== init FreePageOffsTable[], use binary search argorithmn ===
        if (lastEUN < nftl_i->tblock_nums) {
            //=== a valid VUC ===
            int page_i;
            int last_used;
            int last_free;
            last_used = 0; last_free = nftl_i->page_per_block;
            page_i = nftl_i->page_per_block-1; // search from the last page
            do  {

			    nftl_read_oob(nandbase_t,nftl_i, lastEUN * nftl_i->page_per_block + page_i,(char *)&oob);
                
			    //status = oob.b.Status | oob.b.Status1;//???
                status = oob.free_page_pos;
               // if (oob.b.Status != oob.b.Status1) {
			   // 	printk("mounting: bci status confused for page %d in EUN %d:, 1:0x%02x 2:0x%02x\n",
			   // 	       page_i, lastEUN, oob.b.Status,oob.b.Status1);
               //     if (status != SECTOR_FREE) {
               //         // === if not free , treated as a used page ===
               //         status = SECTOR_USED;
               //     }
               // }

                if (status == PAGE_FREE) {  last_free = page_i;  }
                else{ // page is used 
                    last_used = page_i;  
                }

                if (last_used + 1 == last_free ) break;  // we found the first free page
                
                page_i = (last_free + last_used)/2;  // next for seach                
            } while(1);

            nftl_i->ffptable[lastEUN] = last_free;
        }


        //=== check if we need fold this chain ===
        if (chain_length > CONFIG_AFTL_FOLD_THRESHOLD){
            fold_chain (nandbase_t,nftl_i, thisVUC);
        }
    }
*/

    // === init ffptable[] , and search and fold all vuc that exceed limits ===
	for (writeEUN = 0; writeEUN < nftl_i->tblock_nums; writeEUN++) {

        if ((nftl_i->revtable[writeEUN] < nftl_i->tblock_nums)||(nftl_i->revtable[writeEUN] == BLOCK_HEAD)) {
        //=== init FreePageOffsTable[], use binary search argorithmn ===
            //=== a valid EUN ===
            int page_i;
            int last_used;
            int last_free;
            last_used = 0; last_free = nftl_i->page_per_block;
            page_i = nftl_i->page_per_block-1; // search from the last page
            do  {
			    nftl_read_oob(nandbase_t,nftl_i, writeEUN * nftl_i->page_per_block + page_i,(char *)&oob);                
                status = oob.free_page_pos;

                if (status == PAGE_FREE) {  last_free = page_i;  }
                else{ last_used = page_i;  } // page is used 

                if (last_used + 1 == last_free ) break;  // we found the first free page
                
                page_i = (last_free + last_used)/2;  // next for seach                
            } while(1);

            nftl_i->ffptable[writeEUN] = last_free;
        }
    }

    puts("4 table init completed\n");
//    reptable_dump(nftl_i);
//    revtable_dump(nftl_i);
//    EUNtable_dump(nftl_i);
//    ffptable_dump(nftl_i);
#if CONFIG_AFTL_DEBUG
    chains_dump(nftl_i);
#endif

    /* init write back timer */
    as3310_aftl_init_timer();

	return 0;
}

int nftl_probe(struct device * dev){
    nftl_info * nftl_i;

    nftl_i = get_nftl_info();
    dev->priv_data =  nftl_i;

    return nftl_init(nftl_i);
}

/*
dev_read_sector
ofs: sector number              
count: number of sectors(512 bytes) to read              
*/
int dev_read_sector(struct device * dev,d_offs ofs,void * buf,int count){
    int ret;
    ret = 0;
    while(count > 0) {
        read_sector((nftl_info *)dev->priv_data, ofs, buf);
        ofs ++; ret++;
        buf += 512;
        count--;
    }
    return ret;
}

/*
dev_write_sector
ofs: sector number              
count: number of sectors(512 bytes) to write              
*/
int dev_write_sector(struct device * dev,d_offs ofs,const void * buf,int count){
    int ret;
    ret = 0;
    while(count > 0) {
        write_sector((nftl_info *)dev->priv_data, ofs, buf);
        ofs ++; ret++;
        buf += 512;
        count--;
    }
    return ret;
}

struct device dev_aftl = {
    .name       = "aftl",
    .dev_id     = 0,
    .probe      = nftl_probe,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = dev_read_sector,  
    .write      = dev_write_sector, 
    .ioctl      = nftl_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = NULL,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device_lv0(dev_aftl);



/* =============== Block Device Interface =============== */

unsigned long  aftl_block_read(int dev, unsigned long start,
		lbaint_t blkcnt, unsigned long *buffer){

    nftl_info * nftl_i;
    char * byte_ptr;
    int ret,error,success;

    error = success = 0;
    nftl_i = get_nftl_info();
    byte_ptr = (char *)buffer;

    while(blkcnt > 0) {
        ret = read_sector(nftl_i, start, byte_ptr);
        error += ret;
        success++;
        start++;
        byte_ptr += 512;
        blkcnt--;
    }
    if (error) {
        return  error;
    }
    else return success;
}

struct blk_dev blk_dev_aftl = {
    .name       = "aftl",
    .block_read = aftl_block_read,  
    .lba        = 0,
    .blksz      = 512,
};

__add_blk_device(blk_dev_aftl);

