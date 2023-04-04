/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

GPIO UART Source file
 
Change log: 

------------------- Version 1.0  ----------------------
He Yong 2006-04-14
    -- Create file

*/

#include <drivers/gpio_uart.h>

#if CONFIG_GPIO_UART

#define BAND_RATE_38400 0x50 

#define calloc                  c_calloc
//#define asfree                 free
#define malloc                  c_malloc  
//#define asrealloc              realloc

/* USE_AS_PREFIX */

gpio_uart_regs * p_uart_reg;
UART_TX_CTRL * p_uart_tx;

gpio_uart_regs  Gpio_uart_Reg;
UART_TX_CTRL    tx_ctrl;

int gpio_uart_interrupt_handler(){

    if (p_uart_reg->SATA.field.RUN) {    
        if (p_uart_reg->SATA.field.TXFF){
            //putc('t');
            gpio_uart_tx();
        }
        gpio_uart_rx();
    }
}

/*
current_bit_index :     8       [ 7 6 5 4 3 2 1 0 ]
                    start bit          data            stop = 1
*/
void inline gpio_uart_tx(){

    if (p_uart_tx->current_bit_index == 9){
        /*  end of whole tx routine */
        set_uart_gpio(1);   // stop bit
        p_uart_reg->SATA.field.TXFF = 0;
        p_uart_reg->SATA.field.TXFE = 1;
    }
    else{
        /*  bit start ...*/
        set_uart_gpio( (p_uart_tx->data & ( 1 << (p_uart_tx->current_bit_index++) ) ) != 0 );
    }

}

//void gpio_uart_tx(){
//
//    if ( (p_uart_tx->time_line--) == p_uart_reg->BAND_TIME) {
//        /*  bit start ...*/
//        set_uart_gpio( (p_uart_tx->data & ( 1 << p_uart_tx->current_bit_index ) ) != 0 );
//    }
//    else {
//        if (p_uart_tx->time_line == 0) {
//            /* end of bit*/
//            if (p_uart_tx->current_bit_index == 0){
//                /*  end of whole tx routine */
//                set_uart_gpio(1);   // stop bit
//                p_uart_reg->SATA.field.TXFF = 0;
//                p_uart_reg->SATA.field.TXFE = 1;
//            }
//            else {
//                /* next bit  */
//                p_uart_tx->current_bit_index-- ;
//                p_uart_tx->time_line = p_uart_reg->BAND_TIME;
//            }
//        }
//        else {
//            /* do nothing */
//        }
//
//    }
//}
//

int inline gpio_uart_rx(){
}

int gpio_uart_init(int port,int pin,int bandrate){
 u32 val;
 u32 addr;

    //p_uart_reg = (gpio_uart_regs *) calloc(sizeof(gpio_uart_regs),1);
    p_uart_reg = &Gpio_uart_Reg;
    if (!p_uart_reg) {
        return -ENOMEM;
    }

    //p_uart_tx = (UART_TX_CTRL *) calloc(sizeof(UART_TX_CTRL),1);
    p_uart_tx = &tx_ctrl;
    if (!p_uart_tx) {
        return -ENOMEM;
    }


    p_uart_reg->SATA.field.RUN = 0;

    p_uart_reg->SATA.field.TXFF = 0;
    p_uart_reg->SATA.field.TXFE = 1;
    p_uart_reg->SATA.field.RXFF = 0;
    p_uart_reg->SATA.field.RXFE = 1;

    p_uart_reg->BAND_TIME = bandrate;

    p_uart_reg->Port = port;
    p_uart_reg->Pin = pin;

    p_uart_reg->SATA.field.present = 1;

    /*  arch specific configuration */
    
    /* ================== GPIO ================*/
    val = (3<<((pin&0xf)<<1));
    addr = HW_PINCTRL_MUXSEL0 + (port*0x100) + (pin>>4)*0x10 + 4;
    //printf("addr:%p  val:0x%x\n" ,addr, val);
    outl( val ,addr );   // GPIO3[14]
    
    val = (1<<pin);
    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (port*0x100) + 0x50 + 4;
    //printf("addr:%p  val:0x%x\n" ,addr, val);
    outl( val ,addr );   // Output data, set to 1

    val =  (1<<pin);
    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (port*0x100) + 0x70 + 4;
    //printf("addr:%p  val:0x%x\n" ,addr, val);
    outl( val ,addr );   // Output Enable

    printf("GPIO UART Inited [Port %d, Pin %d]\n",port,pin);
    timer2_init(bandrate);
    start_gpio_uart();
}


void inline set_uart_gpio(u32 val){
    u32 addr;
    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (p_uart_reg->Port*0x100) + 0x50 + 4;
    if (val == 0) {
        addr += 4; // clear
    }
    outl( (1<<p_uart_reg->Pin) ,addr );   // Output data
}


void inline start_gpio_uart(){

    p_uart_reg->SATA.field.RUN = 1;

}



void gpio_uart_putc(unsigned char data){
    while (p_uart_reg->SATA.field.TXFF != 0) {
    }
    p_uart_tx->data = (data<<1); // for start bits

    p_uart_tx->current_bit_index = 0 ;
    p_uart_tx->time_line = p_uart_reg->BAND_TIME;

    p_uart_reg->SATA.field.TXFE = 0;
    p_uart_reg->SATA.field.TXFF = 1;

    if (data == '\n') gpio_uart_putc('\r');
}


void gpio_uart_puts(uchar * buffer){
/*
*@Input  uchar * buffer; the data buffer which we will send out through rs232
*@Return null
*@Description send ¡®num¡¯ BYTES out through rs232
*/

while ((*buffer)!= 0)
	gpio_uart_putc(*(buffer++));
}



int do_gpio_putc(cmd_tbl_t *cmdtp,int argc,char * argv[]){
    char val;
    val = TextToLong_TRL(argv[1]);
    gpio_uart_putc(val);
}

int do_setband(cmd_tbl_t *cmdtp,int argc,char * argv[]){
    p_uart_reg->BAND_TIME = TextToLong_TRL(argv[1]);
}


//int do_set_gpio(cmd_tbl_t *cmdtp,int argc,char * argv[]){
//
//u32 port,pin,val,addr,set_val;
//
//    port = TextToLong_TRL(argv[1]);
//    pin = TextToLong_TRL(argv[2]);
//    val = TextToLong_TRL(argv[3]);
//
//    set_val = (1<<pin);
//    addr = HW_PINCTRL_MUXSEL0 - 0x10 + (port*0x100) + 0x50 + 4;
//    if (val == 0) {
//        addr += 4; // clear
//    }
//    printf("addr:%p  val:0x%x\n" ,addr, set_val);
//    outl( set_val ,addr );   // Output data
//    
//}

// BOOT_CMD(gpio,do_set_gpio,
//         " #gpio port pin val:0/1",
//         "Set GPIO");

 BOOT_CMD(gpio_putc,do_gpio_putc,
         " #do_gpio_putc [char]",
         "Putc");

 BOOT_CMD(band,do_setband,
         " #band [num]",
         "Set GPIO UART bandrate");


#endif // CONFIG_GPIO_UART
 
