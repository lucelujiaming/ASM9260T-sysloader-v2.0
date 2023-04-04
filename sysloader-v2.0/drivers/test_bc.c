#include <common.h>

#define BAT 0x8001c000
#define AUDIOOUT_CTRL 0x80048000
#define AUDIOOUT_PWRDN 0X80048070
#define REFCTRL 0X80048080
#define AUDIOIN_CTRL 0X8004C000

//#include "test_bc.h"

int test_bc(cmd_tbl_t *cmdtp,int argc,char* argv[]){

    if (argc!=2) {
        printf("Invalid args!\n");
        printf("Usage: %s\n",cmdtp->usage);
        return 0;
    }

    if(!strcmp(argv[1],"max"))
    {
        printf("test battery charge max\n");
        outl(0x00000000,AUDIOOUT_CTRL); //clkgate out
        outl(0x00000000,AUDIOIN_CTRL); //clkgate in
        outl(0x01001111,AUDIOOUT_PWRDN); //pwdn ref
        outl(0x00003770,REFCTRL); //ref ctrl     
        outl(0x00140504,BAT);
        return 0;
    }

    if(!strcmp(argv[1],"min")){
        printf("test battery charge min\n");
        outl(0x00000000,AUDIOOUT_CTRL); //clkgate out
        outl(0x00000000,AUDIOIN_CTRL); //clkgate in
        outl(0x01001111,AUDIOOUT_PWRDN); //pwdn ref
        outl(0x00003770,REFCTRL); //ref ctrl     
        outl(0x00004084,BAT);


        return 0;
    }

    if(!strcmp(argv[1],"stop")){
        printf("test battery charge stop\n");
        outl(0x00000000,AUDIOOUT_CTRL); //clkgate out
        outl(0x00000000,AUDIOIN_CTRL); //clkgate in
        outl(0x01001111,AUDIOOUT_PWRDN); //pwdn ref
        outl(0x00003770,REFCTRL); //ref ctrl     
        outl(0x00000084,BAT);


        return 0;
    }
    /*
          printf("test battery charge");
          outl(0x00000000,AUDIOOUT_CTRL); //clkgate out
          outl(0x00000000,AUDIOIN_CTRL); //clkgate in
          outl(0x01001111,AUDIOOUT_PWRDN); //pwdn ref
	      outl(0x00003770,REFCTRL); //ref ctrl     
          outl(0x001c0504,BAT);

         printf("battery charge test finished!\n");
*/
}
 

BOOT_CMD(testbc,test_bc,
         " for the test of battery charge\n testbc max: for charge with max current \n testbc min : for charge with min current\n testbc stop:stop charge\n",
         " for the test of battery charge\n testbc max: for charge with max current \n testbc min : for charge with min current\n testbc stop:stop charge\n"
                                         );




