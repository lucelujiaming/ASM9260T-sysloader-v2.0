
/*

pwm Source file
 
Change log: 

------------------- Version 1.0  ----------------------
zhangbo 2007-10-29
    -- Create file
zhangbo 2008-1-07
    -- transplant to nuwastone
*/
#include <common.h>
#include <drivers/pwm.h>

//spinlock_t as3310_pwm_req_lock;
int Section_TEXTLIB Ns_of_period_div[8] = {42,83,167,333,667,2667,10667,42667}; //ns
int Section_TEXTLIB pwm_percent[6] = {0,20,40,60,80,100}; //percent*100
static int Section_TEXTLIB bl_level = 5;
int mod_of_period[8];

void percent_demo(int a,int b){
    int vi,vf;
    vi = a*100/b;
    vf = a*10000/b-vi*100;

    if (vf > 10) {
        printf("percentage is %d.%d%\n",vi,vf);
    }else printf("percentage is %d.0%d%\n",vi,vf);

}
char calcu_freq(int p,int h){
    int retval;
    unsigned char sel,i,temp;

    temp =0;
    while((p/Ns_of_period_div[temp]) > 65536)temp++;
    //printf("temp:%d\n",temp);
    for (i=temp;i<8;i++) {
        mod_of_period[i] = h % Ns_of_period_div[i];
    }
    sel = temp;
    retval = mod_of_period[temp];
    for (i=temp;i<8;i++) {
        if(mod_of_period[i] <= retval){
            sel = i;
            retval = mod_of_period[i];
        }
    }
    return sel;
}
int change_one_step(int num,int ud){
    int reg_act,reg_prd;
    int new_high,one_percent,high,period;
    reg_act = inl(HW_PWM_ACTIVE(num));
    reg_prd = inl(HW_PWM_PERIOD(num));
    period = (reg_prd & 0xffff);
    //high = (reg_act >> 16);
    //one_percent = high/3;
    //if (!one_percent)one_percent = 1; 

    if(ud){
        if (bl_level == 5){
            new_high = period;
        }else {
            bl_level++;
            new_high = period*pwm_percent[bl_level]/100;
        }
    }else{
        if (bl_level == 0){
            new_high =0;
        }else{
            bl_level--;
            new_high = period*pwm_percent[bl_level]/100;
        }
    }
    alp_printf("back light ");
    percent_demo(new_high,period);
    outl((new_high << 16),HW_PWM_ACTIVE(num));
    outl(reg_prd,HW_PWM_PERIOD(num));

    return bl_level;
}

int active_pwm(int num,int period,int high){//period & high should be the number of ns
    int retval;
    unsigned char which_div;
    int n_active,n_inactive;
    int va,vp;

    //printf("HW_PWM_CTRL:0x%p\n",inl(HW_PWM_CTRL));
    if(!(inl(HW_PWM_CTRL)&(1<<(25+num)))){
        printf("pwm active Error: pwm module %d isn't present in this product\n",num);
        goto _pwm_panic_;
    }
    retval = request_as3310_gpio(GPIO_PWM_PORT,(GPIO_PWM_BASE_PIN + num),PIN_FUNCTION_0);

    outl((1<<num),HW_PWM_CTRL_SET);

    which_div = calcu_freq(period,high);
    n_active = 0;
    n_inactive = high / Ns_of_period_div[which_div];
    period = period / Ns_of_period_div[which_div];
    va = ((n_inactive << 16) | n_active);
    vp = ((period-1) | (ACTIVE_STATE_HIGE<<16) | (INACTIVE_STATE_LOW<<18) | (which_div<<20));

    //printf("va:0x%p\n",va);
    //printf("vp:0x%p\n",vp);
    outl(va,HW_PWM_ACTIVE(num));
    outl(vp,HW_PWM_PERIOD(num));

    return 0;
_pwm_panic_:
    panic("Invalid Pwm Active\n");
}

int as3310_pwm_init(struct device *dev){
    printf("pwm inited\n");
    outl(0x80000000,HW_PWM_CTRL_CLR);
    outl(0x40000000,HW_PWM_CTRL_CLR);
    active_pwm(2,10000000,5000000); //backlight init

    return 0;
}

int pwm_ioctl(struct device *dev,unsigned int cmd, unsigned long arg)
{
    int val;
    switch (cmd) {
    case BACK_LIGHT_SET:
        return change_one_step(PWM_BACKLIGHT,*(int *)arg);
    default:
		printf("as3310_monitor: unimplemented ioctl=0x%x\n",
		       cmd);
		return -EINVAL;
    }
}

struct device dev_pwm = {
    .name       = "pwm",
    .dev_id     = 0,
    .probe      = as3310_pwm_init,
    .remove     = NULL,  
    .open       = NULL,  
    .close      = NULL,  
    .read       = NULL,  
    .write      = NULL, 
    .ioctl      = pwm_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = NULL,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device(dev_pwm);


