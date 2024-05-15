/*!
    \file  main.c
    \brief GPIO running led
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "systick.h"

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{  
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x2000);
    gd_eval_led_init(LED_RST);
    gd_eval_led_init(LED_SIGNAL);
    gd_eval_led_init(LED_ALARM);
    
    systick_config();
    
    while(1){
//        /* turn on LED1 */
//        gd_eval_led_on(LED2);
//        /* insert 200 ms delay */
//        delay_1ms(200);
//        
//        /* turn on LED2 */
//        gd_eval_led_on(LED3);
//        /* insert 200 ms delay */
//        delay_1ms(200);
//        
//        /* turn on LED3 */
//        gd_eval_led_on(LED4);
//        /* insert 200 ms delay */
//        delay_1ms(200);
//        
//        gd_eval_led_on(LED5);
//        /* insert 200 ms delay */
//        delay_1ms(200); 

//        /* turn off LEDs */
//        gd_eval_led_off(LED2);
//        gd_eval_led_off(LED3);
//        gd_eval_led_off(LED4);
//        gd_eval_led_off(LED5);
//        
//        /* insert 200 ms delay */
        gd_eval_led_toggle(LED_RST);
        gd_eval_led_toggle(LED_SIGNAL);
        gd_eval_led_toggle(LED_ALARM);  
        delay_1ms(200);
    }
}
