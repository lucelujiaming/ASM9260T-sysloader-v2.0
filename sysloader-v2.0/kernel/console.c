/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Console Source file
 
Change log: 

------------------- Version 1.3.2  ----------------------
Move main() to main,c
 He Yong 2007-3-18
 
------------------- Version 1.3.1  ----------------------
Support malloc printf mmu, etc..
He Yong 2007-03-18

------------------- Version 1.3  ----------------------
Support NAND NOR UART ROM boot
He Yong 2006-12-04

------------------- Version 1.2  ----------------------
Switch to AS3310C, build in Boot Rom
He Yong 2006-11-06

------------------- Version 1.1  ----------------------
Define all registers IO prots .. etc
He Yong 2006-11-03

------------------- Version 1.0  ----------------------
He Yong 2006-05-26
Create File.

NOTE: DO NOT add code before Sram_relocate()
*/

#include <common.h>
/*--------------------- I/O Functions-----------------------*/

uchar Section_TEXTLIB buffer_command[MAX_ARG_LENGTH];
uchar Section_TEXTLIB *buff;
volatile int Section_TEXTLIB rcv_p = 0;
volatile long Section_TEXTLIB rcv_num = 0;


/*--------------------- Other Helpful Functions-----------------------*/

/*
Convert Byte data to HEX String
*/
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


/*
Convert String to long
*/
long TextToLong_TRL(char *S){
    int i,neg;
    long value;

    if (S[0]=='-') {
    	neg = -1;
  	  S=S+1;
    }
    else neg = 1;

    if ((S[0]<'0')&&(S[0]>'9')) {
        return 0;
    }

    i=0;
    value=0;
    if ((S[0]=='0')&&((S[1]=='x')||(S[1]=='X'))) {
        // Hex
        i=2;
        while (1) {
            if ((S[i]>='0')&&(S[i]<='9')) {
                  value = (value<<4) +  S[i] - '0';
                  i++;
            }
            else if ((S[i]>='a')&&(S[i]<='f')) {
                  value = (value<<4) +  S[i] - 'a' + 10;
                  i++;
            }
            else if ((S[i]>='A')&&(S[i]<='F')) {
                  value = (value<<4) +  S[i] - 'A' + 10;
                  i++;
            }
            else return value*neg;
        }
    }
    else  if ((S[0]=='0')&&((S[1]=='o')||(S[1]=='O'))) {
        // Oct
        i=2;
        while (1) {
            if ((S[i]>='0')&&(S[i]<='7')) {
                  value = (value<<3) +  S[i] - '0';
                  i++;
            }
            else return value*neg;
        }
    }
    else {
        // Dec
        i=0;
        while (1) {
            if ((S[i]>='0')&&(S[i]<='9')) {
                  value = value*10 +  S[i] - '0';
                  i++;
            }
            else return value*neg;
        }
    }
}

/*
Memory Display Function
Display memory data from address (*buf) for (total_length) BYTE
*/

