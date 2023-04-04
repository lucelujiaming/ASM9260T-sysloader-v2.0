.section .irq_entry
  b irq_0
  b irq_1
  b irq_2
  b irq_3
  b irq_4
  b irq_5
  b irq_6
  b irq_7
  b irq_8
  b irq_9
  b irq_10
  b irq_11
  b irq_12
  b irq_13
  b irq_14
  b irq_15
  b irq_16
  b irq_17
  b irq_18
  b irq_19
  b irq_20
  b irq_21
  b irq_22
  b irq_23
  b irq_24
  b irq_25
  b irq_26
  b irq_27
  b irq_28
  b irq_29
  b irq_30
  b irq_31
  b irq_32
  b irq_33
  b irq_34
  b irq_35
  b irq_36
  b irq_37
  b irq_38
  b irq_39
  b irq_40
  b irq_41
  b irq_42
  b irq_43
  b irq_44
  b irq_45
  b irq_46
  b irq_47
  b irq_48
  b irq_49
  b irq_50
  b irq_51
  b irq_52
  b irq_53
  b irq_54
  b irq_55
  b irq_56
  b irq_57
  b irq_58
  b irq_59
  b irq_60
  b irq_61
  b irq_62
  b irq_63

    .macro  m_do_irq
        stmfd	sp!,{lr}  
        bl   do_asm_irq
        ldmfd   sp!,{pc}
    .endm

.equ	ApbBase,	0x80000000

irq_0:
irq_1:
irq_2:
irq_3:
irq_4:
irq_5:
irq_6:
irq_7:
irq_8:
irq_9:
irq_10:
irq_11:
irq_12:
irq_13:
irq_14:
irq_15:
irq_16:
irq_17:
irq_18:
irq_19:
irq_20:
irq_21:
irq_22:
irq_23:
irq_24:
irq_25:
irq_26:
irq_27:   
        
irq_28:
        
        
irq_29:
        
        
irq_30:
        
        
irq_31:
irq_32:
irq_33:
irq_34:
irq_35:
        
        
irq_36: 
        
        
irq_37: 
        
        
irq_38: 
        
        
irq_39: 
        
        
irq_40: 
        
        
irq_41: 
        
        
irq_42: 
        
        
irq_43: 
        
        
irq_44: 
        
        
irq_45: 
        
        
irq_46: 
        
        
irq_47: 
        
        
irq_48: 
        
        
irq_49: 
        
        
irq_50: 
        
        
irq_51: 
        
        
irq_52: 
        
        
irq_53: 
        
        
irq_54: 
        
        
irq_55: 
        
        
irq_56: 
        
        
irq_57: 
        
        
irq_58: 
        
        
irq_59: 
        
        
irq_60: 
        
        
irq_61: 
        
        
irq_62: 
        
        
irq_63: 
        
         ldr     r2,  =ApbBase
         ldr     r0, [r2,  #0x30]   @ read irq num
        bl   do_asm_irq
        @m_do_irq       

irq_return:
         
	ldr	r0, [sp, #64]		@ irqs are disabled now
	msr	spsr_cxsf, r0
       
	ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr

        
irq_return_null:
    mov  pc,  lr
.section .bss
