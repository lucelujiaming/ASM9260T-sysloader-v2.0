#ifndef  __DUART_H__
#define __DUART_H__

/* UART2 Registers  */  
#define ALPAS3310_UART2_BASE 0x80070000

#define UART2_DATA ALPAS3310_UART2_BASE
#define UART2_RSR_ECR (ALPAS3310_UART2_BASE + 0x4)
#define UART2_FR (ALPAS3310_UART2_BASE + 0x18)
#define UART2_IrDA_ILPR (ALPAS3310_UART2_BASE + 0x20)
#define UART2_IBRD (ALPAS3310_UART2_BASE + 0x24)
#define UART2_FBRD (ALPAS3310_UART2_BASE + 0x28)
#define UART2_LCR_H (ALPAS3310_UART2_BASE + 0x2C)
#define UART2_CTRL (ALPAS3310_UART2_BASE + 0x30)
#define UART2_IFLS (ALPAS3310_UART2_BASE + 0x34)
#define UART2_IMSC (ALPAS3310_UART2_BASE + 0x38)
#define UART2_RIS (ALPAS3310_UART2_BASE + 0x3C)
#define UART2_MIS (ALPAS3310_UART2_BASE + 0x40)

#define UART2_ICR (ALPAS3310_UART2_BASE + 0x44)

#define MAX_DUART_BUF 64

#define BAUD_DIVINT() (outl(0x0027,UART2_IBRD))	//UARTDBGIBRD	
#define BAUD_DIVFRAC() (outl(0x0004,UART2_FBRD))	//UARTDBGFBRD, set baudrate to be 38400	
#define SET_LCRH() (outl(0x0070,UART2_LCR_H))	   //UARTDBGLCR, 8bit frame, fifo enable	
#define SET_CR() (outl(0x0301,UART2_CTRL))	//UARTDBGCR, enable uart	
#define SET_INT() (outl(0x07f0,UART2_IMSC))	//UARTDBIMSC

void durt_pin_sel();
void duart_uart_init();
void duart_putc(uchar data);
int duart_getc(uchar * data);
void duart_puts(uchar * buffer);
int duart_gets(uchar * buffer);
#endif


