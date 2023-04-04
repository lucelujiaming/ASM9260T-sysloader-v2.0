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

#include <common.h>

#ifdef CONFIG_MALLOC
static AS_BIN_INFO as_info[NUMBER_OF_BINS];
#ifdef CONFIG_MEM_SIZE
int mem_size;
#endif //CONFIG_MEM_SIZE

#define REGION_SPACE 0x20   /*  spaces between regions */
#define INFO_SIZE 0x14 /*info space used for one chunk*/

int init_asmalloc(void * mem, int size, int bin_index){
int i;
AS_CHUNK * chunk_start, * chunk_end;

    #ifdef CONFIG_MEM_SIZE
    mem_size = 0;
    #endif //CONFIG_MEM_SIZE

    if (( bin_index < 0 )||(bin_index >= NUMBER_OF_BINS)) {
        return INVALID_BIN_INDEX;
    }

    /*  initial Regions */
    size = size & (~0x3UL);
    mem =  (void *)(((unsigned long)mem + 0x3)& (~0x3UL));
    as_info[bin_index].number_of_regions = NUMBER_OF_REGIONS;
    as_info[bin_index].size = size;
    as_info[bin_index].start = (unsigned long)mem;
    as_info[bin_index].type = bin_index;

    as_info[bin_index].regions[0].start = mem;
    as_info[bin_index].regions[0].current_chunk = mem;
    for ( i = 1 ; i < NUMBER_OF_REGIONS ; i++ ) {
        /*   /4 policy, small region is 1/4 size of the larger one   */
        as_info[bin_index].regions[i].start = (char *)
            (((unsigned long)mem + (size>> ((NUMBER_OF_REGIONS - i) * 2))) & (~0x3UL));
        as_info[bin_index].regions[i-1].size = 
            (unsigned long)(as_info[bin_index].regions[i].start - as_info[bin_index].regions[i-1].start);
        as_info[bin_index].regions[i].current_chunk = as_info[bin_index].regions[i].start;
    }
    as_info[bin_index].regions[NUMBER_OF_REGIONS-1].size = 
        ((unsigned long)mem + size) - (unsigned long)as_info[bin_index].regions[NUMBER_OF_REGIONS-1].start;

    /*  initial chunks */
    for ( i = 0 ; i < NUMBER_OF_REGIONS ; i++ ) {
        chunk_start = (AS_CHUNK *)as_info[bin_index].regions[i].start;
        chunk_end = (AS_CHUNK *)(as_info[bin_index].regions[i].start 
            + as_info[bin_index].regions[i].size - REGION_SPACE);
        chunk_start->next = chunk_end;
        chunk_start->previous = chunk_end;
        chunk_start->size = (unsigned long)chunk_end - (unsigned long)(&chunk_start->mem_h);
        chunk_start->status = CHUNK_FREE;

        //printf("chunk %d size:0x%x\n",i,chunk_start->size);

        chunk_end->next = chunk_start;
        chunk_end->previous = chunk_start;
        chunk_end->size = 0 ;
        chunk_end->status = CHUNK_END;
    }

return 0;
}


