/* linux/drivers/char/as3310_monitor.c*/
/*you should create nod c 254 0 first manually*/

#include <common.h>
#include <drivers/as3310x_monitor.h>
#include <drivers/sound.h>


/**==============================================================*/
static struct timer_list monitor_timer;
static char charging,thres1,thres2;
static int audio_out;

static int battary_v;

void clk_switch(int cpu,int hclk_per_cpu,int emi,int gpmi_per_cpu,int xclk_per_24){
    AS3310_CLK * clk;
    int pll;

    clk = AS3310_GetBase_CLK();
    clk->PLLCTRL0[2] = 0x00008000;

    clk->HBUSCLKCTRL[0] = hclk_per_cpu;//HCLK devided by x from CPU CLK
    clk->XBUSCLKCTRL[0] = xclk_per_24;//XCLK devided by x from CRYSTAL 24M
    clk->EMICLKCTRL[0] = emi;// EMI devided by x from HCLK

    switch (cpu) {
    case 160:
        clk->CPUCLKCTRL[0] = 0x3;//480M / 3
        pll = 480;
        break;
    case 120:
        clk->CPUCLKCTRL[0] = 0x4;//480M / 4
        pll = 480;
        break;
    case 180:
        clk->CPUCLKCTRL[0] = 0x2;//360M / 2
        pll = 360;
        break;
    case 90:
        clk->CPUCLKCTRL[0] = 0x4;//90M
        pll = 360;
        break;
    case 60:
        clk->CPUCLKCTRL[0] = 0x6;//60M
        pll = 360;
        break;
    default: 
        cpu = 24;
        break;
    }

    clk->SSPCLKCTRL[0] = 2*clk->CPUCLKCTRL[0];// SSP devided by 2 from CPU CLK                                                   
    clk->GPMICLKCTRL[0] = gpmi_per_cpu*clk->CPUCLKCTRL[0];;// GPMI devided by x from CPU CLK                                                   

    if (pll==480) {
        clk->PLLCTRL0[0] = 0x000701e0;              // 480MHz
        clk->PLLCTRL1[1] = 0x20000000;              // divide by 4        
        while ((clk->PLLCTRL1[0] & (1<<31)) ==0);   // wait
        clk->UTMICLKCTRL[0] = 0x0;              // enable 30m 120m clk        
        clk->PLLCTRL0[2] = 0x00028000;
    }
    else{
        clk->PLLCTRL0[0] = 0x00070168;              // 360MHz
        while ((clk->PLLCTRL1[0] & (1<<31)) ==0);   // wait
        clk->UTMICLKCTRL[0] = 0xc0000000;              // disable 30m 120m clk        
        clk->PLLCTRL0[2] = 0x00028000;
    }
}

static struct batt_info get_batt_val(void){
    int get_val;
    struct batt_info batt;
    outl(0x00000080,HW_LRADC_CTRL0_SET);//start lradc conversion once
    get_val = inl(HW_LRADC_CH7);
    batt.value = get_val & 0x3ffff;
    batt.num_samples = (get_val & 0x1f000000)>>24;
    //printf("value:%d\n",batt.value);
    return batt;    
}

int calcu_ba(struct batt_info b){
    int voltage;
    int v;
    if (b.num_samples != 1) {
        printf("the sample bit of lradc7(batt) reg isn't 1, you should check if LOOP_COUNT bit is set\n");
    }
    if((inl(HW_LRADC_CTRL2)) >> 31)voltage = b.value * 2 * 2;//check if divide bit set
    else voltage = b.value * 2;

    v = voltage * 1000 /(1<<11);
    v= (v*9426+248193)/10000;//polyfit by matlab
    
#ifdef PRINT_V
 //print to float type
    int vint,vfl;
    vint = v/1000;
    vfl = v%1000;

    if (vfl >= 100) {
        printf("current voltage is:%d.%dv\n",vint,vfl);
    }else if (vfl >= 10) {
        printf("current voltage is:%d.0%dv\n",vint,vfl);
        }
    else printf("current voltage is:%d.00%dv\n",vint,vfl);
#endif
    return v;


}

