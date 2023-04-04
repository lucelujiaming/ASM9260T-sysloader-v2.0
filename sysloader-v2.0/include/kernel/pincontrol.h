/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

Pin Control HEADER file
 
Change log: 

------------------- Version 1.0  ----------------------
He Yong 2006-04-14
    -- Create file

*/

#ifndef _PIN_CONTROL_H_
#define _PIN_CONTROL_H_

#define GPIO_TOTAL_PORTS           4    /* total banks */
#define GPIO_PINS_OF_EACH_PORTS    32

#define PINCTRL_CTRL_OFFSET 0
#define PINCTRL_MUXSEL0     0x10
#define PINCTRL_MUXSEL1     0x20
#define PINCTRL_DRIVE       0x30
#define PINCTRL_DOUT        0x50
#define PINCTRL_DIN         0x60
#define PINCTRL_DOE         0x70
#define PINCTRL_PIN2IRQ     0x80
#define PINCTRL_IRQEN       0x90
#define PINCTRL_IRQLEVEL    0xA0
#define PINCTRL_IRQPOL      0xB0
#define PINCTRL_IRQSTAT     0xC0
#define PINCTRL_PU          0xD0
#define PINCTRL_PD          0xE0

/* GPIO TYPE */
#define PIN_FUNCTION_0      0
#define PIN_FUNCTION_1      1
#define PIN_FUNCTION_2      2
#define PIN_FUNCTION_3      3
//#define PIN_FUNCTION_GPIO   3

/* GPIO Status */
#define GPIO_FREE       0
#define GPIO_USED       3
#define GPIO_RESERVED   7

struct as3310_gpio{
    int port;
    int pin;
    int function;

};

/* 
user can not call set_pin_mux() outside pincontrol.c it's unsafe,
call request_as3310_gpio() instead !!
*/
// void set_pin_mux(int port,int pin,int mux_type);
int request_as3310_gpio(int port,int pin,unsigned int type);
void release_as3310_gpio(int port,int pin);
void as3310_gpio_init(void);
int get_pin_mux_val(int port,int pin);
void set_GPIO(int port,int pin);
void clear_GPIO(int port,int pin);
void write_GPIO(int port,int pin,int value);
int read_GPIO(int port,int pin);

int is_io_irq_pending(int port,int pin);
int pin_hex2dec(int pin_hex);

#define GPIO_IRQ_LEVEL_LOW          0
#define GPIO_IRQ_LEVEL_HIGH         1
#define GPIO_IRQ_EDGE_FALLING       0
#define GPIO_IRQ_EDGE_RISING        1

void io_irq_enable_edge(int port,int pin,int type);
void io_irq_enable_level(int port,int pin,int type);
void io_irq_disable(int port,int pin);
void io_irq_mask(int port,int pin);
void io_irq_unmask(int port,int pin);
void io_irq_clr(int port,int pin);
int get_io_irq_status(int port,int pin);



/* AS3310 Default Pin Assign */

        /* ============== UART =============== */
#define GPIO_UART_PORT       1      
#define GPIO_UART_RX_PIN        24 
#define GPIO_UART_TX_PIN        25      

        /* ============== SDRAM ============== */
#define GPIO_SDRAM_A_PORT        2      
#define GPIO_SDRAM_A_PIN_START   16 
#define GPIO_SDRAM_A_PIN_END     27 
     
#define GPIO_SDRAM_D_PORT        2      
#define GPIO_SDRAM_D_PIN_START   0 
#define GPIO_SDRAM_D_PIN_END     15 

#define GPIO_SDRAM_MISC_PORT        3      
#define GPIO_SDRAM_MISC_PIN_START   4 
#define GPIO_SDRAM_MISC_PIN_END     9 

#define GPIO_SDRAM_RASN_PORT     2      
#define GPIO_SDRAM_RASN_PIN      31 
            /* cen 2*/
#define GPIO_SDRAM_CEN_PORT     3      
#define GPIO_SDRAM_CEN_PIN      2 
        /* ============== pwr_on ============== */
#define GPIO_PWRON_PORT           3      
#define GPIO_PWRON_PIN           10

     /* ============== pwm ============== */
#define GPIO_PWM_PORT           3      
#define GPIO_PWM_BASE_PIN       10
    
#ifdef CONFIG_BOARD_AS3310_MP4_DEMO
        /* ============== audio ============== */
    #define GPIO_HP_DET_PORT        0      
    #define GPIO_HP_DET_PIN         19

    #define GPIO_SPK_PORT          3      
    #define GPIO_SPK_PIN           3
#endif

#ifdef CONFIG_BOARD_AS3310_DEV
    
#endif

#ifdef CONFIG_BOARD_AS3310E_FPGA_QQ
    
#endif

    /* ============== NAND ============== */
/* these pin uses type 0*/
#define NAND_PIN_TYPE           PIN_FUNCTION_0

#define GPIO_NAND_D_PORT        0    /* D0-D7 */   
#define GPIO_NAND_D_PIN_START   0 
#define GPIO_NAND_D_PIN_END     7 