void * asmalloc(int size_malloc, int bin_index)
{
	uchar region_num;
	AS_CHUNK * current, * chunk_search, * chunk_next;

    //printf("size_malloc:%d\n",size_malloc);
    if ((size_malloc % 4)!=0) {
        size_malloc = ((size_malloc + 0x3)&(~0x3UL));
        //printf("after resize, size_malloc:%d\n",size_malloc);
    }
	if(size_malloc<SIZE_SMALL)region_num=0;
		else if(size_malloc<SIZE_MIDDLE)region_num=1;
			else region_num=2;

    //printf("region_num:%d\n",region_num);
    #ifdef CONFIG_MEM_SIZE_C
    if (bin_index == BIN_NORMAL) {
        mem_size += size_malloc;
        alp_printf("Cache Malloc memory size:0x%x\n",mem_size);
    }
    #endif //CONFIG_MEM_SIZE_C
    
    #ifdef CONFIG_MEM_SIZE_NC
    if (bin_index == BIN_NONECACHED) {
        mem_size += size_malloc;
        alp_printf("Non-Cache Malloc memory size:0x%x\n",mem_size);
    }
    #endif //CONFIG_MEM_SIZE_NC


	while(region_num<NUMBER_OF_REGIONS)
	{
        chunk_search = (AS_CHUNK *)as_info[bin_index].regions[region_num].current_chunk;
		do{
			if(chunk_search->status != CHUNK_FREE)chunk_search = (AS_CHUNK *)chunk_search->next;
			else	if(chunk_search->size<size_malloc)chunk_search = (AS_CHUNK *)chunk_search->next;
				else	if(chunk_search->size == size_malloc){
					chunk_search->status = CHUNK_INUSE;
					as_info[bin_index].regions[region_num].current_chunk = chunk_search->next;
                    chunk_search->malloc_type = bin_index;//mark type
                    return (void *)(&chunk_search->mem_h);
					}
					else{
						current = (AS_CHUNK *)((int)(&chunk_search->mem_h) + size_malloc);
						current->next = chunk_search->next;
						current->previous = (void *)chunk_search;
						current->status = CHUNK_FREE;
						current->size = ((unsigned long)current->next - (unsigned long)(&current->mem_h));

                        //printf("current->next:%x current->previous:%x current->size:%x\n",current->next,current->previous,current->size);

						chunk_next = (AS_CHUNK *)chunk_search->next;
						chunk_next->previous = (void *)current;
						
						chunk_search->next = (void *)current;
						chunk_search->size = size_malloc;
						chunk_search->status = CHUNK_INUSE;

						as_info[bin_index].regions[region_num].current_chunk = current;
                        chunk_search->malloc_type = bin_index;//mark type
                        return (void *)(&chunk_search->mem_h);

					}
		}while(chunk_search!= (AS_CHUNK *)as_info[bin_index].regions[region_num].current_chunk);
	 	region_num++;
	}
	return NULL;
}


void  asfree(void* ptr)
{
	AS_CHUNK * chunk_free, * chunk_p, * chunk_n;
	
	chunk_free =(AS_CHUNK *)((int)ptr - INFO_SIZE);
	chunk_p = (AS_CHUNK *)chunk_free->previous;
	chunk_n = (AS_CHUNK *)chunk_free->next;

    //#ifdef CONFIG_MEM_SIZE
    //mem_size -= chunk_free->size;
    //printf("after asfree mem_size is 0x%x\n",mem_size);
    //#endif //CONFIG_MEM_SIZE
    #ifdef CONFIG_MEM_SIZE_C
    if (chunk_free->malloc_type == BIN_NORMAL) {
        mem_size -= chunk_free->size;
        alp_printf("after asfree,Cache Malloc memory size:0x%x\n",mem_size);
    }
    #endif //CONFIG_MEM_SIZE_C
    
    #ifdef CONFIG_MEM_SIZE_NC
    if (chunk_free->malloc_type == BIN_NONECACHED) {
        mem_size -= chunk_free->size;
        alp_printf("after asfree,Non-Cache Malloc memory size:0x%x\n",mem_size);
    }
    #endif //CONFIG_MEM_SIZE_NC

	if((chunk_p->status == CHUNK_FREE)||(chunk_n->status == CHUNK_FREE))
		{
		 if(chunk_n->status == CHUNK_FREE)
		 	{
		 	chunk_free->next = chunk_n->next;
			chunk_free->status = CHUNK_FREE;
			chunk_free->size = ((unsigned long)(&chunk_n->mem_h) - (unsigned long)(&chunk_free->mem_h) + (chunk_n->size));
		 }
		 if(chunk_p->status == CHUNK_FREE)
		 	{
		 	chunk_p->next = chunk_free->next;
			chunk_p->size = ((unsigned long)(&chunk_free->mem_h) - (unsigned long)(&chunk_p->mem_h) + (chunk_free->size));
		 }

	}else{
		chunk_free->status = CHUNK_FREE;
	}	
}

void* ascalloc(int n_elements, int element_size, int index)
{
	int element,i;
	void * ptr_calloc;
	uchar * ptr_set;
	element = n_elements*element_size;

	ptr_calloc = asmalloc(element,index);
	ptr_set = (uchar *)ptr_calloc;

	for(i=0;i<element;i++)
		{
			*(ptr_set++) = 0;
		}
	return ptr_calloc;
}