#define DISP_LINE_LEN	16
int MemDisp_TRL(uchar *buf,int total_length,int size)
{
	ulong	addr;
	ulong	i, nbytes, linebytes;
	uchar	*cp;

	int rc = 0;
    addr = (ulong)buf;

	/* Print the lines.
	 *
	 * We buffer all read data, so we can make sure data is read only
	 * once, and all accesses are with the specified bus width.
	 */
	nbytes = total_length;
	do {
		char	linebuf[DISP_LINE_LEN];
		ulong	*uip = (ulong   *)linebuf;
        /* 	
        ushort	*usp = (ushort *)linebuf;
		uchar	*ucp = (uchar *)linebuf;
        */
		puts("0x");puth(addr);puts (": ");
		
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		for (i=0; i<linebytes; i+= size) {
			puth( (*uip++ = *((ulong *)addr)));
			puts ("  ");
			addr += size;
		}

		puts ("   ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				putc ('.');
			else
				putc(*cp);
			cp++;
		}
		putc('\n');
		nbytes -= linebytes;

	} while (nbytes > 0);

	return (rc);
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


/*------Command Loop: recieve command,and do cmd-----------*/
#if CONFIG_COMMAND_ONLY

/*
get input String from UART interface
*/
int argv_detect(){
    volatile uchar temp;

    if(getc((uchar *)&temp))
     {
         if (((temp == KEY_BACKSPACE_LINUX)&&(current_platform == PLAT_LINUX))
             ||((temp == KEY_BACKSPACE_WIN32)&&(current_platform == PLAT_WIN32))) {
             if (rcv_p > 0) {   
                 __BACKSPACE(current_platform);
             }
         }
         else putc(temp);

         if (temp==0x0d) {
             putc(0x0a);	
             buffer_command[rcv_p++]=0;
             return ENTER_INPUT;
         }
         else{ 
             rcv_num++;
         if (((temp == KEY_BACKSPACE_LINUX)&&(current_platform == PLAT_LINUX))
             ||((temp == KEY_BACKSPACE_WIN32)&&(current_platform == PLAT_WIN32)))
             { if(rcv_p>0) {rcv_p--;rcv_num--;}}
                 else buffer_command[rcv_p++]=temp;				
         }
    }
         return NON_ENTER_INPUT;
}


int Get_argv_TRL(uchar * buffer)
{
	volatile uchar temp;
	volatile int ptr;
	volatile long received_num;

	ptr= 0;
	received_num = 0;	
	while(1){

		if(getc((uchar *)&temp))
		{
            if (((temp == KEY_BACKSPACE_LINUX)&&(current_platform == PLAT_LINUX))
                ||((temp == KEY_BACKSPACE_WIN32)&&(current_platform == PLAT_WIN32))) {
                if (ptr > 0) {   
                    __BACKSPACE(current_platform);
                }
            }
            else putc(temp);

			if (temp==0x0d) {
                putc(0x0a);	
                buffer[ptr++]=0;
                break;
			}
			else{ 
				received_num++;
            if (((temp == KEY_BACKSPACE_LINUX)&&(current_platform == PLAT_LINUX))
                ||((temp == KEY_BACKSPACE_WIN32)&&(current_platform == PLAT_WIN32)))
                { if(ptr>0) {ptr--;received_num--;}}
					else buffer[ptr++]=temp;				
			}
		}
       #if __EMI_DEBUG__
        putc('.');
       #endif //__EMI_DEBUG__
	}
  //  puts("get a line\n");
	return received_num;	
}

int do_cmd(char * buffer){
    char *argv[MAX_AGRC];
    cmd_tbl_t * cmd_tb;
    int argc;

    argc = 1;
    argv[0] = buffer;

    // Receiving command byte from RS232
    while(*buffer !=0 ){// converting one line cmd to divided argvs (seperate by SPACE) 
        if (*buffer == ' '){*buffer = 0;	while(*(++buffer)==' '); argv[argc++]=buffer;}
            else buffer++;
    }

    for (cmd_tb = (cmd_tbl_t *)&__CMD_START ; cmd_tb != (cmd_tbl_t *)&__CMD_END ; ) {
        if (strcmp(argv[0],cmd_tb->name)==0){
            return cmd_tb->cmd(cmd_tb,argc,argv);
        }
        cmd_tb++;
    }
    return -133;
}

void Command_loop(){
    uchar buffer_org[MAX_ARG_LENGTH];
    uchar *buffer;

    while (1){
		buffer = buffer_org;
        putc('[');
        puts(CMD_LOGO);
#ifdef CONFIG_VFS
        print_pwd();
#endif// CONFIG_VFS
        putc(']');
        switch (current_platform) {
            case PLAT_WIN32:
                puts(CMD_PROMPT_WIN32);break;
            default:
                puts(CMD_PROMPT_LINUX);
        }
        
		Get_argv_TRL(buffer);

        if (do_cmd(buffer) != -133 ) continue;

        if (*buffer_org!=0) {        
            //printf("No such command :%s\nTry 'help' to list all commands\n",buffer_org);
            puts("No such command :");puts(buffer_org);puts("\nTry 'help' to list all commands\n");
        }
    }
}
#endif// CONFIG_COMMAND_ONLY

#ifdef CONFIG_COMMAND_AND_TASK

int command_task(void){
    int ret;
    ret = argv_detect();
    if (ret == ENTER_INPUT) {
        if ((do_cmd(buffer_command) == -133)&&(*buffer_command!=0)) 
            printf("No such command :%s\nTry 'help' to list all commands\n",buffer_command);
        else 
            printf("");
        rcv_p =0;
        rcv_num = 0;
    }
    return 0;
}



t_ops t_command = {
    .tname      = "as3310_command",
    .tid        = 1001,
    .task_init  = NULL,
    .task_in    = NULL,
    .task_out   = NULL,
    .tfunc      = command_task,
};

__define_init_task(t_command);

#endif //CONFIG_COMMAND_AND_TASK
