#ifndef CACHE_OP_H__
#define CACHE_OP_H__


void IvalidDCache();
void cleanDCache();
void IvalidICache();
void cleanIvalidDCacheLineByVA(void *pva);
void cleanDCacheLineByVA(void *pva);
void cleanDCacheRegion(void *pva, unsigned int size);
void IvalidDCacheRegion(void *pva, unsigned int size);

#endif

