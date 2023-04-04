@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Define apb registers base address
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        .equ            AS3310_EMI_BASE,        0x80020000
        .equ            ALPAS3310_UART1_BASE,   0x8006C000
	.equ		CFG_GBL_DATA_SIZE,	0x00000010
	.equ		REAL_RESET,		0xffff0000        
	.equ		GPIO_BASE,		0x80018000
        .equ	        ApbBase,	        0x80000000

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ exception vectors
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

@        b   __raw_dbg
        b       reset
  @      b       UNDEF_Handler
  @      b       SWI_Handler
  @      b       PABT_Handler
  @      b       DABT_Handler
  @      b       NULL_Handler
  @      b       IRQ_Handler
  @      b       FIQ_Handler

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Define stack position
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        .extern __TEXT_END

@        .equ            UND_STACK_START,        0x40004000
@        .equ            ABT_STACK_START,        0x40005000
@        .equ            FIQ_STACK_START,        0x40006000      
@        .equ            SYS_STACK_BASE,         0x40007000        
         .equ	         STACK_BASE,		 __STACK_START @- 0x180
@        .equ		 STACK_BASE,		 0x4000a000 - 0x1000

@ Put Stack either in cached memory or in SRAM for performance consideration !!!

@         .equ	         IRQ_STACK_BASE,	 __STACK_START 
@        .equ		 IRQ_STACK_BASE,	 0x4000a000

@@@@@@@@@@
@        @
@ Debug  @
@        @
@@@@@@@@@@ 
/*
.globl __raw_dbg
__raw_dbg:

        .equ            ALPAS3310_UART1_DATA,   0x8006C050
        
        mov     r0,#0x37
	ldr	r1, __uart_data
 __dbg_again:
	str	r0,	[r1]
        b       __dbg_again
 __uart_data: 
        .long   ALPAS3310_UART1_DATA
*/
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ Reset handler
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

reset:
        
	  @setup IRQ stack
 @         mrs     r0,cpsr
 @         bic     r0,r0,#0x1f
 @         orr     r0,r0,#0x12
 @         msr     cpsr,r0
 @         ldr     r0, =IRQ_STACK_BASE     /* upper 128 KiB: relocated uboot   */
 @         sub     sp, r0, #12             /* leave 3 words for abort-stack    */

          @setup SVC stack
          mrs     r0,cpsr
          bic     r0,r0,#0x1f
          orr     r0,r0,#0xd3
          msr     cpsr,r0
          ldr     r0, =STACK_BASE         /* upper 128 KiB: relocated uboot   */
          sub     r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
          sub     sp, r0, #12             /* leave 3 words for abort-stack    */
            
          /*when irq ,cpu jump to 0x0 to get the instructure*/
          mrc     p15, 0, r0, c1, c0, 0
          bic     r0, r0, #0x00002000
          mcr     p15, 0, r0, c1, c0, 0

         /*
          * we do sys-critical inits only at reboot,
          * not when booting from ram!   
          */  