static int normal_v(int v){
    return (v-3500);

}
static int set_current(int v){
    int val;
    int cur;

    //if ((v != 400)&&(v != 200)&& (v != 100)&&(v != 50)&&(v != 20)&&(v != 10)) return -EINVAL;

    val = inl(HW_POWER_BATTCHRG); //read current bit value
    cur = val & 0x00003f00;
    outl(cur,HW_POWER_BATTCHRG_CLR);//clear existing current bit
    cur = 0;
    if (v >= 400) {
        cur |= 0x00002000;
        v -= 400;}
    if (v >= 200) {
        cur |= 0x00001000; 
        v -= 200;}
    if (v >= 100) {
        cur |= 0x00000800;
        v -= 100;}
    if (v >= 50)  {
        cur |= 0x00000400;
        v -= 50;}
    if (v >= 20)  {
        cur |= 0x00000200;
        v -= 20;}
    if (v >= 10)  {
        cur |= 0x00000100;
        v -= 10;}

    outl(cur,HW_POWER_BATTCHRG_SET);
    return cur;
}

static int calcu_current(void){
    int v;
    int cur;

    cur = 0;
    v = inl(HW_POWER_BATTCHRG); //read current bit value
    //printf("HW_POWER_BATTCHRG:0x%p\n",v);

    if (v & 0x00002000){
        cur += 400;}
    if (v & 0x00001000){
        cur += 200;}
    if (v & 0x00000800){
        cur += 100;}
    if (v & 0x00000400){
        cur += 50;}
    if (v & 0x00000200){
        cur += 20;}
    if (v & 0x00000100){
        cur += 10;}
    printf("cur:0x%d\n",cur);

    return cur;
}

void checkv(int vol){
    //printf("voltage value checked:%d\n",vol);
    if ((vol < V_LOW)&&(charging == 0)) {
        charging = 1;
        outl(0x00000020,HW_POWER_BATTCHRG_SET);    //start charge
        outl(0x00400000,HW_POWER_BATTCHRG_CLR);
        //set_current(780);
        printf("start charge\n");
    }
    if ((vol > V_HIGH)&&(charging == 1)) {
        charging =0;
        outl(0x00000020,HW_POWER_BATTCHRG_CLR);    //clear bit charg_pwd_battchrg;
        outl(0x00400000,HW_POWER_BATTCHRG_SET);    //set bit en_battchrg;
        printf("stop charging\n");
    }
/*
    if (charging) {
        if ((thres1 == 0) && (vol > V_THRESHOLD1)) {
            set_current(380);
            thres1 = 1;
            printf("change charging current to 380mA\n");
        }
        if ((thres2 == 0) && (vol > V_THRESHOLD2)) {
            set_current(10);
            thres2 =1;
            printf("change charging current to 10mA\n");
        }
    }else {
        thres1 = 0;
        thres2 = 0;
    }
*/
}

static int set_sample_val(int v){
    int val;
    val = (v << 24);
    outl(val,HW_LRADC_CH7_SET);
    //printf("HW_LRADC_CH7:0x%p\n",inl(HW_LRADC_CH7));
    return 0;

}



static int as3310x_monitor_open()
{
    printf("as3310x_monitor driver opened\n");
    return 0;
}

static int as3310x_monitor_release(struct device *dev)
{
    printf("as3310x_monitor driver closed\n");
	return 0;
}
/*
static ssize_t as3310x_monitor_read(struct file *file, char *buf, size_t count,
			       loff_t * ppos)
{
	return 0;
}
static ssize_t as3310x_monitor_write(struct file *file, const char *buf, size_t count,
			       loff_t * ppos)
{
	return 0;
}
*/

int vol_change(int ud){
    int cur,temp;
    if (audio_out) {
        cur = inl(AUDIOOUT_HPVOL);
        temp = cur & 0x1f;
        if (ud) {
            if (temp == 0) {
                printf("volume max\n");
            }else cur -=  0x101;
        }else{
              if (temp == 0x1f) {
                  printf("volume min\n");
              }else cur += 0x101;
        }
        outl(cur,AUDIOOUT_HPVOL);
        temp = cur & 0x1f;
        printf("volume now ");
        //percent_demo((0x1f-temp),0x1f);
        return ((0x1f-temp)/2);
    }else {
        cur = inl(AUDIOOUT_SPKRVOL);
        temp = cur & 0xf;
        if (ud) {
            if (temp == 0) {
                printf("volume max\n");
            }else cur -= 0x1;
        }else{
              if (temp == 0xf) {
                  printf("volume min\n");
              }else cur += 0x1;
        }
        outl(cur,AUDIOOUT_SPKRVOL);
        temp = cur & 0xf;
        printf("volume now ");
        //percent_demo((0xf-temp),0xf);
        return (0xf-temp);
    }
}

