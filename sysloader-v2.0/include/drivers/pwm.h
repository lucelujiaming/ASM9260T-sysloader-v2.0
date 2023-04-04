
/*

PWM HEADER file
 
Change log: 

------------------- Version 1.0  ----------------------
zhangbo 2007-10-29
    -- Create file

*/

#ifndef _PWM_H_
#define _PWM_H_

#define INACTIVE_STATE_HIGE 3
#define INACTIVE_STATE_LOW  2
#define ACTIVE_STATE_HIGE  3
#define ACTIVE_STATE_LOW   2

#define PWM_BACKLIGHT   2
#define BL_UP           1
#define BL_DOWN         0

#define BACK_LIGHT_SET       0xaabb

/*
 * ----------------------------------------------------------------------------
 * PWM
 * ----------------------------------------------------------------------------
 */ 
#define HW_PWM_CTRL 0x80064000
#define HW_PWM_CTRL_SET HW_PWM_CTRL + 0x4
#define HW_PWM_CTRL_CLR HW_PWM_CTRL + 0x8
#define HW_PWM_CTRL_TOG HW_PWM_CTRL + 0xc

#define HW_PWM_ACTIVE(x) HW_PWM_CTRL + 0x10 + 0x20*x
#define HW_PWM_PERIOD(x) HW_PWM_CTRL + 0x20 + 0x20*x

#define HW_PWM_ACTIVE_SET(x) HW_PWM_ACTIVE(x) + 0x4
#define HW_PWM_ACTIVE_CLR(x) HW_PWM_ACTIVE(x) + 0x8
#define HW_PWM_ACTIVE_TOG(x) HW_PWM_ACTIVE(x) + 0xc

#define HW_PWM_PERIOD_SET(x) HW_PWM_PERIOD(x) + 0x4
#define HW_PWM_PERIOD_CLR(x) HW_PWM_PERIOD(x) + 0x8
#define HW_PWM_PERIOD_TOG(x) HW_PWM_PERIOD(x) + 0xc


void percent_demo(int a,int b);
int change_one_step(int num,int ud);
char calcu_freq(int p,int h);
int active_pwm(int num,int period,int high);
/************period & high should be the number of ns  **************/
int as3310_pwm_init(struct device *dev);
int pwm_ioctl(struct device *dev,unsigned int cmd, unsigned long arg);



#endif // _PWM_H_
       // 
