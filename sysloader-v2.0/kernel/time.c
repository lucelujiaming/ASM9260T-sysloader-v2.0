
/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader IRQ Source file

------------------- Version 1.0  ----------------------
Create File, Support IRQ handler
 He Yong 2006-11-06

*/

#include <common.h>
#include <kernel/time.h>

volatile long timer_0_div;
volatile long timer_1_div;
volatile long timer_2_div;
unsigned long SRAM jiffies;
struct timer_list SRAM *timer_base;
struct timer_list SRAM *timer_act;
struct timer_handle_list SRAM *handle_head;
char irqing;


struct timer_list test1,test2,test3;

int clear_irq_timer(int irq){
    outl(0x00008000, TIMER0_CTRL + (irq-INT_AS3310_TIMER0)*0x20 + 0x8);
    return 0;
}

void timer_init(){

    irq_action_t timer_irq;

    jiffies = 0;
    timer_base = NULL;
    handle_head =NULL;

    outl(0xc0000000, TIMER_ROTCTRL+8);   // clear CLKGATE

    // Timer 0 init
    outl(0x0000407f, TIMER0_CTRL); // 24MHz devided by 8
    outl(0x75307530, TIMER0_COUNTER);  // 100 Hz
    timer_0_div = 0;
    
    timer_irq.irq = INT_AS3310_TIMER0;
    timer_irq.irq_handler = do_irq_timer_0;
    timer_irq.clear = clear_irq_timer;
    timer_irq.priv_data = INT_AS3310_TIMER0;
    request_irq(&timer_irq);

#if CONFIG_TIMER_1
    // Timer 1 init
    outl(0x0000407f, TIMER1_CTRL); // 24MHz devided by 8
    outl(0xffffffff, TIMER1_COUNTER);  // 45 Hz
    timer_1_div = 0;

    timer_irq.irq = INT_AS3310_TIMER1;
    timer_irq.irq_handler = do_irq_timer_1;
    timer_irq.clear = clear_irq_timer;
    timer_irq.priv_data = INT_AS3310_TIMER1;
    request_irq(&timer_irq);
#endif //CONFIG_TIMER_1


#if CONFIG_GPIO_UART
    // Timer 2 init
    outl(0x0000407f, TIMER2_CTRL); // 24MHz devided by 8
    outl(GPIO_UART_BANDRATE_38400 & (GPIO_UART_BANDRATE_38400<<16), TIMER2_COUNTER);  // 45 Hz
    timer_2_div = 0;
#endif

    puts("Timer Inited.\n");

}

#if CONFIG_GPIO_SPI

void timer2_init(ushort counter){
    printf("Timer 2 Inited (%d)\n",counter);
    outl(0x0000407f, TIMER2_CTRL); // 24MHz devided by 8
    outl(0xffff0000+counter, TIMER2_COUNTER); 

}
void timer2_release(void){
    printf("Timer 2 Released\n");
    outl(0x0000003f, TIMER2_CTRL); // stop Timer
}


#endif