int get_vol(void){
    int cur,temp;
    if (audio_out) {
        cur = inl(AUDIOOUT_HPVOL);
        temp = cur & 0x1f;
        printf("volume now ");
        //percent_demo((0x1f-temp),0x1f);
        return ((0x1f-temp)/2);
    }else {
        cur = inl(AUDIOOUT_SPKRVOL);
        temp = cur & 0xf;
        printf("volume now ");
        //percent_demo((0xf-temp),0xf);
        return (0xf-temp);
    }
}

//#ifndef CONFIG_BOARD_AS3310_DEV
static void headphone_vs_speaker(void){
    int headio;
    headio = read_GPIO(GPIO_HP_DET_PORT,GPIO_HP_DET_PIN);
    //printf("headio:%d\n\n",headio);
    if (headio != audio_out) {
        audio_out = headio;
        if (headio) {
            clear_GPIO(GPIO_SPK_PORT,GPIO_SPK_PIN);//hw close speaker
            printf("headphone opened\n");
            outl(0x00001001,AUDIOOUT_PWRDN_CLR);//power on headphone
            outl(0x01000000,AUDIOOUT_PWRDN_SET);//power down speaker
            outl(0x00010000,AUDIOOUT_HPVOL_CLR);//demute headphone
            outl(0x00010000,AUDIOOUT_SPKRVOL_SET);//mute speaker
        }else{
            set_GPIO(GPIO_SPK_PORT,GPIO_SPK_PIN);//hw open speaker
            printf("speaker opened\n");
            outl(0x01001000,AUDIOOUT_PWRDN_CLR);
            outl(0x00000001,AUDIOOUT_PWRDN_SET);
            outl(0x00010000,AUDIOOUT_SPKRVOL_CLR);
            outl(0x00010000,AUDIOOUT_HPVOL_SET);

        }
    }
}
static int power_down(void){
    int v;
    clear_GPIO(GPIO_PWRON_PORT,GPIO_PWRON_PIN);
    v = read_GPIO(GPIO_PWRON_PORT,GPIO_PWRON_PIN);
    return v;
}
static void audioout_pin_init(void){
    int headio;
    //request_as3310_gpio(GPIO_HP_DET_PORT,GPIO_HP_DET_PIN,PIN_FUNCTION_3);
    //request_as3310_gpio(GPIO_SPK_PORT,GPIO_SPK_PORT,PIN_FUNCTION_3);
    headio = read_GPIO(GPIO_HP_DET_PORT,GPIO_HP_DET_PIN);
    audio_out = headio;
}
//#endif// CONFIG_BOARD_AS3310_DEV

static void as3310_monitor(ulong data){
    struct batt_info battary_info;

    //printf("as3310_monitor...\n");
    battary_info = get_batt_val();
    battary_v = calcu_ba(battary_info);
    checkv(battary_v);
    //printf("as3310_monitor...\n");
#ifdef CONFIG_BOARD_AS3310_MP4_DEMO
    headphone_vs_speaker();
#endif// CONFIG_BOARD_AS3310_MP4_DEMO

    monitor_timer.expires = jiffies + MONITOR_TIME ;//- (battary_v/10);
    add_timer(&monitor_timer);
    
}

static void as3310_monitor_init_timer(void){

    //init_timer(&monitor_timer);
    monitor_timer.function = as3310_monitor;
    monitor_timer.expires = jiffies + MONITOR_TIME;
    monitor_timer.next = monitor_timer.prev = NULL;
    monitor_timer.handle_later = 0;
    add_timer(&monitor_timer);
}

static void  lradc_batt_init(void){
    outl(0xc0000000,HW_LRADC_CTRL0_CLR);
    outl(0x80000000,HW_LRADC_CTRL2_SET);//lradc7 value divided by 2
    outl(0x00a00000,HW_LRADC_CTRL3_SET);
    outl(0x00800000,HW_LRADC_CTRL1_SET);//enable an irq

    set_sample_val(1);//temprary...............
    
    outl(0x00000080,HW_LRADC_CTRL0_SET);
}
void keep_sys_open(void){
    request_as3310_gpio(GPIO_PWRON_PORT,GPIO_PWRON_PIN,PIN_FUNCTION_3);
    set_GPIO(GPIO_PWRON_PORT,GPIO_PWRON_PIN);
}

static int as3310_get_detect_value(void)
{
    return inl(PIN_CTRL_DIN_0)&(0x1<<25);
}


