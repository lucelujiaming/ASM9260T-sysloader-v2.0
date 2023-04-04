/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

GPIO UART Header file
 
Change log: 

------------------- Version 1.0  ----------------------
He Yong 2006-04-14
    -- Create file

*/

#ifndef __GPIO_UART_H__
#define __GPIO_UART_H__

#include <common.h>

#define PACKED __attribute__((packed))
//#define malloc asmalloc

#define GPIO_UART_BANDRATE_38400 0x4E /*  24M / 4 / 38400 /2 = 0x4E  */

/* public interface */

int gpio_uart_init(int port,int pin,int bandrate);

void inline start_gpio_uart();
int stop_gpio_uart();

int gpio_uart_getc(unsigned char * data);
void gpio_uart_putc(unsigned char data);

/* Private routins & datas */

typedef union _uart_stat{
    struct _uart_stat_{
        u32 present       : 1; 
        u32 hispeed       : 1;   
        u32 busy          : 1;  
        u32 CTS           : 1; 
        u32 TXFE          : 1;   
        u32 RXFF          : 1;   
        u32 TXFF          : 1;   
        u32 RXFE          : 1;   
        u32 RXINVALID     : 4;   
        u32 NONE          : 3;   
        u32 RUN           : 1;   
        u32 DMA           : 16;   
    } PACKED field ;
    ulong value;
}UART_SATA;


//typedef union _uart_ctrl{
//    struct _uart_ctrl_{
//        ulong BAND_TIME      : 16; 
//        ulong NONE           : 16;   
//    } PACKED field ;
//    ulong value;
//}UART_CTRL;

typedef struct _gpio_uart_regs_ {
    u32           DATA;       // 0x00
    UART_SATA       SATA;       // 0x10
    u32           BAND_TIME;       // 0x20
    u32           DEBUG;      // 0x30
    u8           Port;       // 0x30
    u8           Pin;        // 0x30
    unsigned char   FIFO[16];    // 0x40
}__attribute__((packed)) gpio_uart_regs;

extern gpio_uart_regs * p_uart_reg;


typedef struct  _uart_tx_control{
    u32 time_line;
    int current_bit_index;
    unsigned char data;
}UART_TX_CTRL;

extern UART_TX_CTRL * p_uart_tx;

void inline set_uart_gpio(u32 val);

int gpio_uart_interrupt_handler();

int inline gpio_uart_rx();

/*
current_bit_index :     8       [ 7 6 5 4 3 2 1 0 ]
                    start bit          data            stop = 1
*/
void inline gpio_uart_tx();

#endif // __GPIO_UART_H__