int do_irq_timer_0(int d){

    struct timer_handle_list *t,*l;
    struct timer_list *get;
    int i;
    /********** main timer counter *********/
    irqing = 1;
    jiffies++;
    //printf("jiffies:%d\n",jiffies);
    //printf("irq vector:0x%x\n",inl(INT_IVAR));
    //timer_base = NULL;

#ifdef CONFIG_TIMER_LIST

    if (timer_base != NULL) {
        while(timer_base->expires == jiffies){

    #ifdef CONFIG_TIMER_LIST_DEBUG
            get = timer_base;
            i=0;
            while (get != NULL) {
                 i++;
                 alp_printf("%d=>",get->expires);
                 get = get->next;
             }
            printf("  ");
            if(i)printf("timer_list number:%d\n",i);
    #endif //CONFIG_TIMER_LIST_DEBUG

            timer_act = timer_base;
            timer_base = timer_base->next;
            if (timer_act->handle_later) {
                //printf("timer later? :%d  ",timer_act->handle_later);
                //printf("timer_act->expires:%d\n",timer_act->expires);
                t = c_malloc(sizeof(struct timer_handle_list));
                if (t == NULL) {
                    printf("panic:do_irq_timer_0() malloc fail\n");
                    while(1);
                }
                t->next = t->prev = NULL;
                t->function = timer_act->function;
                t->data = timer_act->data;
                t->expires = timer_act->expires;
                //asfree(timer_act);
                timer_act->next = timer_act->prev = NULL;

                //find the tail of handle list
                l = handle_head;
                if (l == NULL) {
                    handle_head = t;
                   //printf("new handler added,addr:0x%x\n",handle_head->function);
                }else {
                    while (l->next != NULL) {
                        l = l->next;
                    }
                    t->prev = l;
                    l->next = t;
                }
            }else {
                //printf("timer_act->expires:%d ",timer_act->expires);
                (*timer_act->function)(timer_act->data);
                //asfree(timer_act);
                timer_act->next = timer_act->prev = NULL;
                
            }
          
            if (timer_base == NULL)break;
        }//while(timer_base->expires == jiffies)
    }

#endif // CONFIG_TIMER_LIST

   if (timer_0_div-- <=0 ) {
        #if CONFIG_TIMER_DEBUG 
        putc('0');
        #endif // __TIMER_DEBUG__ 
        timer_0_div = TIMER0_RELOAD_DIV;
    }    
#if CONFIG_WATCHDOG_TIMER
    if (watch_dog == 0) {
        /* wang wang !!! */
        printf("Watch dog Dead!\n");
    
        /* do what needed to be done here ! */
    
        //do_usbdump(0,0,0);
    
        printf("System Panic!\n");
        while(1);
    }else {
        /* we are still alive */
        watch_dog = 0;
    }
#endif
irqing = 0;
return 0;
}

#if CONFIG_TIMER_1
void do_irq_timer_1(){
    if (timer_1_div-- <=0 ) {
        #if CONFIG_TIMER_DEBUG 
        putc('1');
        #endif // __TIMER_DEBUG__ 
        timer_1_div = TIMER1_RELOAD_DIV;
    }
}
#endif// CONFIG_TIMER_1

int do_irq_timer_2(int d){

    #if CONFIG_GPIO_UART
    gpio_uart_interrupt_handler();
    #endif // CONFIG_GPIO_UART
    
    #if CONFIG_GPIO_SPI
    gpio_spi_interrupt_handler();
    #endif // CONFIG_GPIO_SPI
    return 0;
}


#ifdef CONFIG_TIMER_LIST

int add_timer(struct timer_list *t)
{
    struct timer_list *f,*timer;
    if ((t->next)||(t->prev)) {
        alp_printf("Warning:This timer added already\n");
        return -1;
    }
    //timer = c_malloc(sizeof(struct timer_list));
    //if (timer == NULL) {
    //    printf("panic:add_timer() malloc fail\n");
    //    while(1);
    //}
    //timer->data = t->data;
    //timer->expires = t->expires;
    //timer->function = t->function;
    //timer->handle_later = t->handle_later;
    //timer->next = timer->prev = NULL;

    //disable_irq(INT_AS3310_TIMER0);
    //printf("in add_timer1\n");
    //printf("time now:%d\n",jiffies);

    f = timer_base;
    if (f == NULL) {
        timer_base = t;
    }else if (t->expires <= f->expires) {
        t->next = f;
        f->prev = t;
        timer_base = t;
        }else {
            while (t->expires > f->expires) {
                if (f->next == NULL) {
                    f->next = t;
                    t->prev = f;
                    return 0;
                }else f = f->next;
            }
            t->next = f;
            t->prev = f->prev;
            f->prev->next = t;
            f->prev = t;
        }
    //printf("in add_timer2\n");
    //enable_irq(INT_AS3310_TIMER0);
    return 0;

}

