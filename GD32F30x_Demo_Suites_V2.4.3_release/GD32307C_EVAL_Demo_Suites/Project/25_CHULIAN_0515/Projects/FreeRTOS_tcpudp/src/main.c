/*!
    \file  main.c
    \brief enet demo

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f30x.h"
#include "app_netconf.h"
#include "main.h"
#include "tcpip.h"
#include "gd32f307c_eval.h"
#include "app_telnet.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include "udp_echo.h"
#include "protocol_usart.h"
#include "protocol_i2c.h"
#include "module_display.h"
#include "module_audio.h"
#include "module_led.h"
#include "module_gd25qxx.h"
#include "module_key.h"
#include "module_adc.h"
#include "module_rtc.h"
#include "app_service_console.h"
#include "app_mqtt.h"
#include "app_rs485net.h"
#include "app_configure.h"
#include "app_ota.h"
#include "systick.h"

#define INIT_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define USART_TASK_PRIO    ( tskIDLE_PRIORITY + 1 )

extern struct netif g_mynetif;
extern struct config_data * p_configuration_data;

void init_task(void * pvParameters);
int check_net(void);


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{

	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x2000);

    /* configure 4 bits pre-emption priority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    
    /* 初始化一些硬件资源 */
    /* configure SPI0 GPIO and parameter */
    spi_flash_init();

    read_data_and_analyze(RECV_DATA_NUM);
    /* initilaize the rtc */
    rtc_calendar_init();
    
    /* I2C 初始化 */
    i2c_config();

    /* initilaize the key */
    key_group_init();
    
    /* initilaize the usart */
    usart_init();
	
	/* initilaize the rs485 */
    rs485net_task_init();
	rs485net_task_start();
    /* initilaize the led */
    led_group_init();
    
    /* init task */
    xTaskCreate(init_task, "INIT", configMINIMAL_STACK_SIZE * 2, NULL, INIT_TASK_PRIO, NULL);

    /* start scheduler */
    vTaskStartScheduler();

    while (1) {
    }
}

/*!
    \brief      init task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
void init_task(void * pvParameters)
{
	
	int ret = 0;
    /* initilaize the lcd */
	printf("##############################version:222222222222222\n");

	erase_page(1, 3);

    display_init();
    
    audio_init();

    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    enet_system_setup();

    // TODO:根据spi flash数据选择静态或DHCP
    /* initilaize the LwIP stack */
	if(p_configuration_data->local_ip.ipaddr0 != 0 && p_configuration_data->local_ip.ipaddr1 != 0
		&& p_configuration_data->gateway.ipaddr3 != 0){
			lwip_stack_init(USE_STATIC_IP);
	}else{
		lwip_stack_init(USE_DHCP);
	}	
    /* initilaize the tcp server: telnet 8000 */
//    telnet_task_init();
//    register_telnet_handledata_callback(handle_command);
	
    vTaskDelay(pdMS_TO_TICKS(5000));
	
	/* initilaize the tcp client: echo 23 */
    tcp_client_init();
	
	ret = check_net();
	if(ret == 1){
		
	//	vTaskDelay(pdMS_TO_TICKS(5000));
		
		mqtt_task_init();
		mqtt_task_start();
		
		/* initilaize the udp: echo 1025 */
	//    udp_echo_init();
		if(p_configuration_data->camera_type == 1){
			tcp_server_init();
		}
	}
    for ( ;; ) {
        vTaskDelete(NULL);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM0, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    return ch;
}
