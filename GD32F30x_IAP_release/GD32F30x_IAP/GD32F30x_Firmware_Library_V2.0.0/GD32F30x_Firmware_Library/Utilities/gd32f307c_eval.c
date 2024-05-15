/*!
    \file  gd32f307c_eval.c
    \brief firmware functions to manage leds, keys, COM ports

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

#include <gd32f30x.h>
#include "gd32f307c_eval.h"

/* private variables */
static uint32_t GPIO_PORT[LEDn] = {LED_RST_GPIO_PORT, LED_SIGNAL_GPIO_PORT, LED_ALARM_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn] = {LED_RST_PIN, LED_SIGNAL_PIN, LED_ALARM_PIN};
static rcu_periph_enum GPIO_CLK[LEDn] = {LED_RST_GPIO_CLK, LED_SIGNAL_GPIO_CLK, LED_ALARM_GPIO_CLK};

static uint32_t KEY_PORT[KEYn] = {USER_KEY_GPIO_PORT};
static uint32_t KEY_PIN[KEYn] = {USER_KEY_PIN};
static rcu_periph_enum KEY_CLK[KEYn] = {USER_KEY_GPIO_CLK};
static exti_line_enum KEY_EXTI_LINE[KEYn] = {USER_KEY_EXTI_LINE};
static uint8_t KEY_PORT_SOURCE[KEYn] = {USER_KEY_EXTI_PORT_SOURCE};
static uint8_t KEY_PIN_SOURCE[KEYn] = {USER_KEY_EXTI_PIN_SOURCE};
static uint8_t KEY_IRQn[KEYn] = {USER_KEY_EXTI_IRQn};

static rcu_periph_enum COM_CLK[COMn] = {EVAL_COM0_CLK, EVAL_COM1_CLK, EVAL_COM3_CLK, EVAL_COM4_CLK};
static uint32_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN, EVAL_COM1_TX_PIN, EVAL_COM3_TX_PIN, EVAL_COM4_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN, EVAL_COM1_RX_PIN, EVAL_COM3_RX_PIN, EVAL_COM4_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn + 1] = {EVAL_COM0_GPIO_PORT, EVAL_COM1_GPIO_PORT, EVAL_COM3_GPIO_PORT, EVAL_COM4_TX_GPIO_PORT, EVAL_COM4_RX_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn + 1] = {EVAL_COM0_GPIO_CLK, EVAL_COM1_GPIO_CLK, EVAL_COM3_GPIO_CLK, EVAL_COM4_TX_GPIO_CLK, EVAL_COM4_RX_GPIO_CLK};

/*!
    \brief      configure led GPIO
    \param[in]  lednum: specify the led to be configured
      \arg        LED_RST
      \arg        LED_SIGNAL
      \arg        LED_ALARM
    \param[out] none
    \retval     none
*/
void  gd_eval_led_init (led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    /* configure led GPIO port */ 
    gpio_init(GPIO_PORT[lednum], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);

    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      turn on selected led
    \param[in]  lednum: specify the led to be turned on
      \arg        LED_RST
      \arg        LED_SIGNAL
      \arg        LED_ALARM
    \param[out] none
    \retval     none
*/
void gd_eval_led_on(led_typedef_enum lednum)
{
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      turn off selected led
    \param[in]  lednum: specify the led to be turned off
      \arg        LED_RST
      \arg        LED_SIGNAL
      \arg        LED_ALARM
    \param[out] none
    \retval     none
*/
void gd_eval_led_off(led_typedef_enum lednum)
{
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      toggle selected led
    \param[in]  lednum: specify the led to be toggled
      \arg        LED_RST
      \arg        LED_SIGNAL
      \arg        LED_ALARM
    \param[out] none
    \retval     none
*/
void gd_eval_led_toggle(led_typedef_enum lednum)
{
    gpio_bit_write(GPIO_PORT[lednum], GPIO_PIN[lednum], 
                    (bit_status)(1-gpio_input_bit_get(GPIO_PORT[lednum], GPIO_PIN[lednum])));
}

/*!
    \brief      configure key
    \param[in]  key_num: specify the key to be configured
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER: user key
    \param[in]  key_mode: specify button mode
      \arg        KEY_MODE_GPIO: key will be used as simple IO
      \arg        KEY_MODE_EXTI: key will be connected to EXTI line with interrupt
    \param[out] none
    \retval     none
*/
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode)
{
    /* enable the key clock */
    rcu_periph_clock_enable(KEY_CLK[key_num]);
    rcu_periph_clock_enable(RCU_AF);

    /* configure button pin as input */
    gpio_init(KEY_PORT[key_num], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, KEY_PIN[key_num]);

    if (key_mode == KEY_MODE_EXTI) {
        /* enable and set key EXTI interrupt to the lowest priority */
        nvic_irq_enable(KEY_IRQn[key_num], 2U, 0U);

        /* connect key EXTI line to key GPIO pin */
        gpio_exti_source_select(KEY_PORT_SOURCE[key_num], KEY_PIN_SOURCE[key_num]);

        /* configure key EXTI line */
        exti_init(KEY_EXTI_LINE[key_num], EXTI_INTERRUPT, EXTI_TRIG_FALLING);
        exti_interrupt_flag_clear(KEY_EXTI_LINE[key_num]);
    }
}

/*!
    \brief      return the selected key state
    \param[in]  key: specify the key to be checked
      \arg        KEY_TAMPER: tamper key
      \arg        KEY_WAKEUP: wakeup key
      \arg        KEY_USER: user key
    \param[out] none
    \retval     the key's GPIO pin value
*/
uint8_t gd_eval_key_state_get(key_typedef_enum key)
{
    return gpio_input_bit_get(KEY_PORT[key], KEY_PIN[key]);
}

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        EVAL_COM0: COM0 on the board
      \arg        EVAL_COM1: COM1 on the board
    \param[out] none
    \retval     none
*/
void gd_eval_com_init(uint32_t com)
{
    uint32_t com_id = 0U;
    if (EVAL_COM0 == com) {
        com_id = 0U;
        /* enable GPIO clock */
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
        /* enable USART clock */
        rcu_periph_clock_enable(COM_CLK[com_id]);
        /* connect port to USARTx_Tx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
        /* connect port to USARTx_Rx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    } else if (EVAL_COM1 == com) {
        com_id = 1U;
        /* enable GPIO clock */
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
        /* enable USART clock */
        rcu_periph_clock_enable(COM_CLK[com_id]);
        /* remap */
        gpio_pin_remap_config(GPIO_USART1_REMAP, ENABLE);
        /* connect port to USARTx_Tx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
        /* connect port to USARTx_Rx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    } else if (EVAL_COM3 == com) {
        com_id = 2U;
        /* enable GPIO clock */
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
        /* enable USART clock */
        rcu_periph_clock_enable(COM_CLK[com_id]);
        /* connect port to USARTx_Tx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
        /* connect port to USARTx_Rx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    } else if (EVAL_COM4 == com) {
        com_id = 3U;
        /* enable GPIO clock */
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);
        rcu_periph_clock_enable(COM_GPIO_CLK[com_id + 1]);
        /* enable USART clock */
        rcu_periph_clock_enable(COM_CLK[com_id]);
        /* connect port to USARTx_Tx */
        gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);
        /* connect port to USARTx_Rx */
        gpio_init(COM_GPIO_PORT[com_id + 1], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);
    }

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, 115200U);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}
