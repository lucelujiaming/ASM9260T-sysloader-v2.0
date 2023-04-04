编译说明
一、根据芯片和引导的系统配置代码
	1、ASM9260T（SDRAM封装在芯片内部）中运行；引导Linux，
	需要在/sysloader-v2.0/loader4ksram/kernel/main.c中做如下修改：
	#define ASM9260_SDRAM_SIP
	//#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	//#undef	ASM9260_LOAD_LINUX


	2、ASM9260T（SDRAM封装在芯片内部）中运行；引导Alpos，
	需要在/sysloader-v2.0/loader4ksram/kernel/main.c中做如下修改：
	#define ASM9260_SDRAM_SIP
	//#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	#undef	ASM9260_LOAD_LINUX


	3、ASM1926T（SDRAM外挂在芯片外部）中运行；引导Linux，
	需要在/sysloader-v2.0/loader4ksram/kernel/main.c中做如下修改：
	#define ASM9260_SDRAM_SIP
	#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	//#undef	ASM9260_LOAD_LINUX


	4、ASM1926T（SDRAM外挂在芯片外部）中运行；引导Alpos，
	需要在/sysloader-v2.0/loader4ksram/kernel/main.c中做如下修改：
	#define ASM9260_SDRAM_SIP
	#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	#undef	ASM9260_LOAD_LINUX

二、/sysloader-v2.0目录下执行mk文件：./mk
