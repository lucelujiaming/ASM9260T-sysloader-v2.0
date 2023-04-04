#include <common.h>
#include <drivers/rtc.h>

int rtc_reset()
{
	int status;
//	irqcounter = 0;
	outl(0xc0000000,CTRL+8);
	
	status = inl(STAT);
	
	while((status & 0x003f0000)!=0)
	{
		status = inl(STAT);
	}
	
	outl(0x000004be,PERSISTENT0);
	outl(0x00000093,CTRL+4);

//write the six analog shadow register:
	outl(0x00000040,PERSISTENT1);
	outl(0x00000060,PERSISTENT3);
	
	status = inl(STAT);
	
	while((status & 0x00003f00)!=0)
	{
		status = inl(STAT);
	}
	outl(0x00000005,SECONDS);
        outl(0x00000010,ALARM);
	outl(0x00000050,PERSISTENT2);
	status = inl(STAT);
	
	while((status & 0x00003f00)!=0)
	{
		status = inl(STAT);
	}

     outl(0x00000020,PERSISTENT0+8);  // clear 32k power down
	//outl(0x00000010,ALARM);
//	outl(0x00001001,CPUCLK);
//use 32768 crystal oscillator
	//outl(0x00000001,PERSISTENT0+4);
	
//	outl(0x00000002,WATCHDOG);
	
	
    outl(0x0,SECONDS); // clear time
    return 0;
}

int rtc_sec;


int do_set_time(cmd_tbl_t *cmdtp,int argc,char* argv[])
{
	int hour,minute,second;
	if (argc != 4) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;} 
	hour = TextToLong_TRL(argv[1]);
	minute = TextToLong_TRL(argv[2]);
	second = TextToLong_TRL(argv[3]);
	rtc_sec = 3600*hour+60*minute+second;
	outl(0x0,SECONDS);
	return 0;
}

int do_read_time(cmd_tbl_t *cmdtp,int argc,char* argv[])
{
	int hour,minute,second;
	int temp_sec;
	temp_sec = rtc_sec + inl(SECONDS);
	hour = temp_sec/3600;
	minute = (temp_sec - hour*3600)/60;
	second = temp_sec - hour*3600 - minute*60;
	printf("TIME: %d:%d:%d\n",hour,minute,second);
	return 0;	
}

int  rtc_clk_init(void){
    outl(0xc0000000,HW_RTC_BASE+8);
    alp_printf("rtc_clk_init\n");
    return 0;
}

__init_device(rtc_clk_init);

BOOT_CMD(rtc,rtc_reset,
         " #rtc",
         "rtc osc");
         
BOOT_CMD(set_time,do_set_time,
         " #set_time hh mm ss ",
         "set time to rtc");

BOOT_CMD(read_time,do_read_time,
         " #read_time",
         "read time in rtc");
