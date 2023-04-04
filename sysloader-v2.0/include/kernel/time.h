
/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

*/


#ifndef __TIME_H__
#define __TIME_H__

#define TIMER0_RELOAD_DIV 100
#define TIMER1_RELOAD_DIV 45
#define TIMER2_RELOAD_DIV 90

struct timer_list {
	struct timer_list *next,*prev;
	unsigned long expires;
    int handle_later;   //if 0, handle in irq handler,if 1, handle in task chain
	void (*function)(unsigned long);
	unsigned long data;
};

struct timer_handle_list {
	struct timer_handle_list *next,*prev;
    unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

void timer_init(void);

#if CONFIG_GPIO_SPI
void timer2_init(ushort counter);
void timer2_release(void);
#endif

int do_irq_timer_0(int d);
void do_irq_timer_1(void);
int do_irq_timer_2(int d);

int timer_task_handle(void);
void test_timer(void);
int clear_irq_timer(int irq);
int add_timer(struct timer_list *t);
int del_timer(struct timer_list *t);
int mod_timer(struct timer_list *t, unsigned long expire);


extern volatile  long timer_0_div;
extern volatile  long timer_1_div;
extern volatile  long timer_2_div;
extern unsigned  long SRAM jiffies;

#endif
