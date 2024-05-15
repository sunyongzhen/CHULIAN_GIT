#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "gd32f30x_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "module_key.h"

volatile int key_bitmap = 0;

void pulse_gpio_set(bit_status status)
{
    gpio_bit_write(GPIOB, GPIO_PIN_15, status);  //¼ÌµçÆ÷ ¶ÏÆø¶Ïµç
}

int key_process_cmd(int cmd_id, int * param, int paramLen)
{
    pulse_gpio_set(SET);
    //return key_bitmap;
	return gpio_output_bit_get(GPIOB, GPIO_PIN_15);
}

int get_key_value(void)   
{
    return key_bitmap;
}

static void pulse_gpio_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_bit_write(GPIOB, GPIO_PIN_15, RESET);
}

void key_group_init(void)
{
    pulse_gpio_init();
    gd_eval_key_init(KEY_SWITCH1, KEY_MODE_EXTI);
    gd_eval_key_init(KEY_SWITCH2, KEY_MODE_EXTI);
    gd_eval_key_init(KEY_SWITCH3, KEY_MODE_EXTI);
    gd_eval_key_init(KEY_SWITCH4, KEY_MODE_EXTI);
    gd_eval_key_init(KEY_USER, KEY_MODE_EXTI);
}
