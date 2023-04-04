/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Command Source file
 
Change log: 
------------------- Version 1.0  ----------------------
 He Yong 2006-11-06
 Create File
 
 */

#include <common.h>
#include <drivers/nand_search.h>
#include <drivers/flash.h>

// int do_about(cmd_tbl_t *cmdtp,int argc, char *argv[]){
// puts(cmdtp->usage);
// return 0;
// }
// 
// BOOT_CMD(about,do_about,
//          ABOUT_MSG,
//          "about this program");


#if CONFIG_CMD_HELP

int do_help(cmd_tbl_t *cmdtp,int argc,char* argv[]){
cmd_tbl_t * cmd;
if (argc == 1) {/* list all cmd*/
    for (cmd = (cmd_tbl_t *)&__CMD_START ; cmd < (cmd_tbl_t *)&__CMD_END ; ) {
      puts(cmd->name);puts("\t: ");
      puts(cmd->help);putc('\n');
      cmd++;
    }
}
else {
/* display usage of one cmd */
    for (cmd = (cmd_tbl_t *)&__CMD_START ; cmd < (cmd_tbl_t *)&__CMD_END ; ) {
      if (strcmp(argv[1],cmd->name)==0){
          puts("usage : ");puts(cmd->usage);putc('\n');
          return 0;
      }
      cmd++;
    }
puts("No such command :");
puts(argv[1]);
puts("\nTry 'help' to list all commands\n");
}

return 0;
}

BOOT_CMD(help,do_help,
         " #help",
         "this program");

#endif// CONFIG_CMD_HELP

#if CONFIG_CMD_MEMFILL
int do_memfill(cmd_tbl_t *cmdtp,int argc,char* argv[])
{ 
	AS3310_REG32 * addr;
	ulong data,end_addr,len;
    volatile ushort * sptr;
    char aligin;

	if (argc != 5) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
	aligin = argv[1][0];	
	addr = (AS3310_REG32 *)(TextToLong_TRL(argv[2]));	
	len = TextToLong_TRL(argv[3]);	
	end_addr = (ulong)addr + len;
	data = TextToLong_TRL(argv[4]);

    if (aligin == 'l') { // 4 byte - aligin
        while ((ulong)addr<end_addr){		
        	*(addr++) = data;
        }
    }
    else if(aligin == 'w'){// 2 byte - aligin
        sptr = (ushort *)addr;
        while ((ulong)sptr<end_addr){		
        	*(sptr++) = data;
        }
    }
    else{// 1 byte - aligin
        memset((void *)addr,data,len);
    }
    
	return 0;
}

BOOT_CMD(memfill,do_memfill,
         " #memfill ADDR Length data",
         "Fill Memory/Regs with one input data");

#endif// CONFIG_CMD_MEMFILL


#if CONFIG_CMD_MEMCMP
/*
memory copy functions
*/
int do_mcmp(cmd_tbl_t *cmdtp,int argc,char* argv[]){
ulong * src_addr,* des_addr;
ulong length;
int i;

	if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

	src_addr = (ulong *)(TextToLong_TRL(argv[1]));	
	des_addr = (ulong *)(TextToLong_TRL(argv[2])); 
	length = TextToLong_TRL(argv[3]);

    if (memcmp(des_addr,src_addr,length)!=0) {
        puts("Mem Differ\n");
    }
    else {
        puts("Mem The Same\n");
    }

 return 0;
} 
BOOT_CMD(mcmp,do_mcmp,
         " #mcmp src_addr des_addr length",
         "Compare memory");

#endif //CONFIG_CMD_MEMCMP


#if CONFIG_CMD_CP
/*
memory copy functions use asm calls
*/
int do_cp(cmd_tbl_t *cmdtp,int argc,char* argv[]){
ulong * src_addr,* des_addr;
ulong length;
int i;

	if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

	src_addr = (ulong *)(TextToLong_TRL(argv[1]));	
	des_addr = (ulong *)(TextToLong_TRL(argv[2])); 
	length = TextToLong_TRL(argv[3]);
	memcpy(des_addr,src_addr,length);
 return 0;
} 
BOOT_CMD(cp,do_cp,
         " #cp src_addr des_addr length",
         "Memory Copy");

#endif

#if CONFIG_CMD_MEMMOV
/*
memory move functions use asm calls
*/
int do_mmov(cmd_tbl_t *cmdtp,int argc,char* argv[]){
ulong * src_addr,* des_addr;
ulong length;
int i;

	if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  

	src_addr = (ulong *)(TextToLong_TRL(argv[1]));	
	des_addr = (ulong *)(TextToLong_TRL(argv[2])); 
	length = TextToLong_TRL(argv[3]);
	memmove(des_addr,src_addr,length);
 return 0;
} 
BOOT_CMD(mmov,do_mmov,
         " #mmov src_addr des_addr length",
         "Memory Move (for overlaped memcpy)");
