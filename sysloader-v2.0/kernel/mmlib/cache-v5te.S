/*************************************************************************
	this file define some cache operation and only applify for ARM 926 EJS

	author: donier
	date: 07-08-28
**************************************************************************/

@ #include <kernel/linkage.h>

.section .text, "x"
.arm

.global add
.global IvalidDCache
.global cleanDCache
.global IvalidICache
.global cleanIvalidDCacheLineByVA
.global cleanDCacheLineByVA
.global cleanDCacheRegion
.global IvalidDCacheRegion



.equ 	CSIZE, 	14 	@cache size as 2**CSIZE (16 K assumed)
.equ 	CLINE,	5 	@cache line size in bytes as 2**CLINE
.equ 	CACHE_DLINESIZE,	32 	@cache line size in bytes 32
.equ		NWAY,	2 	@set associativity = 2**NWAY (4 way)
.equ 	I7SET,	4 	@CP15 c7 set incrementer as 2**ISET
.equ 	I7WAY,	30	@CP15 c7 way incrementer as 2**IWAY
.equ		NSET,	(CSIZE-NWAY-CLINE) @ cachelines per way = 2**NSET


add:
	mov	ip, sp
	stmdb	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4

	
	add r0, r0, r1
	
	ldmdb	fp, {fp, sp, pc}





/******************************************************
void  cleanDCache(void);              
******************************************************/
cleanDCache:
	str r0, [sp, #-4]!
	
	mov     r0, #0                     @create c7 format
next_way:
next_set:	
	mcr     p15, 0, r0, c7, c10, 2   @ clean D-cline
	add     r0, r0, #1<<I7SET         @ +1 set index
	tst     r0, #1<<(NSET+I7SET)       @ test index overflow
	beq     next_set 

	bic     r0, r0, #1<<(NSET+I7SET)  @clear index overflow
	adds    r0, r0, #1<<I7WAY         @ +1 victim pointer
	bcc     next_way                        @ test way overflow
	
	ldr r0, [sp], #4
	mov     pc, lr




/******************************************************
void  IvalidDCache();
******************************************************/
IvalidDCache:
	sub sp, sp, #4
	str r0, [sp]
	
	mov     r0, #0                     
	mcr     p15, 0, r0, c7, c6, 0   @ invalid whole D-cache
	
	ldr r0, [sp]
	add sp, sp, #4
	mov     pc, lr



/******************************************************
void  IvalidICache();
******************************************************/
IvalidICache:
	sub sp, sp, #4
	str r0, [sp]
	
	mov     r0, #0                     
	mcr     p15, 0, r0, c7, c5, 0   @ invalid whole I-cache
	
	ldr r0, [sp]
	add sp, sp, #4
	mov     pc, lr



/******************************************************
void  cleanIvalidDCacheLineByVA(void *pva);
******************************************************/
cleanIvalidDCacheLineByVA:
	mcr     p15, 0, r0, c7, c14, 1  
	mov     pc, lr



/******************************************************
void  cleanDCacheLineByVA(void *pva);
******************************************************/
cleanDCacheLineByVA:
	mcr     p15, 0, r0, c7, c10, 1  
	mov     pc, lr



/******************************************************
void  cleanDCacheRegion(void *pva, unsigned int size);
******************************************************/

	current_va	.req		r0   			@ current virtual address
	region_bytes	.req		r1   			@ size of region in bytes
	cline_count 	.req		r1   			@ number of cache lines to clean
        
cleanDCacheRegion:

	bic     current_va, current_va, #(1<<CLINE)-1       @ make current_va align with cache line size
	add     region_bytes, region_bytes, #(1<<CLINE)-1          @ aligin 32 
	mov     cline_count, region_bytes, lsr #CLINE          @ get cache line counter need be clean 
	
clean_next_cache_line:
	mcr     p15, 0, current_va, c7, c10, 1     		@ clean Dcline by virtual address
	
	add     current_va, current_va, #1<<CLINE           	@ +1 next cline adr
	subs    cline_count, cline_count, #1       @ decrease one cline counter
	bne     clean_next_cache_line                        	@ if cline_count still more than 0, then continue
	mov     pc, lr




/******************************************************
void  IvalidDCacheRegion(void *pva, unsigned int size);
******************************************************/
IvalidDCacheRegion:

	bic     current_va, current_va, #(1<<CLINE)-1       @ make current_va align with cache line size
	add     region_bytes, region_bytes, #(1<<CLINE)-1          @ aligin 32 
	mov     cline_count, region_bytes, lsr #CLINE          @ get cache line counter need be clean 
	
Ivalid_next_cache_line:
	mcr     p15, 0, current_va, c7, c6, 1     		@ clean Dcline by virtual address
	
	add     current_va, current_va, #1<<CLINE           	@ +1 next cline adr
	subs    cline_count, cline_count, #1       @ decrease one cline counter
	bne     Ivalid_next_cache_line                        	@ if cline_count still more than 0, then continue
	mov     pc, lr

.end

