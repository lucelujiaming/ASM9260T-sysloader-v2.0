#ifndef __I2S_H__
#define __I2S_H__

/*	I2S  Registers		*/
#define AS3310_I2S_BASE		0x80056000

#define I2S_CTRL_SET        0x80056004
#define I2s_CTRL_CLR        0x80056008

#define I2S_PKG    0x40000200  

//should replaced by malloc**********************
#define I2SCFG	(AS3310_I2S_BASE + 0x20)  //W/R	0x0	Configuration register
#define I2SSPCR	(AS3310_I2S_BASE + 0x10)		//W	0x0	Protocol control register
#define I2SSPCR_SET  I2SSPCR + 0x4
#define I2SEVENT  (AS3310_I2S_BASE + 0x50)  	//W/R	0x1203	Event flag register
#define I2SSTAT   (AS3310_I2S_BASE + 0x40) //R	0x0	Status register


#define DMAX_CH2_CURCMDAR   0x80024110               //Channel 2 for I2s
#define DMAX_CH2_NXTCMDAR   0x80024120
#define DMAX_CH2_CMDAR      0x80024130
#define DMAX_CH2_BAR        0x80024140
#define DMAX_CH2_SEMA       0x80024150

#endif


