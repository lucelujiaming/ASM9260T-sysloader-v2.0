/*

Alpha Scale AS3310X IronPalms Console
He Yong, AlpScale Software Engineering, heyong@alpscale.com

GPIO SPI Source file
 
Change log: 

------------------- Version 1.0  ----------------------
He Yong 2007-10-08
    -- Create file

*/

#include <common.h>
#include <drivers/gpio_spi.h>

volatile struct gpio_spi_control  spi_con;

void gpio_spi_interrupt_handler(void){

    if (spi_con.status) {    // if in transaction 
    
        if (!spi_con.edge) { 
            /* falling edge , give out data*/

            if ( spi_con.index < 0 ){
                spi_con.status = 0; /* transaction complete */
                set_GPIO(spi_con.sdo_port,spi_con.sdo_pin);
                return;
            }
            write_GPIO(spi_con.sdo_port,spi_con.sdo_pin,
                       (spi_con.wdata & (1<<spi_con.index)) );
        }
        else {
            /* rising edge , read data and count for index */
            if (spi_con.index < spi_con.data_bits) {
                if (read_GPIO(spi_con.sdi_port,spi_con.sdi_pin)) {
                    spi_con.rdata |= (1<<spi_con.index);
                }
            }
            spi_con.index--;
        }

        /* handle clock signal */
        write_GPIO(spi_con.scl_port,spi_con.scl_pin,spi_con.edge);

        /* for next SCL edge */
        spi_con.edge = 1 - spi_con.edge;

    }
}


int gpio_spi_init(){

    irq_action_t spi_irq;
    int ret;

    spi_con.edge = 0;
    spi_con.status = 0;    // 0 stop, 1 run         
    spi_con.total_bits = GPIO_SPI_BITS;
    spi_con.data_bits = GPIO_SPI_DATA_BITS;

    spi_con.index = -1;

    spi_con.scl_port =  GPIO_SPI_SCL_PORT;   
    spi_con.scl_pin =   GPIO_SPI_SCL_PIN ;   
    spi_con.sdi_port =  GPIO_SPI_SDI_PORT;   
    spi_con.sdi_pin =   GPIO_SPI_SDI_PIN ;   
    spi_con.sdo_port =  GPIO_SPI_SDO_PORT;   
    spi_con.sdo_pin =   GPIO_SPI_SDO_PIN ;   

    printf("GPIO SPI Inited [SCL(%d,%d)  SDI(%d,%d)  SDO(%d,%d)]\n",
           spi_con.scl_port,spi_con.scl_pin,
           spi_con.sdi_port,spi_con.sdi_pin,
           spi_con.sdo_port,spi_con.sdo_pin
           );

    /*  arch specific configuration */
    /* ================== GPIO ================*/
    request_as3310_gpio(spi_con.scl_port,spi_con.scl_pin,PIN_FUNCTION_GPIO);
    set_GPIO(spi_con.scl_port,spi_con.scl_pin);

    request_as3310_gpio(spi_con.sdi_port,spi_con.sdi_pin,PIN_FUNCTION_GPIO);

    request_as3310_gpio(spi_con.sdo_port,spi_con.sdo_pin,PIN_FUNCTION_GPIO);
    set_GPIO(spi_con.sdo_port,spi_con.sdo_pin);

    spi_irq.irq = INT_AS3310_TIMER2;
    spi_irq.irq_handler = do_irq_timer_2;
    spi_irq.clear = clear_irq_timer;
    spi_irq.priv_data = INT_AS3310_TIMER2;
    ret = request_irq(&spi_irq);
    if (ret) {
        printf("GPIO SPI Request IRQ %d Error\n",INT_AS3310_TIMER2);
    	ret = -1;
    	goto gpio_spi_fail;
    }
    
    timer2_init(GPIO_SPI_BANDRATE);

gpio_spi_fail:
    release_as3310_gpio(spi_con.scl_port,spi_con.scl_pin);
    release_as3310_gpio(spi_con.sdi_port,spi_con.sdi_pin);
    release_as3310_gpio(spi_con.sdo_port,spi_con.sdo_pin);
    return ret;
}

int gpio_spi_release(void){
    timer2_release();
    free_irq(INT_AS3310_TIMER2);
    release_as3310_gpio(spi_con.scl_port,spi_con.scl_pin);
    release_as3310_gpio(spi_con.sdi_port,spi_con.sdi_pin);
    release_as3310_gpio(spi_con.sdo_port,spi_con.sdo_pin);
    return 0;
}

int gpio_spi_trans(int data){

    spi_con.index = spi_con.total_bits - 1;  // reset index
    spi_con.rdata = 0;
    spi_con.wdata = data;
    spi_con.edge = 0;
    spi_con.status = 1;    // 0 stop, 1 run        

    while (spi_con.status != 0) { // wait for complete
    }

    return spi_con.rdata;
}


int do_gpio_spi(cmd_tbl_t *cmdtp,int argc,char * argv[]){
    int val,ret;
    val = TextToLong_TRL(argv[1]);
    ret = gpio_spi_trans(val);
    printf("SPI returned 0x%x\n",ret);
    return 0;
}

 BOOT_CMD(spi,do_gpio_spi,
         " #do_gpio_spi <data>",
         "Test for GPIO SPI");