@          ldr     r12, =ALPAS3310_UART1_BASE
@          ldr     r3,     =0x80000000             @ clear sft rst
@          str     r3,     [r12, #0x8]
@          ldr     r3,     =0x40000000             @ enable clk
@          str     r3,     [r12, #0x8]
@          ldr     r3,     =0x000d0470             @ set baudrate 180M/(16*38400)=292.96875=0x0124_3E70, 0x10070 for test
@          @str    r3,     [r12, #0x30]
@          ldr     r3,     =0x201                  @ enable rec
@          str     r3,     [r12, #0x24]
@          ldr     r3,     =0xc000                  @ clr hardware ctrl
@          str     r3,     [r12, #0x28]
          
          bl 		main
          mov           pc,     lr @ return

@  IRQ_Handler:
@  	@
@  	@ Save r0, lr_<exception> (parent PC) and spsr_<exception>
@  	@ (parent CPSR)
@  	@
@          sub	lr, lr, #4              @ fix lr for return
@  	stmia	sp, {r0, lr}		@ save r0, lr (already fixed)
@  	mrs	lr, spsr
@  	str	lr, [sp, #8]		@ save spsr (svc_cpsr)
@  
@  	@
@  	@ Prepare for SVC32 mode.  IRQs remain disabled.
@  	@
@  	mrs	r0, cpsr
@  	bic	r0, r0, #0x1f
@  	orr	r0, r0, #0x13   @svc mode
@  	msr	spsr_cxsf, r0   @r0 is not start the irqen
@                                  @ save to spsr, prepare for switch
@  
@  	@
@  	@ the branch table must immediately follow this code
@  	@
@  	mov	r0, sp                  @ save sp__<exception> to r0
@  	and	lr, lr, #0x0f
@  	ldr	lr, [pc, lr, lsl #2]
@  	movs	pc, lr			@ branch to handler in SVC mode
@                                          @ now switch to SVC when jumping
@  
@  __stubs_start:
@  /*
@   * Interrupt dispatcher
@   */
@           .long       __irq_invalid                   @  0  (USR_26 / USR_32)
@           .long       __irq_invalid                   @  1  (FIQ_26 / FIQ_32)
@           .long       __irq_invalid                   @  2  (IRQ_26 / IRQ_32)
@           .long       __irq_svc                       @  3  (SVC_26 / SVC_32)
@           .long       __irq_invalid                   @  4
@           .long       __irq_invalid                   @  5
@           .long       __irq_invalid                   @  6
@           .long       __irq_invalid                   @  7
@           .long       __irq_invalid                   @  8
@           .long       __irq_invalid                   @  9
@           .long       __irq_invalid                   @  a
@           .long       __irq_invalid                   @  b
@           .long       __irq_invalid                   @  c
@           .long       __irq_invalid                   @  d
@           .long       __irq_invalid                   @  e
@           .long       __irq_invalid                   @  f
@  
@   __irq_invalid:
@         @  b invalid_irq_dbg
@  
@  .align	5
@   __irq_svc:
@               @ here we entered SVC mode
@           sub     sp, sp, #72             @ save space for storing info
@           stmib   sp, {r1 - r12}          @ stmib, add sp pointer before store
@                                           @ reserve a 4 byte space for "real" r0
@  
@           ldmia   r0, {r1 - r3}           @ load from original sp_<exception>
@                                           @ now,r1:  "real" r0 
@                                           @     r2:  lr_<exception>, saved before 
@                                           @     r3:  spsr_<exception>, saved before
@                                           
@           add     r5, sp, #52             @ here for interlock avoidance
@                                           @ 52 = 13 * 4 byte (13: r0-r12)
@                                           @ now r5 continues from r13
@           
@           mov     r4, #-1                 @  ""  ""      ""       ""
@           add     r0, sp, #72             @ r0 restore to original sp_svc !!
@           str     r1, [sp]                @ save the "real" r0 copied
@                                           @ from the exception stack
@  
@           mov     r1, lr                  @ r1 restore to original lr_svc !!
@  
@           @
@           @ We are now ready to fill in the remaining blanks on the stack:
@           @
@           @  r0 - sp_svc
@           @  r1 - lr_svc
@           @  r2 - lr_<exception>, already fixed up for correct return/restart
@           @  r3 - spsr_<exception>
@           @  r4 - orig_r0 (see pt_regs definition in ptrace.h)
@           @
@           stmia   r5, {r0 - r4}     @ 
@                                     @ now r5 pointed right after "stmib  sp, {r1 - r12}"
@                                     @ so, r0 stored as r13 (sp),   [sp, #52]
@                                     @     r1 stored as r14 (lr),   [sp, #56]
@                                     @     r2 stored as r15 (pc),   [sp, #60]
@                                     @     r3 stored                [sp, #64]
@                                     @ prepared everything we need for irq return
@                                     @ just call :
@                                     @ "ldr	r0, [sp, #64]"
@                                     @ "msr	spsr_cxsf, r0"
@                                     @ "ldmia	sp, {r0 - pc}^"
@  
@           @clear_irq_out_reg
@           ldr     r2,  =ApbBase
@           ldr     r0, [r2,  #0x30]   @ read irq num
@           @ldr     r1, [r2]   @ read irq base to clear pending, should NOT read again
@  
@           @ re-enable IRQ
@           @ mrs             r2,     cpsr
@           @ bic             r2,     r2,     #0x80           @ clear the I  bit (use 0x40 for the F bit)
@           @ msr             cpsr,   r2
@  
@           ldr     r3,     [r2, #0x1f0]  @ read irq_entry
@           mov     lr,     pc
@           mov     pc,     r3 
@  
@  irq_return:
@           mrs             r0,     cpsr
@           orr             r0,     r0,     #0x80                   @ disable interrupts
@           msr             cpsr,   r0
@  
@  return:
@  	ldr	r0, [sp, #64]		@ irqs are disabled now
@  	msr	spsr_cxsf, r0
@         
@  	ldmia	sp, {r0 - pc}^		@ load r0 - pc, cpsr
@  
@  
@  irq_return_null:
@      mov  pc,  lr
@  
@          
@  UNDEF_Handler:
@          mov r0,#0x32
@          b dbg_putc
@          b reset
@          
@  SWI_Handler:
@          mov r0,#0x33
@          b dbg_putc
@          b reset
@          
@  PABT_Handler:
@          mov r0,#0x34
@          b dbg_putc
@  	b reset
@          
@  DABT_Handler:   
@          mov r0,#0x35
@          b dbg_putc
@  	b reset
@          
@  NULL_Handler:
@          mov r0,#0x36
@          b dbg_putc
@  	b reset  
@                
@  FIQ_Handler: 
@          mov r0,#0x37
@          b dbg_putc
@  	b reset


.size	startup, .-startup