#endif


#if CONFIG_CMD_MD
int do_md(cmd_tbl_t *cmdtp,int argc,char* argv[]){

	ulong addr;
	int length;	
	if ((argc > 3)||(argc < 2)) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
	addr = TextToLong_TRL(argv[1]); 
    if (argc == 2)  length = 0x60;    
        else length = TextToLong_TRL(argv[2]);	
	puts("Memory Display:\n");
	MemDisp_TRL((uchar *)addr,length,4); 
    return 0;
}
BOOT_CMD(md,do_md,
         " #md ADDR [Length]",
         "Memory/Reg Display");

#endif


#if CONFIG_CMD_MW
int do_mw(cmd_tbl_t *cmdtp,int argc,char* argv[]){
	ulong addr;
	ulong value;	
	if (argc != 4){puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
	addr = TextToLong_TRL(argv[2]);
	value = TextToLong_TRL(argv[3]);   
    if (argv[1][0]=='b') {
	outb((uchar)value,addr);
    }
    else if (argv[1][0]=='w') {
	outw((ushort)value,addr);
    }
    else 
	outl(value,addr);
    return 0;
}
BOOT_CMD(mw,do_mw,
         " #mw [b w l] ADDR data",
         "Write ONE Memory/Regs 8/16/32 bit mode selectable,no outputs");

#endif


#if CONFIG_CMD_LOADU
/*
Load a binary file from UART
*/
int do_loadu(cmd_tbl_t *cmdtp,int argc,char * argv[]){
ulong addr,length,end_addr;
uchar temp[4];
volatile ulong *lptr;
volatile ulong *addrm;
int i;

    if (argc != 2) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
#if (CONFIG_IRQ_AS3310D || CONFIG_IRQ_AS3310E)
    disable_interrupts();
#endif
	addr = TextToLong_TRL(argv[1]);	
	addr = addr&0xfffffffc; /*min data with = 32bit*/

	puts("Waiting For SIZE....");

	for (i=0;i<4;i++){
		while (getc(&temp[i])==0);  	    			
	}
    lptr = (volatile ulong *)temp;
    length = *lptr;

    addrm = (volatile ulong *)addr;
    end_addr= addr + length;

	while ((ulong)addrm<end_addr){
		for (i=0;i<4;i++){
            while (getc(&temp[i])==0);                
		}
		*(addrm++) = *lptr;
	}
    puts("File Size: 0x");puth(length);putc('\n');
#if (CONFIG_IRQ_AS3310D || CONFIG_IRQ_AS3310E)
    enable_interrupts();
#endif
    return 0;
}
BOOT_CMD(loadu,do_loadu,
         " #loadu memaddr",
         "Load a binary file from UART to Memory");

#endif


#if CONFIG_CMD_RAMTEST

int do_sdramtest(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
ulong length,start_addr,end_addr,error;
ulong * addr,*lptr;
int i;
char dma_buf[1024];

if ((argc < 3)||(argc > 4)) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}  
start_addr = TextToLong_TRL(argv[1]);
length = TextToLong_TRL(argv[2]);

error = 0;

/* Byte Mode Word Mode cross-test */

addr=(ulong * )start_addr;
end_addr = start_addr + length;

for (;(ulong)addr<end_addr;){
    *addr = (ulong)addr;
    if (((ulong)addr & 0xffff) == 0x0304) {
        outb(0x04,addr);
        outb(0x03,((ulong)addr)+1);
    }
    if (argc == 4) {
        puth((ulong)addr);puts(":   ");
        puth(*addr);putc('\n');
    }
    addr++;
}  


/* Area Verify */
		
for (addr=(ulong * )start_addr ;(ulong)addr<end_addr;){
    if (argc == 4) {
        puth((ulong)addr);puts(":   ");
        puth(*addr);putc('\n');
    }
	if( (ulong)addr != *addr) error++;
	addr++;
}	


#ifdef CONFIG_DEBUG_MODE
/* Burst DMA test */
addr=(ulong * )start_addr;
end_addr = start_addr + length;
end_addr = ((end_addr>>10)<<10);

for (;(ulong)addr<end_addr ;){

    lptr = (ulong * )dma_buf;

    for (i = 0; i < (1024>>2) ; i++) {
        *(lptr++) = (((ulong)addr)>>2);
        addr++;
    }
    memory_copy((int)(addr-256), (int)dma_buf,  1024);
}  

for (addr=(ulong * )start_addr ;(ulong)addr<end_addr;){

    lptr = (ulong * )dma_buf;
    memory_copy((int)dma_buf, (int)addr, 1024);
    for (i = 0; i < (1024>>2) ; i++) {
        if( (((ulong)addr)>>2) != *lptr) error++;
        lptr++;  addr++;
    }
}	

#endif //(DEBUG_MODE)


puts("\nTest Result: Total Error: 0x");
error = (error << 2);puth(error);putc('\n');
return error;
}
BOOT_CMD(ramtest,do_sdramtest,
         " #ramtest ADDR Length [debug]",
         "test SDRAM");
