/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Serial Source file
 
Change log: 

*/

#include <common.h>
/*--------------------- I/O Functions-----------------------*/
/*
	send a byte using UART1
	*/
void serial_putc(uchar data)
{
	//volatile unsigned long status;	
	while ((( inl(HW_UART4_STAT)) & 0x08000000) ==0)	;
		outl(data,HW_UART4_DATA);	
    if (data == '\n') putc('\r');
}




void as3310_puts(uchar * buffer){
/*
*@Input  uchar * buffer; the data buffer which we will send out through rs232
*@Return null
*@Description send ¡®num¡¯ BYTES out through rs232
*/

while ((*buffer)!= 0)
	putc(*(buffer++));
}
// 
// int getc_r(uchar * data){
// 	unsigned long status;	
// 	if (((status = inl(UART1_SATA)) & 0x01000000) ==0){	
// 		*data =	inl(UART1_DATA);
//         putc(*data);
// 		return 1;
// 		}
// 	else return 0;
// }

