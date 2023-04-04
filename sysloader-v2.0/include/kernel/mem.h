/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

Memory Manage Header File
MMU, malloc, etc.
AlpScale 2007-01-21
Hoffer
*/

#ifndef  __MEM_H__
#define  __MEM_H__

#include <common.h>

#define PACKED __attribute__((packed))


/*  ================    First Level    =============   */

#define TRANSLATION_BASE 0x20000000 /* MMU TABLE BASE ADDR*/

#define AP_RW 0x3    /* 11b for Read/Write */

#define M_TYPE_COARSEPAGE   1
#define M_TYPE_SECTION      2
#define M_TYPE_FINEPAGE     3

typedef union _descrp_section_lv1{
    struct _descrp_section_lv1_{
        ulong type      : 2;    // 10b for section table
        ulong Buffer    : 1;    // buffer-able
        ulong Cache     : 1;    // Cachable
        ulong IMP       : 1;    // implementation defined
        ulong domain    : 4;    // up to 6 domains per section
        ulong SBZ0      : 1;    // should be Zero
        ulong AP        : 2;    // 11b for Read/Write
        ulong SBZ       : 8;    // should be ZERO
        ulong PhyAddr   : 12;
    } PACKED field ;
    ulong value;
}M_SECTOR_DESCRPTOR;


typedef union _descrp_coarse_page_base{
    struct _descrp_coarse_page_base_{
        ulong type          : 2;   // 01b for coarse page table base
        ulong IMP           : 3;    // implementation defined
        ulong domain        : 4;    // up to 6 domains per section
        ulong SBZ           : 1;    // should be ZERO
        ulong BasePhyAddr   : 22;
    } PACKED field ;
    ulong value;
}M_COARSE_PAGE_BASE_DESCRPTOR;

/*  ================    Second Level    =============   */

#define M_TYPE_LARGEPAGE   1    /*  128KB  */
#define M_TYPE_SMALLPAGE   2    /*  8KB  */
#define M_TYPE_TINYPAGE    3    /*  2KB */

typedef union _descrp_page_lv2{
    struct _descrp_page_lv2_{
        ulong type          : 2;    // 
        ulong Buffer        : 1;    // buffer-able
        ulong Cache         : 1;    // Cachable
        ulong AP            : 6;    // should be 0x111111
        ulong PhyAddr       : 22;
    } PACKED field ;
    ulong value;
}M_LV2_PAGE_DESCRPTOR;

#define M_CACHE_OFF     0
#define M_CACHE_ON      1

#define M_BUFFER_OFF    0
#define M_BUFFER_ON     1

#define M_READWRITE     3

/*  ================      =============   */

typedef struct _m_map {
    ulong   phy_addr;
    ulong   virtual_addr;
    ulong   map_length ;
    uchar   buffer;
    uchar   cache;
    uchar   map_type;
    uchar   rsvd;
} PACKED M_MAP;


#define SIZE_1K     0x00000400
#define SIZE_2K     0x00000800
#define SIZE_128K   0x00020000
#define SIZE_512K   0x00080000
#define SIZE_1M     0x00100000


#define CP15R1_MMU_ENABLE               (1<<0)
#define CP15R1_ALIG_ENABLE              (1<<1)
#define CP15R1_CACHE_ENABLE             (1<<2)
#define CP15R1_BUFFER_ENABLE            (1<<3)
#define CP15R1_BIG_ENDIAN_ENABLE        (1<<7)
#define CP15R1_SYS_PROTECT_ENABLE       (1<<8)
#define CP15R1_ROM_PROTECT_ENABLE       (1<<9)
#define CP15R1_BRANCH_PRETECT_ENABLE    (1<<11)
#define CP15R1_I_CACHE_ENABLE           (1<<12)
#define CP15R1_HIGH_VECTOR_ENABLE       (1<<13)
#define CP15R1_RR_ENABLE                (1<<12)


/* See also ARM Ref. Man. */
#define C1_MMU		(1<<0)		/* mmu off/on */
#define C1_ALIGN	(1<<1)		/* alignment faults off/on */
#define C1_DC		(1<<2)		/* dcache off/on */
#define C1_WB		(1<<3)		/* merging write buffer on/off */
#define C1_BIG_ENDIAN	(1<<7)	/* big endian off/on */
#define C1_SYS_PROT	(1<<8)		/* system protection */
#define C1_ROM_PROT	(1<<9)		/* ROM protection */
#define C1_IC		(1<<12)		/* icache off/on */
#define C1_HIGH_VECTORS	(1<<13)	/* location of vectors: low/high addresses */
#define RESERVED_1	(0xf << 3)	/* must be 111b for R/W */


ulong make_first_level_desc(ulong phyaddr,int buffer,int cache,int type);
ulong make_second_level_desc(ulong phyaddr,int buffer,int cache,int type);
void create_mmap(M_MAP * m_map,ulong virtual_addr,ulong phyaddr,int buffer,int cache,int type);
void io_remap_lv1(ulong * translation_base,M_MAP * m_map);
void create_page_table(ulong * translation_base,M_MAP m_list[], int n);
void create_coarse_page_table(ulong * second_lv_base,M_MAP  m_list[],int n);
int cleanup_before_linux (void);
void init_page_table(void);
void enable_mmu(ulong return_address);

int Drain_write_buffer (void);
int Clean_data_cache_line (void * virtual_addr);

void icache_enable (void);
void icache_disable (void);

void dcache_enable (void);
void dcache_disable (void);

ulong make_section_desc(ulong phyaddr,int buffer,int cache,int domain);

#define set_trans_base(x)					\
	do {						\
	__asm__ __volatile__(				\
	"mcr	p15, 0, %0, c2, c0,	0 @ set trans_base"	\
	  : : "r" (x));					\
	} while (0)


 /*
  * Domain numbers
  *
  *  DOMAIN_IO     - domain 2 includes all IO only
  *  DOMAIN_USER   - domain 1 includes all user memory only
  *  DOMAIN_KERNEL - domain 0 includes all kernel memory only
  */
 #define DOMAIN_KERNEL   0
 #define DOMAIN_TABLE    0
 #define DOMAIN_USER 1
 #define DOMAIN_IO   2

 /*
  * Domain types
  */
 #define DOMAIN_NOACCESS 0
 #define DOMAIN_CLIENT   1
 #define DOMAIN_MANAGER  3

 #define domain_val(dom,type)    ((type) << (2*(dom)))

 #ifndef __ASSEMBLY__
 #define set_domain(x)                   \
     do {                        \
     __asm__ __volatile__(               \
     "mcr    p15, 0, %0, c3, c0,  0 @ set domain"   \
       : : "r" (x));                 \
     } while (0)

 #endif /* !__ASSEMBLY__ */


#define Struct_Section_BOOT_INFO    __attribute__ ((section (".boot_info")))
#define Section_SOURCE           /* __attribute__ ((section (".source")))*/
#define Section_TEXTLIB             __attribute__ ((section (".textlib")))

//#define NCached                     __attribute__ ((section (".non_cached")))
//#define SRAM                        __attribute__ ((section (".sram")))
#define NCached     //                __attribute__ ((section (".non_cached")))
#define SRAM        //                __attribute__ ((section (".sram")))


#endif // __MEM_H__
