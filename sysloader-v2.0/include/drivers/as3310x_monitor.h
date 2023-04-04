#ifndef __AS3310X_MONITOR_H__
#define __AS3310X_MONITOR_H__

/****************************** register ********************************/
//battary charge reg
#define HW_POWER_BATTCHRG 0x8001c320
#define HW_POWER_BATTCHRG_SET HW_POWER_BATTCHRG + 0x4
#define HW_POWER_BATTCHRG_CLR HW_POWER_BATTCHRG + 0x8
#define HW_POWER_BATTCHRG_TOG HW_POWER_BATTCHRG + 0xc
/* lradc */
// lradc base addr
#define HW_LRADC_BASE       0x80050000
                            
/*                          
 * Register offsets         
 */                         
#define HW_LRADC_CTRL0   HW_LRADC_BASE//0x8005000
#define HW_LRADC_CTRL0_SET HW_LRADC_CTRL0 + 0x4
#define HW_LRADC_CTRL0_CLR HW_LRADC_CTRL0 + 0x8
#define HW_LRADC_CTRL0_TOG HW_LRADC_CTRL0 + 0xc

#define HW_LRADC_CTRL1   (HW_LRADC_BASE+0x10)//0x8005010
#define HW_LRADC_CTRL1_SET HW_LRADC_CTRL1 + 0x4                                           // 
#define HW_LRADC_CTRL1_CLR HW_LRADC_CTRL1 + 0x8                                           // 
#define HW_LRADC_CTRL1_TOG HW_LRADC_CTRL1 + 0xc   
                                        // 
#define HW_LRADC_CTRL2   (HW_LRADC_BASE+0x20)//0x8005020
#define HW_LRADC_CTRL2_SET HW_LRADC_CTRL2 + 0x4                                              
#define HW_LRADC_CTRL2_CLR HW_LRADC_CTRL2 + 0x8 
#define HW_LRADC_CTRL2_TOG HW_LRADC_CTRL2 + 0xc 
                                             
#define HW_LRADC_CTRL3   (HW_LRADC_BASE+0x30)//0x8005030
#define HW_LRADC_CTRL3_SET HW_LRADC_CTRL3 + 0x4                                              
#define HW_LRADC_CTRL3_CLR HW_LRADC_CTRL3 + 0x8 
#define HW_LRADC_CTRL3_TOG HW_LRADC_CTRL3 + 0xc                          
/*                        
 * Status register bits.  
 */                       
#define HW_LRADC_STATUS  HW_LRADC_BASE+0x40//0x8005040
                         
/*                       
 * Result register bits  
 */                      
#define HW_LRADC_CH0     (HW_LRADC_BASE+0x50)//0x80050050
#define HW_LRADC_CH1     (HW_LRADC_BASE+0x60)//0x80050060
#define HW_LRADC_CH2     (HW_LRADC_BASE+0x70)//0x80050070
#define HW_LRADC_CH3     (HW_LRADC_BASE+0x80)//0x80050080
#define HW_LRADC_CH4     (HW_LRADC_BASE+0x90)//0x80050090
#define HW_LRADC_CH5     (HW_LRADC_BASE+0xA0)//0x800500A0
#define HW_LRADC_CH6     (HW_LRADC_BASE+0xB0)//0x800500B0
#define HW_LRADC_CH7     (HW_LRADC_BASE+0xC0)//0x800500C0
                         
#define HW_LRADC_CH7_SET HW_LRADC_CH7 + 0x4
#define HW_LRADC_CH7_CLR HW_LRADC_CH7 + 0x8
#define HW_LRADC_CH7_TOG HW_LRADC_CH7 + 0xc
/*                       
 * Delay register bits   
 */                      
#define HW_LRADC_DELAY0  (HW_LRADC_BASE+0xD0)//0x800500D0
#define HW_LRADC_DELAY1  (HW_LRADC_BASE+0xE0)//0x800500E0
#define HW_LRADC_DELAY2  (HW_LRADC_BASE+0xF0)//0x800500F0
#define HW_LRADC_DELAY3  (HW_LRADC_BASE+0x100)//0x80050100
                         
/*                          
 * Debug register bits      
 */                         
#define HW_LRADC_DEBUG0     (HW_LRADC_BASE+0x110)//0x80050110
#define HW_LRADC_DEBUG1     (HW_LRADC_BASE+0x120)//0x80050120
#define HW_LRADC_CONVERSION (HW_LRADC_BASE+0x130)//0x80050130

/* length of lradc register*/
#define HW_LRADC_LEN        0x140

/*************************************************************************/


#define VOLUME_UP 1
#define VOLUME_DOWN 0

#define MO_GET_BATTRY_VAL       0x55ff
#define MO_SET_CHAR_PWD_DOWN    0X55FE
#define MO_SET_CHAR_PWD_UP      0X55FD
#define MO_GET_CHARGING         0X55FC
#define MO_SET_BATTRY_VAL       0X55FB
#define MO_GET_CHARGE_CURRENT   0X55FA
#define MO_SET_PWRDN            0X55F9
#define MO_SET_VOL_CHANGE       0X55F8
#define MO_SET_BACKLIGHT        0X55F7
#define MO_GET_AO_STATUS        0X55F6
#define MO_GET_VOL              0X55F5
#define MO_SET_CLOCK_SW         0X55F4
#define MO_GET_MMC_STATUS       0X55F3

#define MONITOR_TIME 100
#define V_LOW 3500
#define V_HIGH 4200
#define V_THRESHOLD1 3700
#define V_THRESHOLD2 4100
//#define PRINT_V

/****************************************************************/

struct batt_info{
    unsigned char num_samples;
    int value;
};

int vol_change(int ud);
int audioout_hw_init_det(void);
void clk_switch(int cpu,int hclk_per_cpu,int emi,int gpmi_per_cpu,int xclk_per_24);
static struct batt_info get_batt_val(void);
int calcu_ba(struct batt_info b);
void checkv(int vol);
int get_vol(void);
void keep_sys_open(void);

#endif