static int as3310x_monitor_ioctl(struct device *dev,
       unsigned int cmd, unsigned long arg)
{
    int val;
    //struct batt_info batt_i;
    switch (cmd) {
    //printf("as3310_monitor ioctl is:0x%x\n",cmd);
    case MO_GET_BATTRY_VAL:
        val = normal_v(battary_v);
        return val;
    case MO_SET_BATTRY_VAL:
        set_sample_val(*(int *)arg);//lradc 7 result status.
        return 0;
    case MO_SET_CHAR_PWD_DOWN:
        outl(0x00000200,HW_POWER_BATTCHRG_CLR);    //clear bit charg_pwd_battchrg;
        outl(0x00400000,HW_POWER_BATTCHRG_SET);    //set bit en_battchrg;
        return 0;
    case MO_SET_CHAR_PWD_UP:
        outl(0x00000200,HW_POWER_BATTCHRG_SET);
        outl(0x00400000,HW_POWER_BATTCHRG_CLR);
        return 0;
	case MO_GET_CHARGING:
		return val;
    case MO_GET_CHARGE_CURRENT:
        val = calcu_current();
        return val;
    case MO_SET_PWRDN:
        while(val = power_down());
        return val;//1:sys on 0:sys off
    case MO_SET_VOL_CHANGE:
        val = vol_change(*(int *)arg);   //*(int *)arg:1:volume up, 0:volume down
        return val;//return the volume of headphone or speaker (0-15)
   // case MO_SET_BACKLIGHT:
   //     val = change_one_step(PWM_BACKLIGHT,*(int *)arg);   
   //     return val;
    case MO_GET_VOL:
        val = get_vol();   
        return val;
    case MO_SET_CLOCK_SW:    // bit31-16:cpu 15-12:hclk_per_cpu 
         val = *(int *)arg;  // 11-8:emi 7-4:gpmi_per_cpu 3-1:xclk_per_24
         clk_switch((val>>16),((val>>12)&0xf),((val>>8)&0xf),((val>>4)&0xf),(val&0xf));
         return 0;
    case MO_GET_MMC_STATUS:
        val=!as3310_get_detect_value();
        return val;
    case MO_GET_AO_STATUS:
        return audio_out;//1:headphone 0:speaker
    default:
		printf("as3310_monitor: unimplemented ioctl=0x%x\n",
		       cmd);
		return -EINVAL;
	}

}
/*
static const struct file_operations as3310x_monitor_ops = {
    .owner   = THIS_MODULE,
    //.write   = as3310x_monitor_write,
    //.read    = as3310x_monitor_read,
    .open    = as3310x_monitor_open,
    .release = as3310x_monitor_release,
    .ioctl   = as3310x_monitor_ioctl,
};
*/
static int  as3310x_monitor_init(struct device *dev){
    int ret;
    keep_sys_open();

#ifdef CONFIG_BOARD_AS3310_MP4_DEMO
    audioout_pin_init();
#endif//CONFIG_BOARD_AS3310_MP4_DEMO

    lradc_batt_init();
    as3310_monitor_init_timer();

    charging = 0;
    thres1 =0;
    thres2 =0;

    return ret;
}

struct device dev_monitor = {
    .name       = "monitor",
    .dev_id     = 0,
    .probe      = as3310x_monitor_init,
    .remove     = NULL,  
    .open       = as3310x_monitor_open,  
    .close      = as3310x_monitor_release,  
    .read       = NULL,  
    .write      = NULL, 
    .ioctl      = as3310x_monitor_ioctl, 
    .suspend    = NULL, 
    .resume     = NULL, 
    .mmap       = NULL,
    .status     = 0, 
    .priv_data  = NULL, 
};

__add_device_lv0(dev_monitor);

//static
//int do_test_del_timer(cmd_tbl_t *cmdtp,int argc,char * argv[])
//{
//
//  if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}
//
//  if(del_timer(&monitor_timer)<0)
//      printf("del_timer error\n");;
//
//  return 0;
//}
//static
//int do_test_mod_timer(cmd_tbl_t *cmdtp,int argc,char * argv[])
//{
//  unsigned long exp;
//  if (argc != 1) {puts("Invalid args.");puts(cmdtp->usage);putc('\n');return 0;}
//
//  exp = jiffies + MONITOR_TIME*10;
//  if(mod_timer(&monitor_timer,exp)<0)
//      printf("mod_timer error\n");;
//
//  return 0;
//}
//
//
//BOOT_CMD(d,do_test_del_timer,
//         " #madplay length",
//         "test mad player");
//BOOT_CMD(m,do_test_mod_timer,
//         " #madplay length",
//         "test mad player");

