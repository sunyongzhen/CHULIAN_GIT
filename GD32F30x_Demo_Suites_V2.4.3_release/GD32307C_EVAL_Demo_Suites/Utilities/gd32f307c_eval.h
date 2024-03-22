/*!
    \file  gd32f307c_eval.h
    \brief definitions for GD32f307C_EVAL's leds, keys and COM ports hardware resources

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

#ifndef GD32F307C_EVAL_H
#define GD32F307C_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"
     
/* exported types */
typedef enum 
{
    LED_RST = 0,
    LED_SIGNAL = 1,
    LED_ALARM = 2
} led_typedef_enum;

typedef enum 
{
    KEY_USER = 0,
	KEY_SWITCH1 = 1,
	KEY_SWITCH2 = 2,
	KEY_SWITCH3 = 3,
	KEY_SWITCH4 = 4,
	KEY_USER_ELSE = 5
} key_typedef_enum;

typedef enum 
{
    KEY_MODE_GPIO = 0,
    KEY_MODE_EXTI = 1
} keymode_typedef_enum;

/* eval board low layer led */
#define LEDn                             3U

#define LED_RST_PIN                         GPIO_PIN_11
#define LED_RST_GPIO_PORT                   GPIOD
#define LED_RST_GPIO_CLK                    RCU_GPIOD
  
#define LED_SIGNAL_PIN                      GPIO_PIN_1
#define LED_SIGNAL_GPIO_PORT                GPIOE
#define LED_SIGNAL_GPIO_CLK                 RCU_GPIOE
  
#define LED_ALARM_PIN                       GPIO_PIN_2
#define LED_ALARM_GPIO_PORT                 GPIOE
#define LED_ALARM_GPIO_CLK                  RCU_GPIOE

#define COMn                             4U

#define EVAL_COM0                        USART0
#define EVAL_COM0_CLK                    RCU_USART0
#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA

#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCU_USART1
#define EVAL_COM1_TX_PIN                 GPIO_PIN_5
#define EVAL_COM1_RX_PIN                 GPIO_PIN_6
#define EVAL_COM1_GPIO_PORT              GPIOD
#define EVAL_COM1_GPIO_CLK               RCU_GPIOD

#define EVAL_COM3                        UART3
#define EVAL_COM3_CLK                    RCU_UART3
#define EVAL_COM3_TX_PIN                 GPIO_PIN_10
#define EVAL_COM3_RX_PIN                 GPIO_PIN_11
#define EVAL_COM3_GPIO_PORT              GPIOC
#define EVAL_COM3_GPIO_CLK               RCU_GPIOC

#define EVAL_COM4                        UART4
#define EVAL_COM4_CLK                    RCU_UART4
#define EVAL_COM4_TX_PIN                 GPIO_PIN_12
#define EVAL_COM4_RX_PIN                 GPIO_PIN_2
#define EVAL_COM4_TX_GPIO_PORT           GPIOC
#define EVAL_COM4_RX_GPIO_PORT           GPIOD
#define EVAL_COM4_TX_GPIO_CLK            RCU_GPIOC
#define EVAL_COM4_RX_GPIO_CLK            RCU_GPIOD

#define KEYn                             5U

/* user push-button PC8 */
#define USER_KEY_PIN                     GPIO_PIN_8
#define USER_KEY_GPIO_PORT               GPIOC
#define USER_KEY_GPIO_CLK                RCU_GPIOC
#define USER_KEY_EXTI_LINE               EXTI_8
#define USER_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOC
#define USER_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_8
#define USER_KEY_EXTI_IRQn               EXTI5_9_IRQn
//add by heyt 20240314
/* user push-button PC6 */
#define USER_ELSE_KEY_PIN                GPIO_PIN_6
#define USER_ELSE_KEY_GPIO_PORT          GPIOC
#define USER_ELSE_KEY_GPIO_CLK           RCU_GPIOC
#define USER_ELSE_KEY_EXTI_LINE          EXTI_6
#define USER_ELSE_KEY_EXTI_PORT_SOURCE   GPIO_PORT_SOURCE_GPIOC
#define USER_ELSE_KEY_EXTI_PIN_SOURCE    GPIO_PIN_SOURCE_6
#define USER_ELSE_KEY_EXTI_IRQn          EXTI5_9_IRQn

/* switch1 PB8 */
#define SW1_KEY_PIN                     GPIO_PIN_8
#define SW1_KEY_GPIO_PORT               GPIOB
#define SW1_KEY_GPIO_CLK                RCU_GPIOB
#define SW1_KEY_EXTI_LINE               EXTI_8
#define SW1_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOB
#define SW1_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_8
#define SW1_KEY_EXTI_IRQn               EXTI5_9_IRQn

/* switch2 PB9 */
#define SW2_KEY_PIN                     GPIO_PIN_9
#define SW2_KEY_GPIO_PORT               GPIOB
#define SW2_KEY_GPIO_CLK                RCU_GPIOB
#define SW2_KEY_EXTI_LINE               EXTI_9
#define SW2_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOB
#define SW2_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_9
#define SW2_KEY_EXTI_IRQn               EXTI5_9_IRQn

/* switch3 PB10 */
#define SW3_KEY_PIN                     GPIO_PIN_10
#define SW3_KEY_GPIO_PORT               GPIOB
#define SW3_KEY_GPIO_CLK                RCU_GPIOB
#define SW3_KEY_EXTI_LINE               EXTI_10
#define SW3_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOB
#define SW3_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_10
#define SW3_KEY_EXTI_IRQn               EXTI10_15_IRQn

/* switch4 PB14 */
#define SW4_KEY_PIN                     GPIO_PIN_14
#define SW4_KEY_GPIO_PORT               GPIOB
#define SW4_KEY_GPIO_CLK                RCU_GPIOB
#define SW4_KEY_EXTI_LINE               EXTI_14
#define SW4_KEY_EXTI_PORT_SOURCE        GPIO_PORT_SOURCE_GPIOB
#define SW4_KEY_EXTI_PIN_SOURCE         GPIO_PIN_SOURCE_14
#define SW4_KEY_EXTI_IRQn               EXTI10_15_IRQn

/* function declarations */
/* configure led GPIO */
void gd_eval_led_init(led_typedef_enum lednum);
/* turn on selected led */
void gd_eval_led_on(led_typedef_enum lednum);
/* turn off selected led */
void gd_eval_led_off(led_typedef_enum lednum);
/* toggle the selected led */
void gd_eval_led_toggle(led_typedef_enum lednum);
/* configure key */
void gd_eval_key_init(key_typedef_enum key_num, keymode_typedef_enum key_mode);
/* return the selected key state */
uint8_t gd_eval_key_state_get(key_typedef_enum key);
/* configure COM port */
void gd_eval_com_init(uint32_t com);

#ifdef __cplusplus
}
#endif

#endif /* GD32F307C_EVAL_H */
