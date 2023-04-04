/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Command Header file
 
Change log: 
------------------- Version 1.0  ----------------------
 Create File, define cmd struct,and sections
 He Yong 2006-11-06
*/

/*
 *  Definitions for Command Processor
 */
#ifndef __IRQ_H__
#define __IRQ_H__


#define INT_AS3310_DEBUG_RX		0
#define INT_AS3310_DEBUG_TX		1
#define INT_AS3310_LCD		    3
#define INT_AS3310_VGA			4
#define INT_AS3310_DAC_DMA		5
#define INT_AS3310_ADC_DMA		7
#define INT_AS3310_USB_DMA		11
#define INT_AS3310_I2S			12
#define INT_AS3310_NAND			13
#define INT_AS3310_I2C			14
#define INT_AS3310_SPI			15
#define INT_AS3310_GPIO0		16
#define INT_AS3310_GPIO1		17
#define INT_AS3310_GPIO2		18
#define INT_AS3310_GPIO3		19
#define INT_AS3310_HWECC		20
#define INT_AS3310_ALARM        22
#define INT_AS3310_UART_TX      23
#define INT_AS3310_UART_ERR     24
#define INT_AS3310_UART_RX      25
#define INT_AS3310_TIMER0		28
#define INT_AS3310_TIMER1		29
#define INT_AS3310_TIMER2		30
#define INT_AS3310_TIMER3		31
#define INT_AS3310_MONIT_EXCEED		32
#define INT_AS3310_MONIT_FINISH		33

#define INT_AS3310_TS_DETECT    36  
#define INT_AS3310_LRADC_CH0    37  
#define INT_AS3310_LRADC_CH1    38  
#define INT_AS3310_LRADC_CH2    39  
#define INT_AS3310_LRADC_CH3    40  
#define INT_AS3310_LRADC_CH4    41  
#define INT_AS3310_LRADC_CH5    42  
#define INT_AS3310_LRADC_CH6    43  
#define INT_AS3310_LRADC_CH7    44  


#define INT_AS3310_USB_CTRL		57

#ifndef NULL
#define NULL	0
#endif

#ifndef	__ASSEMBLY__
/*
 * Monitor Command Table
 */

typedef struct irq_action {
	int		irq;        /* IRQ Number */
	int		(*irq_handler)(int pri);   /* handler */
	int		(*clear)(int pri);   /* clear source if needed */
	int 	priv_data;        /* user saved irq data */
}irq_action_t;


#endif	/* __ASSEMBLY__ */

void enable_irq(int irq);
void disable_irq(int irq);
int request_irq(irq_action_t * irq_desc);
int release_irq(int irq);
int free_irq(int irq);


int do_irq(cmd_tbl_t *cmdtp,int argc,char * argv[]);
int do_disirq(cmd_tbl_t *cmdtp,int argc,char * argv[]);
void invalid_irq_dbg(void);
void do_asm_irq(int irq);

#endif	/* __IRQ_H__ */
