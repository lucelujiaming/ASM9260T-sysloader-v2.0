
 /*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

Pin Control Source file
 
Change log: 

------------------- Version 1.0  ----------------------
He Yong 2006-04-14
    -- Create file

*/
#include <common.h>
/*

 ================  Basic Pin IO Funtions  ================

*/

    
/* 
user can not call set_pin_mux() directly it's unsafe and forbiddened
call request_as3310_gpio() instead !!
*/
void set_pin_mux(int port,int pin,int mux_type){
{
    u32 val,addr;

    addr = (u32)(HW_IOCON_PIO_BASE + (port*32) + (pin*4));
    val = inl( addr );   // read org val

    val &= 0xFFFFFFF8; // clear mux feild
    val |= mux_type; // set mux feild with new value

    outl( val ,addr );   // Set new value
}



