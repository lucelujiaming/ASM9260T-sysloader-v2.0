/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader IRQ Source file

------------------- Version 1.0  ----------------------
Create File, Support IRQ handler
 He Yong 2006-11-06

*/

#include <common.h>

irq_action_t irq_table[64];  /* totally 64 IRQs*/

/* enable IRQ interrupts */
void enable_interrupts (void)
{
	unsigned long temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "bic %0, %0, #0x80\n"
			     "msr cpsr_c, %0"
			     : "=r" (temp)
			     :
			     : "memory");
}

/*
 * disable IRQ/FIQ interrupts
 * returns true if interrupts had been enabled before we disabled them
 */

int disable_interrupts (void)
{
	unsigned long old,temp;
	__asm__ __volatile__("mrs %0, cpsr\n"
			     "orr %1, %0, #0xc0\n"
			     "msr cpsr_c, %1"
			     : "=r" (old), "=r" (temp)
			     :
			     : "memory");
	return (old & 0x80) == 0;
}


/* nothing really to do with interrupts */
int interrupt_init (void)
{
	outl(0xc0000000, INT_RWCTRL+8);   // clear CLKGATE
    outl(0x00050000, INT_RWCTRL);   // Enable  IRQ  (FIQ)
    //outl(0x000d0000, INT_RWCTRL);   // no nest and Enable  IRQ  (FIQ)
    memset((char * )(INT_RWPRIORITY0),0,0x100);

    memset((char * )(irq_table),0,sizeof(irq_table));

#if CONFIG_IRQ_AS3310D
    outl((ulong)(&__INT_ENTRY_START), INT_IVBA);   // set irq_table addr
#endif


#if CONFIG_IRQ_AS3310E
    outl(0, INT_IVBA);   // set irq_entry addr
    outl(do_asm_irq, AS3310_INT_BASE_ADDR);   // set irq_entry addr
#endif
    
 //   puts("IRQ inited, Entry = 0x");
 //   puth((ulong)(&__INT_ENTRY_START));
 //   putc('\n');

	/* init the timestamp and lastdec value */
 //   enable_interrupts();
	return (0);
}

void enable_irq(int irq){
    if ((irq<0)||(irq>=64)) {
        printf("enable_irq: Invalid IRQ (%d)\n",irq);
        return;
    }
    outl((4<<((irq&3)*8)), INT_RWPRIORITY0 + ((irq>>2)*0x10) + 4);
}

void disable_irq(int irq){
    if ((irq<0)||(irq>=64)) {
        printf("disable_irq: Invalid IRQ (%d)\n",irq);
        return;
    }
    outl((4<<((irq&3)*8)), INT_RWPRIORITY0 + ((irq>>2)*0x10) + 8);
}


int request_irq(irq_action_t * irq_desc){

    int irq = irq_desc->irq;

    if ((irq<0)||(irq>=64)) {
        printf("request_irq: Invalid IRQ (%d)\n",irq);
        return -1;
    }

    if (irq_table[irq].irq_handler != NULL) {
        printf("request_irq: Error there's another irq_handler (%p) already registered.\n"
               ,irq_table[irq].irq_handler);
        return -1;
    }

    irq_table[irq].irq = irq;
    irq_table[irq].irq_handler = irq_desc->irq_handler;
    irq_table[irq].clear = irq_desc->clear;
    irq_table[irq].priv_data = irq_desc->priv_data;

    enable_irq(irq);

    return 0;
}

int free_irq(int irq){

    if ((irq<0)||(irq>=64)) {
        printf("release_irq: Invalid IRQ (%d)\n",irq);
        return -1;
    }

    disable_irq(irq);

    irq_table[irq].irq = 0;
    irq_table[irq].irq_handler = NULL;
    irq_table[irq].clear = NULL;
    irq_table[irq].priv_data = 0;

    return 0;
}


int do_irq(cmd_tbl_t *cmdtp,int argc,char * argv[]){
 puts("Turn On CPU IRQ...\n");
 enable_interrupts();
}

int do_disirq(cmd_tbl_t *cmdtp,int argc,char * argv[]){
 puts("Turn Off CPU IRQ...\n");
 disable_interrupts();
}

void invalid_irq_dbg()
{
    puts("\nInvalid irq mode - not from SVC32\n");
}


/* Main IRQ Serice handler */

void do_asm_irq(int irq){
    unsigned long level;  

   if ((irq<0)||(irq>=64)) {
       printf("do_asm_irq: Invalid IRQ (%d)\n",irq);
       return ;
   }

   /* ========= clear irq source ================*/
   if (irq_table[irq].clear != NULL) {
       irq_table[irq].clear(irq_table[irq].priv_data);
   }

   /* ========= handle irq  ================*/
   if (irq_table[irq].irq_handler != NULL) {
       irq_table[irq].irq_handler(irq_table[irq].priv_data);
   }
   else{
       if (irq != 0x3f) {
           puts("Ack unexpected irq(");putb(irq);puts(")\n");
       }
       //puts("Ack unexpected irq(");putb(irq);puts(")\n");
   }

   /* ========= disable IRQ when prepare to return ================*/

   level = inl(INT_DEBUG0)>>28;

   /* ========= clear irq controller ================*/

   outl((0x00000001 << (irq&0x1f)), INT_IRQCLEAR0 + ((irq>>5)*0x10) + 4 ); // set

   disable_interrupts();
   /* ========= clear irq level  ================*/
	if (level >= 8){
		outl(8, INT_LEVACK);
	}
	else if (level >= 4){   
		outl(4, INT_LEVACK);
	}
	else if (level >= 2){   
		outl(2, INT_LEVACK);
	}
	else outl(1, INT_LEVACK);
}

 BOOT_CMD(irq,do_irq,
         " #irq",
         "Enable CPU IRQ");

 BOOT_CMD(disirq,do_disirq,
         " #disirq",
         "Disable CPU IRQ");