int del_timer(struct timer_list *t)
{
    while (irqing);
    disable_irq(INT_AS3310_TIMER0);
    while (irqing) {
    }
    if ((t->next == NULL)&&(t->prev == NULL)) {
        if (timer_base != t) {
            alp_printf("Warning:This timer isn't added\n");
            return -1;
        }
    }
    if (t == timer_base) {
        if (t->next == NULL) {
            timer_base = NULL;
            t->next = t->prev = NULL;
        }else {
            timer_base = t->next;
            t->next->prev = NULL;
            t->next = t->prev = NULL;
        }
    }else if (t->next == NULL) {
        t->prev->next = NULL;
        t->next = t->prev = NULL;
    }else {
        t->prev->next = t->next;
        t->next->prev = t->prev;
        t->next = t->prev = NULL;
    }
    enable_irq(INT_AS3310_TIMER0);

}

int mod_timer(struct timer_list *t, unsigned long expire)
{
    int ret;
    if (del_timer(t) < 0) {
        printf("mod_timer error, this timer isn't in list\n");
        return -1;
    }
    t->expires = expire;
    while (irqing);
    disable_irq(INT_AS3310_TIMER0);
    add_timer(t);
    enable_irq(INT_AS3310_TIMER0);
    return 0;
}

int timer_task_handle(void){
    struct timer_handle_list *get,*act;

#ifdef CONFIG_TIMER_LIST_DEBUG
    int i=0;
    get = handle_head;
    //alp_printf("\nin timer_handle_list stored irq:\n");
    while (get != NULL) {
        i++;
        printf("%d >>> ",get->expires);
        get = get->next;
    }
    if(i)printf("   number:%d\n",i);
#endif //CONFIG_TIMER_LIST_DEBUG

    disable_irq(INT_AS3310_TIMER0);
    get = handle_head;
    handle_head = NULL;
    enable_irq(INT_AS3310_TIMER0);
    while (get != NULL) {
        act = get;
        get = get->next;
        (*act->function)(act->data);
        //printf("timer_task_handle time: %d\n",jiffies);
        asfree(act);
    }
    return 0;
}

#endif //CONFIG_TIMER_LIST

#ifdef CONFIG_TIMER_LIST_DEBUG
void test1_func(unsigned long d){
    printf("=test1 timer----->time:%d\n",jiffies);
    //test1.expires = jiffies + 250;
    //add_timer(&test1);
}
void test2_func(unsigned long d){
    printf("===test2 timer----->time:%d\n",jiffies);
    test2.expires = jiffies + 240;
    add_timer(&test2);
    test1.expires = jiffies + 200;
    add_timer(&test1);
    //test3.expires = jiffies + 90;
    //add_timer(&test3);
}
void test3_func(unsigned long d){
    printf("======test3 timer----->time:%d\n",jiffies);
    test3.expires = jiffies + 150;
    add_timer(&test3);
    test1.expires = jiffies + 300;
    add_timer(&test1);
}

void test_timer(void){
    test1.next = test1.prev = NULL;
    test2.next = test2.prev = NULL;
    test3.next = test3.prev = NULL;
    test1.handle_later = 0;
    test2.handle_later = 0;
    test3.handle_later = 0;
    test1.function = test1_func;
    test2.function = test2_func;
    test3.function = test3_func;
    test1.expires = jiffies + 100;
    test2.expires = jiffies + 180;
    test3.expires = jiffies + 80;
    printf("original test1:%d test2:%d test3:%d\n",test1.expires,test2.expires,test3.expires);
    add_timer(&test1);    
    add_timer(&test2);
    add_timer(&test3);
}

#endif //CONFIG_TIMER_LIST_DEBUG


//static int test_in(void){
//    printf("timer task in\n");
//}
//static int test_out(void){
//    printf("timer task out\n");
//}
//#ifdef CONFIG_TIMER_H_0
//int task_timer_init(){
//    if(task_mark(TASK_HIGH,0,timer_task_handle) < 0)
//        panic("timer_task_handle task_mark panic\n");
//    return 0;
//}
//__init_task(task_timer_init);
//#endif

#ifdef CONFIG_TIMER_LIST
t_ops t_timer = {
    .tname      = "as3310_timer",
    .tid        = 1000,
    .task_init  = NULL,
    .task_in    = NULL,
    .task_out   = NULL,
    .tfunc      = timer_task_handle,
};

__define_init_task(t_timer);


#endif// CONFIG_TIMER_LIST

