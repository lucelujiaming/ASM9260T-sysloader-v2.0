����˵��
һ������оƬ��������ϵͳ���ô���
	1��ASM9260T��SDRAM��װ��оƬ�ڲ��������У�����Linux��
	��Ҫ��/sysloader-v2.0/loader4ksram/kernel/main.c���������޸ģ�
	#define ASM9260_SDRAM_SIP
	//#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	//#undef	ASM9260_LOAD_LINUX


	2��ASM9260T��SDRAM��װ��оƬ�ڲ��������У�����Alpos��
	��Ҫ��/sysloader-v2.0/loader4ksram/kernel/main.c���������޸ģ�
	#define ASM9260_SDRAM_SIP
	//#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	#undef	ASM9260_LOAD_LINUX


	3��ASM1926T��SDRAM�����оƬ�ⲿ�������У�����Linux��
	��Ҫ��/sysloader-v2.0/loader4ksram/kernel/main.c���������޸ģ�
	#define ASM9260_SDRAM_SIP
	#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	//#undef	ASM9260_LOAD_LINUX


	4��ASM1926T��SDRAM�����оƬ�ⲿ�������У�����Alpos��
	��Ҫ��/sysloader-v2.0/loader4ksram/kernel/main.c���������޸ģ�
	#define ASM9260_SDRAM_SIP
	#undef	ASM9260_SDRAM_SIP

	#define	ASM9260_LOAD_LINUX
	#undef	ASM9260_LOAD_LINUX

����/sysloader-v2.0Ŀ¼��ִ��mk�ļ���./mk
