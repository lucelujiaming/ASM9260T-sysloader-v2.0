#ifndef __RTC_H__
#define __RTC_H__

#define CTRL        	0x8005C000
#define STAT        	0x8005C010
#define MILLISECONDS	0x8005C020
#define SECONDS     	0x8005C030
#define ALARM       	0x8005C040
#define WATCHDOG    	0x8005C050
#define PERSISTENT0 	0x8005C060
#define PERSISTENT1 	0x8005C070
#define PERSISTENT2 	0x8005C080
#define PERSISTENT3 	0x8005C090
#define UNLOCK      	0x8005C200
#define LASERFUSE0  	0x8005C300
#define LASERFUSE1  	0x8005C310
#define LASERFUSE2  	0x8005C320
#define LASERFUSE3  	0x8005C330
#define LASERFUSE4  	0x8005C340
#define LASERFUSE5  	0x8005C350
#define LASERFUSE6  	0x8005C360
#define LASERFUSE7  	0x8005C370
#define LASERFUSE8  	0x8005C380
#define LASERFUSE9  	0x8005C390
#define LASERFUSE10 	0x8005C3A0
#define LASERFUSE11 	0x8005C3B0
#define DEBUG       	0x8005C0A0
#define CPUCLK          0x80040020

int rtc_reset(void);
int do_set_time(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int do_read_time(cmd_tbl_t *cmdtp,int argc,char* argv[]);
int  rtc_clk_init(void);

#endif //__RTC_H__


