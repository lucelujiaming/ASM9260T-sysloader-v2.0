/*
asmalloc
A Simple Memory Allocator for AlphaScale co,Ltd
this tool is developed mainly for hardware debug programs
which need such a feature.

He Yong,    alphascale soft engineer  heyong@alpsacle.com
zhangbo

=================  Change Log   =======================
version 0.1:
    malloc, calloc, realloc, free
    support normal memory and none-cache/bufferable memory

*/

#ifndef __ASMALLOC_H__
#define __ASMALLOC_H__

    /*      Malloc Config      */
    
#ifdef CONFIG_NONCACHE_TO_USER

#define CACHE_MALLOC_START          (long)&__GLUE_7_START //space for cache malloc
#define CACHE_MALLOC_END            0x20a00000//space for cache malloc
#define CACHE_MALLOC_LEN            (CACHE_MALLOC_END - CACHE_MALLOC_START)     /* Space for  malloc */

#else  //CONFIG_NONCACHE_TO_USER

#define CACHE_MALLOC_START          (long)&__GLUE_7_START //space for cache malloc
#define CACHE_MALLOC_END            ((long)&__STACK_START - (CONFIG_STACK_LEN*0x400))//space for cache malloc
#define CACHE_MALLOC_LEN            (CACHE_MALLOC_END - CACHE_MALLOC_START)     /* Space for  malloc */

#endif //CONFIG_NONCACHE_TO_USER

#define NONCACHE_MALLOC_START       (long)&__NONCACHE_MALLOC_START //space for cache malloc
#ifdef CONFIG_SDRAM_8MB
#define NONCACHE_MALLOC_END         0x20800000 //space for cache malloc
#endif
#ifdef CONFIG_SDRAM_16MB
#define NONCACHE_MALLOC_END         0x21000000 //space for cache malloc
#endif
#define NONCACHE_MALLOC_LEN         (NONCACHE_MALLOC_END - NONCACHE_MALLOC_START)      /* Space for  malloc */





#define BIN_NORMAL       0
#define BIN_NONECACHED   1

#define NUMBER_OF_BINS      2   /* one for normal, one for none-cached */
#define NUMBER_OF_REGIONS   3   /* one for small alloc, one for big */   

#define CHUNK_FREE          1
#define CHUNK_INUSE         2
#define CHUNK_END           9
#define MALLOC_NO_MEM       -2

#ifndef NULL
#define NULL  (void *)0
#endif

#define SIZE_SMALL  128
#define SIZE_MIDDLE 1280

/*                                       used in vc
#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _SIZE_U_DEFINED
typedef unsigned char uchar;
#define _SIZE_U_DEFINED
#endif
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _as_chunk_{
    int status;
    int malloc_type;
    size_t size;
    void * previous;
    void * next;
    unsigned long mem_h;
}AS_CHUNK;

typedef struct _region_info_{
    void * current_chunk;
    char * start;
    int size;
    int rsvd;
}REGION_INFO;

typedef struct _as_bin_info_{
    int type;
    int start;
    int size;
    int number_of_regions;
//    char * regions[NUMBER_OF_REGIONS];
    REGION_INFO regions[NUMBER_OF_REGIONS];
}AS_BIN_INFO;


//extern AS_BIN_INFO as_info[NUMBER_OF_BINS];


/* ------------------- Error Message codes ------------------- */
#define INVALID_BIN_INDEX -1

/* ------------------- Declarations of public routines ------------------- */

#define USE_AS_PREFIX
#ifndef USE_AS_PREFIX
#define ascalloc               calloc
#define asfree                 free
#define asmalloc               malloc
#define asrealloc              realloc
#endif 
/* USE_AS_PREFIX */


int init_asmalloc(void * mem, int size, int bin_index);
int init_asmalloc_cache(void * mem, int size);
int init_asmalloc_nonecache(void * mem, int size);



/*
  malloc(int n)
  Returns a pointer to a newly allocated chunk of at least n bytes, or
  null if no space is available, in which case errno is set to ENOMEM
  on ANSI C systems.

  If n is zero, malloc returns a minimum-sized chunk. (The minimum
  size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
  systems.)  Note that int is an unsigned type, so calls with
  arguments that would be negative if signed are interpreted as
  requests for huge amounts of space, which will often fail. The
  maximum supported value of n differs across systems, but is in all
  cases less than the maximum representable value of a int.
*/
void* asmalloc(int,int);

/*deferent uses of asmalloc*/
void * c_malloc(int size_m);
void * nc_malloc(int size_m);



/*
  free(void* p)
  Releases the chunk of memory pointed to by p, that had been previously
  allocated using malloc or a related routine such as realloc.
  It has no effect if p is null. If p was not malloced or already
  freed, free(p) will by default cause the current program to abort.
*/
void  asfree(void*);


/*
  calloc(int n_elements, int element_size);
  Returns a pointer to n_elements * element_size bytes, with all locations
  set to zero.
*/
void* ascalloc(int,int,int);
/*deferent uses of ascalloc*/
void * c_calloc(int n_elements, int element_size);
void * nc_calloc(int n_elements, int element_size);


/*
  realloc(void* p, int n)
  Returns a pointer to a chunk of size n that contains the same data
  as does chunk p up to the minimum of (n, p's size) bytes, or null
  if no space is available.

  The returned pointer may or may not be the same as p. The algorithm
  prefers extending p in most cases when possible, otherwise it
  employs the equivalent of a malloc-copy-free sequence.

  If p is null, realloc is equivalent to malloc.

  If space is not available, realloc returns null, errno is set (if on
  ANSI) and p is NOT freed.

  if n is for fewer bytes than already held by p, the newly unused
  space is lopped off and freed if possible.  realloc with a size
  argument of zero (re)allocates a minimum-sized chunk.

  The old unix realloc convention of allowing the last-free'd chunk
  to be used as an argument to realloc is not supported.
*/
/*deferent uses of ascalloc*/
void* c_realloc(void*, int);
void* nc_realloc(void*, int);

int Test_malloc(void);
int as_get_error_code();




#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */


#endif //__ASMALLOC_H__