#define GPIO_NAND_MISC_PORT      0
#define GPIO_NAND_RDY1_PIN     16   /* Ready 1 */ 
#define GPIO_NAND_RDN_PIN      17   /* nRead*/
#define GPIO_NAND_RDY_PIN      18   /* Ready*/ 
#define GPIO_NAND_WRN_PIN      21   /* nWrite*/ 
#define GPIO_NAND_A0_PIN       22 
#define GPIO_NAND_A1_PIN       23 
#define GPIO_NAND_A2_PIN       24 

/* these pin uses type 1 */
#define NAND_CEN_PIN_TYPE       PIN_FUNCTION_1
#define GPIO_NAND_CEN_PORT       3
#define GPIO_NAND_CEN0_PIN       0
#define GPIO_NAND_CEN1_PIN       1

/* these pin uses type 3 , and need toggle */
#define GPIO_NAND_WP_PORT       1
#define GPIO_NAND_WP_PIN        20


    /* ============== GPIO-SPI pin assign ============== */


#ifdef CONFIG_BOARD_AS3310E_FPGA_QQ
        #define GPIO_SPI_SCL_PORT   3
        #define GPIO_SPI_SCL_PIN    10
        #define GPIO_SPI_SDI_PORT   0
        #define GPIO_SPI_SDI_PIN    20
        #define GPIO_SPI_SDO_PORT   0
        #define GPIO_SPI_SDO_PIN    19		
#endif


#ifdef CONFIG_BOARD_AS3310_DEV
    #define GPIO_SPI_SCL_PORT   3
    #define GPIO_SPI_SCL_PIN    10
    #define GPIO_SPI_SDI_PORT   0
    #define GPIO_SPI_SDI_PIN    19
    #define GPIO_SPI_SDO_PORT   0
    #define GPIO_SPI_SDO_PIN    20
#endif //BORAD_DEV

#ifdef CONFIG_BOARD_AS3310_MP4_DEMO
    #define GPIO_SPI_SCL_PORT   0
    #define GPIO_SPI_SCL_PIN    9
    #define GPIO_SPI_SDI_PORT   0
    #define GPIO_SPI_SDI_PIN    10
    #define GPIO_SPI_SDO_PORT   0
    #define GPIO_SPI_SDO_PIN    11
#endif //BORAD_DEMO_RGB

        /* ============== LCD RGB Interface ============== */
#ifdef CONFIG_LCD_CONTROL
    /* these pin uses type 2*/
    #define LCD_PIN_TYPE           PIN_FUNCTION_2
    
    #define GPIO_LCD_D_PORT        1      
    #define GPIO_LCD_D_PIN_START   0 
    #define GPIO_LCD_D_PIN_END     15 
    
    #define GPIO_LCD_MISC_PORT      1
    #define GPIO_LCD_DEN_PIN        17 
    #define GPIO_LCD_VSYNC_PIN      18 
    #define GPIO_LCD_HSYNC_PIN      19 
    #define GPIO_LCD_PIXCLK_PIN     21         
    
    /* these pin uses type 3 , and need toggle */
    #define LCD_RESETN_GPIO_PROT    1
    #define LCD_RESETN_GPIO_PIN     16
    
    /*     Send Cen Signal      */
    /* these pin uses type 3 */
    #ifdef CONFIG_BOARD_AS3310_MP4_DEMO
        #define LCD_CEN_GPIO_PROT    0
        #define LCD_CEN_GPIO_PIN     8
    #endif// BORAD_DEMO_RGB
    
    #ifdef CONFIG_BOARD_AS3310E_FPGA_QQ
        #define LCD_CEN_GPIO_PROT    3
        #define LCD_CEN_GPIO_PIN     3
    #endif// CONFIG_BOARD_AS3310E_FPGA_QQ
    
    #ifdef CONFIG_BOARD_AS3310_DEV
        #define LCD_CEN_GPIO_PROT    3
        #define LCD_CEN_GPIO_PIN     3
    #endif// BORAD_DEV

    /* PWM module will handle BACKLIGHT Pin */
    #define LCD_BACKLIGHT_GPIO_PROT    3
    #define LCD_BACKLIGHT_GPIO_PIN     12

#endif //CONFIG_LCD_CONTROL

        /* ============== LCD CPU Interface ============== */
#ifdef CONFIG_LCD_IF
    /* these pin uses type 0*/
    #define LCDIF_PIN_TYPE           PIN_FUNCTION_0
    
    #define GPIO_LCDIF_D_PORT        1      
    #define GPIO_LCDIF_D_PIN_START   0 
    #define GPIO_LCDIF_D_PIN_END     15 
    
    #define GPIO_LCDIF_MISC_PORT      1
    #define GPIO_LCDIF_RESET_PIN      16 
    #define GPIO_LCDIF_RS_PIN         17 
    #define GPIO_LCDIF_WR_PIN         18 
    #define GPIO_LCDIF_CS_PIN         19 
    #define GPIO_LCDIF_BUSY_PIN       21 
    
    /* PWM module will handle BACKLIGHT Pin */
    //#define LCD_BACKLIGHT_GPIO_PROT    3
    //#define LCD_BACKLIGHT_GPIO_PIN     12

#endif //CONFIG_FBIF_AS3310



#endif // _PIN_CONTROL_H_
 