#endif


#if CONFIG_CMD_RUN

int do_run(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
       ulong   addr, rc;
       int     rcode = 0;

    if (argc <2) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

       addr = TextToLong_TRL(argv[1]);

       puts("## Starting Application at :0x");puth(addr);putc('\n');

       /*
        * pass address parameter as argv[0] (aka command name),
        * and all remaining args
        */
       rc = ((ulong (*)(int, char *[]))addr) (--argc, &argv[1]);
       if (rc != 0) rcode = 1;

       puts ("\n## Application terminated, rc = 0x");puth(rc);putc('\n');
       return rcode;
}

BOOT_CMD(run,do_run,
         " #run ADDR [args]",
         "run ASM/C entry application at ADDR (returnable)");

#endif


#if CONFIG_CMD_SLECTPLAT
int do_selplat(cmd_tbl_t *cmdtp,int argc,char * argv[])
{

    char plat;

    if (argc <2) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}

    puts("Platfrom Selected ");
    plat = TextToLong_TRL(argv[1]);
    switch(plat){
    case PLAT_WIN32:
        current_platform = PLAT_WIN32;puts("Win32\n");break;
    case PLAT_LINUX:
        current_platform = PLAT_LINUX;puts("Linux\n");break;
    default:
        puts("! Wrong num ");puts(cmdtp->usage);putc('\n');
    }
return 0;
}
BOOT_CMD(selplat,do_selplat,
         " #selplat 1/2 (1-win32, 2-linux)",
         "select platform to enable fine display/operation");
#endif


#if CONFIG_CMD_SETUP
int do_setup(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
 unsigned char cen;

 if (argc <2) goto error;
 if (strcmp(argv[1],"nor")==0) {
    if (argc <3) goto error;
    outl(0x3 << ((TextToLong_TRL(argv[2]))*2)  ,HW_PINCTRL_MUXSEL6_CLR); 
    puts("Nor Flash inited\n");
 }
 else if (strcmp(argv[1],"sdram")==0) {
     if (argc >= 6) {
         outl( 3 << (TextToLong_TRL(argv[2])*2) ,HW_PINCTRL_MUXSEL6_CLR); 
         outl(((TextToLong_TRL(argv[3])&0xf)<<4) + (TextToLong_TRL(argv[4])&0xf) + 0x100,HW_EMIDRAMADDR);
         outl((TextToLong_TRL(argv[5])&0x30),HW_EMIDRAMMODE); /* lagecy  */
     }
     else  goto error;
 }

#if CONFIG_NAND
  else if (strcmp(argv[1],"nand")==0) {
      nand_info * nand_i;
      nand_i = get_nand_info();
      //    puts("NAND Setup");
      if (argc < 7 ){
          /*    Auto Search   */
          if (argc == 2) {
              /* Auto search for the first valid nand chip */
              for (cen = 0; cen < 4 ; cen++ ){
                  select_nand_cen(cen);putc('\n');
                  NAND_init(nand_i);
                  if (nand_i->row_cycles > 0) break; /*nand found*/
              }
          }
          else {
              /* search nand chip with given cen*/
              cen = TextToLong_TRL(argv[2]);
              select_nand_cen(cen);putc('\n');
              NAND_init(nand_i);
          }
      }
      else {
          /*    set with user config */
          cen = TextToLong_TRL(argv[2]);
          select_nand_cen(cen);putc('\n');
          nand_i->row_cycles = TextToLong_TRL(argv[3]);
          nand_i->col_cycles = TextToLong_TRL(argv[4]);
          nand_i->page_shift = TextToLong_TRL(argv[5]);
          nand_i->block_shift = TextToLong_TRL(argv[6]);
          nand_i->addr_cycles = nand_i->row_cycles + nand_i->col_cycles;
      }
 }
#endif //CONFIG_NAND
 else {
     puts("no such device:");
     puts(argv[1]);
     putc('\n');
 }

return 0;
error:
    puts("Invalid args.");
    puts(cmdtp->usage);
    putc('\n');
return -1;

}

BOOT_CMD(setup,do_setup,
         " #setup [device] [options]\n\
         options:\n\
         1,nand [Cen_num row col PageShift BlockPageShift]\n\
         2,nor Cen_num\n\
         3,sdram Cen row col delay",
         "Setup chosen device ready to work\n\
         Assign pins & call init_functions");   
#endif







