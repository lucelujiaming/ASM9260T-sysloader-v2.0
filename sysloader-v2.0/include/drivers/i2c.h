/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

AS3310 Boot Loader Flash Header file

------------------- Version 1.0  ----------------------
i2c program standardlized
 Zhang Bo 2007-03-22

*/

#ifndef __I2C_H__
#define __I2c_H__

void RX_DMA_START_I2C();
void TX_DMA_START_I2C();
void init_i2c();
int eeprom_write(ulong addr,ulong * buffer,int n_bytes);
#endif

