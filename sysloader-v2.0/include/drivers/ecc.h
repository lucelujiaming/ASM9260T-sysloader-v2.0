

/*
Alpha Scale AS3310X Booter, H-BOOT
Zhao Haiyuan, AlpScale Software Engineering, zhaoy@alpscale.com.cn
 
------------------- Version 1.0  ----------------------
Create File, 
    Support Nand ECC 
    Zhao Haiyuan 2007-04-29

*/


#include <common.h>


#define ECC_MAX_PACKAGE     7
#define HW_HWECC_CTRL       0x80008000

#define ECC_NOERROR         0
#define ECC_CORRECTED       1
#define ECC_ALLZEROES       2
#define ECC_UNCORRECTABLE   -1
#define ECC_ALLONES         -2
#define MAX_DMA_WAIT        0x100000

extern volatile AS3310_DMA_PKG NCached ecc_pkg_table[ECC_MAX_PACKAGE];
extern int NCached error_code;
extern volatile int NCached Report[10];

void Ecc_Init(void);

/*
Ecc Encode. Encode the data from the address "datafrom" with the byte length of "Ecclth" eg 512.
Write the status code to "parityadd" and write the parity code to "parityadd+1" 
*/
void EccEncode(char * datafrom, char* parityadd);

/*
Ecc Decode
decode the data from the address "datafrom" 
with the byte length of "Ecclth" eg 512.
based on the party code from parityadd
write the EccReport to "Report" 9*32 bit
*/
int EccDecodeCheck( char* datafrom,char* parityadd);    
//int do_EccEn(cmd_tbl_t *cmdtp,int argc, char *argv[]);
//int do_EccDeCh(cmd_tbl_t *cmdtp,int argc, char *argv[]);    