#ifdef CONFIG_C_MALLOC

int init_asmalloc_cache(void * mem, int size){
    return init_asmalloc(mem,size,BIN_NORMAL);
}

void * c_malloc(int size_m)
{
	return asmalloc(size_m, BIN_NORMAL);
}

void * c_calloc(int n_elements, int element_size)
{
	return ascalloc(n_elements, element_size, BIN_NORMAL);
}

void* c_realloc(void* ptr, int new_size)
{
	AS_CHUNK * p_old;
    char *p_copyed,* p_copy;
	void * p_new;

	p_old = (AS_CHUNK *)((size_t)ptr - INFO_SIZE);
	p_copy = (char *)ptr;
	
	if(new_size<p_old->size)return NULL;

	if((p_new = asmalloc(new_size,BIN_NORMAL))==NULL) return NULL;
	p_copyed = (char *)p_new;
	//printf("p_new:0x%x\n",p_new);
	while(p_copy !=p_old->next)
		*(p_copyed++) = *(p_copy++);

	asfree(ptr);
	return p_new;	
}

#endif //CONFIG_C_MALLOC

#ifdef CONFIG_NC_MALLOC

int init_asmalloc_nonecache(void * mem, int size){
    return init_asmalloc(mem,size,BIN_NONECACHED);
}

void * nc_malloc(int size_m)
{
    //printf("size_m:%d\n",size_m);
	return asmalloc(size_m,BIN_NONECACHED);
}

void * nc_calloc(int n_elements, int element_size)
{
	return ascalloc(n_elements, element_size,BIN_NONECACHED);
}


void* nc_realloc(void* ptr, int new_size)
{
	AS_CHUNK *p_old;
    char *p_copyed,* p_copy;
	void * p_new;

	p_old = (AS_CHUNK *)((size_t)ptr - INFO_SIZE);
	p_copy = (char *)ptr;        //can use memcopy();
	
	if(new_size < p_old->size)return NULL;

	if((p_new = asmalloc(new_size,BIN_NONECACHED))==NULL) return NULL;
	p_copyed = (char *)p_new;
	
	while(p_copy !=p_old->next)
		*(p_copyed++) = *(p_copy++);

	asfree(ptr);
	return p_new;	
}



int Test_malloc(){
        int i;
        void * ptr1,* ptr2,* ptr3,*ptr4,*ptr5;
        uchar *ptr;

/*********************************************/
        printf("======malloc test!======\n");
        ptr1=c_malloc(16);
        ptr=(uchar *)ptr1;
        printf("ptr1 malloc addr:%p\n",ptr1);
        for(i=0;i<16;i++)
        {
        	*ptr = i;
        	printf("%x  ",*(ptr++));
        }
        printf("\n");

/*********************************************/
        printf("======realloc test!======\n");
        ptr4=c_realloc(ptr1,32);
        printf("ptr4 malloc addr:%p\n",ptr4);
        //ptr1=c_malloc(2);
        //ptr1=c_calloc(2,4);
        ptr=(uchar *)ptr4;
        for(i=0;i<32;i++)
        {
        	printf("%x  ",*(ptr++));
        }
        printf("\n");
        asfree(ptr1);
        asfree(ptr4);

/*********************************************/
        printf("======free test!======\n");
        ptr2=nc_malloc(sizeof(uchar));
        printf("ptr2 malloc addr:%p\n",ptr2);
	    ptr3=nc_malloc(sizeof(AS_CHUNK));
        printf("ptr3 malloc addr:%p\n",ptr3);
        asfree(ptr2);
        ptr2 = nc_malloc(32);
        printf("after asfree, ptr2 malloc addr:%p\n",ptr2);
        asfree(ptr2);
        asfree(ptr3);

/*********************************************/
        printf("======calloc test!======\n");
        ptr5 = nc_calloc(2*7,8);
        printf("ptr5 calloc addr:%p\n",ptr5);
        ptr=(uchar *)ptr5;
         for(i=0;i<2*7*8;i++)
         {
             printf("%x ",*(ptr++));
         }
         printf("\n");
         asfree(ptr5);

	return 0;

}

#endif //CONFIG_NC_MALLOC

#endif //CONFIG_MALLOC

