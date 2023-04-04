/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

Sreen Text Header File

*/
#include <common.h>

#ifndef __WORD_H__
#define __WORD_H__

#define WORD_WIDTH 32
#define WORD_HIGHT 32


typedef struct  _HzDotStruct_{
   u8 Index[2];  
   u8 HzDot[128]; 
} __attribute__((packed)) HzDotStruct;

HzDotStruct * got_Hzlib_ptr(int index);
extern HzDotStruct HzLib_temp;

//#define Section_TEXTLIB  __attribute__ ((section (".textlib")))

#endif // __WORD_H__
