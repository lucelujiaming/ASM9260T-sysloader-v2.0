/*
Alpha Scale AS3310X IronPalms Console
zhangbo, AlpScale Software Engineering
*/

/*****************************
// debug_uart test progam 
*****************************/
#include <common.h>

void duart_pin_sel()
{
	outl(0xc0000000,HW_PINCTRL_CTRL_CLR);
	outl(0x00500000,HW_PINCTRL_MUXSEL6_CLR);
}

void duart_uart_init()
{
	duart_pin_sel();
	BAUD_DIVINT();
	BAUD_DIVFRAC();
	SET_LCRH();
	SET_CR();
	SET_INT();
	
}


/*	send a byte using UART2 	*/

void duart_putc(uchar data)
{
	unsigned long status;	
	while (((status = inl(UART2_FR)) & 0x00000080) ==0)	;
		outl(data,UART2_DATA);	
    if (data == '\n') putc('\r');
}

	/*
	read a byte from UART2
	success: return 1
	no data: return 0
	*/
int duart_getc(uchar * data){
	unsigned long status;	
	
	if (((status = inl(UART2_FR)) & 0x00000010) ==0 ){	
		*data =	inl(UART2_DATA);
		return 1;
		}
	else return 0;
}

void duart_puts(uchar * buffer){
/*
*@Input  uchar * buffer; the data buffer which we will send out through rs232
*@Return null
*@Description send ¡®num¡¯ BYTES out through rs232
*/

while ((*buffer)!= 0)
	duart_putc(*(buffer++));
}

/*
get input String from UART interface
*/

int duart_gets(uchar * buffer)
{
	uchar temp;
	int ptr;
	long received_num;
	
	ptr= 0;
	received_num = 0;	
	while(1){
		if(duart_getc(&temp))
		{
            if (((temp == KEY_BACKSPACE_LINUX)&&(current_platform == PLAT_LINUX))
                ||((temp == KEY_BACKSPACE_WIN32)&&(current_platform == PLAT_WIN32))) {
                if (ptr > 0) {   
                    __BACKSPACE(current_platform);
                }
            }
            else {
		putc(temp);
		duart_putc(temp);
            	}

			if (temp==0x0d) {
                putc(0x0a);	
		duart_putc(0x0a);
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
	}
	return received_num;	
}



int do_duart(cmd_tbl_t *cmdtp,int argc,char * argv[])
{
    uchar dubuf[MAX_DUART_BUF];
	
    if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}   
    duart_puts("test debug uart\n");

    duart_gets(dubuf);
	

    return 0;
}

BOOT_CMD(duart,do_duart,
         " #duart", 
         "test debug uart");

