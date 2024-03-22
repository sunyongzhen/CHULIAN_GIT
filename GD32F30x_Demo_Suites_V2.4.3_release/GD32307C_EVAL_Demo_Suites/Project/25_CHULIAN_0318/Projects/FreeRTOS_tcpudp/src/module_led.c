#include "protocol_rs485.h"
#include "protocol_usart.h"
#include "protocol_mqtt.h"
#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )

union led_reg_u
{
    struct {
        unsigned char data0: 1; // 工作方式 0:常亮 1:闪烁
        unsigned char ctrl0: 1; // 控制方式 0:熄灭 1:工作
        unsigned char data1: 1;
        unsigned char ctrl1: 1;
        unsigned char data2: 1;
        unsigned char ctrl2: 1;
        unsigned char data3: 1;
        unsigned char ctrl3: 1;
    } reg;
    unsigned char val;
};

struct module_led
{
    const char *name;   // 设备名称
    union led_reg_u reg;
};

struct module_led led_dev = {
    "led_group",
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    },
};

static void led_task(void * pvParameters);

/**
 * @function [deal led related commands]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:03:08+0800
 * @version  [1.0.0]
 * @param    cmd_id                   [command id]
 * @param    param                    [param]
 * @param    paramLen                 [param length]
 * @return                            [deal result]
 */
int led_process_cmd(int cmd_id, int * param, int paramLen)
{
    /* TODO: handle led commands using net or usart */
    led_dev.reg.val = *param;
//    printf("%s:%x\n", "led_dev.reg", led_dev.reg.val);
    return 0;
}

/**
 * @function [init led device]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:09:12+0800
 * @version  [1.0.0]
 * @return                            [deal result]
 */
void led_group_init(void)
{
    gd_eval_led_init(LED_RST);
    gd_eval_led_init(LED_SIGNAL);
    gd_eval_led_init(LED_ALARM);

    led_dev.reg.reg.ctrl0 = 0;
    led_dev.reg.reg.data0 = 0;
    led_dev.reg.reg.ctrl1 = 0;
    led_dev.reg.reg.data1 = 0;
    led_dev.reg.reg.ctrl2 = 0;
    led_dev.reg.reg.data2 = 0;
    led_dev.reg.reg.ctrl3 = 1;
    led_dev.reg.reg.data3 = 1;

    /* start toogle LED task every 250ms */
    xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
}

/*!
    \brief      led task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
static void led_task(void * pvParameters)
{
    for ( ;; ) {
//		if(rcu_flag_get(RCU_FLAG_EPRST)){
//			led_dev.reg.reg.ctrl0 = 1;
//		}else{
//			led_dev.reg.reg.ctrl0 = 0;
//		}
        if (led_dev.reg.reg.ctrl0)
        {
            if (led_dev.reg.reg.data0)
            {
                gd_eval_led_toggle(LED_RST);
            }
            else
            {
                gd_eval_led_on(LED_RST);
            }
        }
        else
        {
            gd_eval_led_off(LED_RST);
        }

        if (led_dev.reg.reg.ctrl1)
        {
            if (led_dev.reg.reg.data1)
            {
                gd_eval_led_toggle(LED_SIGNAL);
            }
            else
            {
                gd_eval_led_on(LED_SIGNAL);
            }
        }
        else
        {
            gd_eval_led_off(LED_SIGNAL);
        }

        if (led_dev.reg.reg.ctrl2)
        {
            if (led_dev.reg.reg.data2)
            {
			
                gd_eval_led_toggle(LED_ALARM);

            }
            else
            {
				gd_eval_led_on(LED_ALARM);
            }
        }
        else
        {	
            gd_eval_led_off(LED_ALARM);

        }
        vTaskDelay(250);
    }
}
