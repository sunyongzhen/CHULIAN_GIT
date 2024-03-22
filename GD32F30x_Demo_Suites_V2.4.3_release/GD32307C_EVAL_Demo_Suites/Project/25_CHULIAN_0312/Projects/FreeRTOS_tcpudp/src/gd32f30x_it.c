/*!
    \file  gd32f30x_it.c
    \brief interrupt service routines

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
#include "gd32f30x_it.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lwip/sys.h"

#include "module_led.h"
#include "module_key.h"
#include "module_audio.h"
#include "protocol_usart.h"
#include "stdio.h"
#include "gd32f307c_eval.h"

//#define AUDIO_TEST
#ifdef AUDIO_TEST
#include "test_wav.h"
static uint32_t pos = 0;
#endif

extern xSemaphoreHandle g_rx_semaphore;
uint16_t buf_size = 512;
extern volatile uint8_t buf1_read_enable;
extern volatile uint8_t buf2_read_enable;
extern volatile uint16_t * buf1_p;
extern volatile uint16_t * buf2_p;

//uint8_t buf[304] = {
//	0x52, 0x49, 0x46, 0x46, 0xDC, 0x92, 0x01, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20, 
//    0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 
//    0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0xF0, 0x90, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
//    0xFF, 0xFF, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFC, 0xFF, 0xFA, 0xFF, 
//    0xF9, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF8, 0xFF, 0xFA, 0xFF, 0xFE, 0xFF, 
//    0x02, 0x00, 0x06, 0x00, 0x0A, 0x00, 0x0D, 0x00, 0x0F, 0x00, 0x10, 0x00, 0x10, 0x00, 0x0E, 0x00, 
//    0x0B, 0x00, 0x08, 0x00, 0x02, 0x00, 0xFE, 0xFF, 0xFA, 0xFF, 0xF5, 0xFF, 0xF2, 0xFF, 0xEE, 0xFF, 
//    0xEE, 0xFF, 0xEF, 0xFF, 0xF0, 0xFF, 0xF4, 0xFF, 0xF8, 0xFF, 0xFC, 0xFF, 0x01, 0x00, 0x04, 0x00, 
//    0x08, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0x0A, 0x00, 0x09, 0x00, 0x06, 0x00, 0x04, 0x00, 
//    0x02, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFC, 0xFF, 0xFC, 0xFF, 0xFC, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 
//    0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFE, 0xFF, 0xFA, 0xFF, 
//    0xF9, 0xFF, 0xF7, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF8, 0xFF, 0xFA, 0xFF, 0xFD, 0xFF, 
//    0x00, 0x00, 0x04, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x0E, 0x00, 0x10, 0x00, 0x10, 0x00, 0x0F, 0x00, 
//    0x0C, 0x00, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFA, 0xFF, 0xF7, 0xFF, 0xF2, 0xFF, 0xF0, 0xFF, 
//    0xEF, 0xFF, 0xEE, 0xFF, 0xF1, 0xFF, 0xF2, 0xFF, 0xF6, 0xFF, 0xFA, 0xFF, 0xFF, 0xFF, 0x04, 0x00, 
//    0x06, 0x00, 0x0A, 0x00, 0x0B, 0x00, 0x0C, 0x00, 0x0A, 0x00, 0x0A, 0x00, 0x08, 0x00, 0x04, 0x00, 
//    0x02, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFD, 0xFF, 0xFC, 0xFF, 0xFD, 0xFF, 0xFD, 0xFF, 0xFE, 0xFF, 
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFC, 0xFF, 
//    0xFA, 0xFF, 0xF8, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF6, 0xFF, 0xF8, 0xFF, 0xFA, 0xFF, 0xFD, 0xFF,};
uint32_t pos = 0;
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1) {
    }
}

/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* frame received */
    if (SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS)) {
        /* give the semaphore to wakeup LwIP task */
        xSemaphoreGiveFromISR(g_rx_semaphore, &xHigherPriorityTaskWoken);
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    /* switch tasks if necessary */
    if (pdFALSE != xHigherPriorityTaskWoken) {
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

void SPI2_IRQHandler(void)
{
    if (SET == spi_i2s_interrupt_flag_get(SPI2, SPI_I2S_INT_TBE)) {
        /* send audio data */
        /* send the data read from the memory */
#ifdef AUDIO_TEST

        spi_i2s_data_transmit(SPI2, sine_wave[pos++ % 102688]);
#else
//		spi_i2s_data_transmit(SPI2, buf[pos++ % 304]);
		
		if(buf1_read_enable){
			spi_i2s_data_transmit(SPI2, *(buf1_p + pos));
			pos ++;
			if(pos == buf_size){
				buf1_read_enable = 0;
				pos = 0;
			}
		}
		else if(buf2_read_enable){
			spi_i2s_data_transmit(SPI2, *(buf2_p + pos));
			pos ++;
			if(pos == buf_size){
				buf2_read_enable = 0;
				pos = 0;
			}
		}
		else
		{
			pos = 0;
			spi_i2s_data_transmit(SPI2, 0);
		}
#endif
    }
}

/*!
    \brief      this function handles external lines 5 to 9 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI5_9_IRQHandler(void)
{
	//add by heyt 20240314
	if (RESET != exti_interrupt_flag_get(EXTI_6)) {
        if (gpio_input_bit_get(GPIOC , GPIO_PIN_6) == SET)
        {
			key_bitmap |= 1 << KEY_USER_ELSE;
			audio_process_cmd(5, 0, 0);
        }
        else
        {
			key_bitmap &= ~(1 << KEY_USER_ELSE);
        }
        exti_interrupt_flag_clear(EXTI_6);
	}
	//add by heyt 20240314 end
    if (RESET != exti_interrupt_flag_get(EXTI_8)) {
        if (gpio_input_bit_get(GPIOB , GPIO_PIN_8) == SET)
        {
			key_bitmap |= 1 << KEY_SWITCH1;
        }
        else
        {
			key_bitmap &= ~(1 << KEY_SWITCH1);
        }

        if (gpio_input_bit_get(GPIOC , GPIO_PIN_8) == SET)
        {
			key_bitmap |= 1 << KEY_USER;
			audio_process_cmd(5, 0, 0);
        }
        else
        {
			key_bitmap &= ~(1 << KEY_USER);
        }
        exti_interrupt_flag_clear(EXTI_8);
    }

    if (RESET != exti_interrupt_flag_get(EXTI_9)) {
        if (gpio_input_bit_get(GPIOB , GPIO_PIN_9) == SET)
        {
			key_bitmap |= 1 << KEY_SWITCH2;
        }
        else
        {
			key_bitmap &= ~(1 << KEY_SWITCH2);
        }
        exti_interrupt_flag_clear(EXTI_9);
    }

}

void EXTI10_15_IRQHandler(void)
{
    if (RESET != exti_interrupt_flag_get(EXTI_10)) {
		if (gpio_input_bit_get(GPIOB , GPIO_PIN_10) == SET)
        {
			key_bitmap |= 1 << KEY_SWITCH3;
        }
        else
        {
			key_bitmap &= ~(1 << KEY_SWITCH3);
        }
        exti_interrupt_flag_clear(EXTI_10);
    }

    if (RESET != exti_interrupt_flag_get(EXTI_14)) {
		if (gpio_input_bit_get(GPIOB , GPIO_PIN_14) == SET)
        {
			key_bitmap |= 1 << KEY_SWITCH4;
        }
        else
        {
			key_bitmap &= ~(1 << KEY_SWITCH4);
        }
        exti_interrupt_flag_clear(EXTI_14);
    }
}

/*!
    \brief      this function handles USART0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART0_IRQHandler(void)
{

    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        /* read one byte from the receive data register */
        usart0.rxbuffer[uart0_rx_counter++] = (uint8_t)usart_data_receive(USART0);
        if (uart1_rx_counter >= usart0.rxsize)
        {
            /* disable the USART0 receive interrupt */
            usart_interrupt_disable(USART0, USART_INT_RBNE);
        }
    }

    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE)) {
        /* write one byte to the transmit data register */
        usart_data_transmit(USART0, usart0.txbuffer[uart0_tx_counter++]);

        if (uart0_tx_counter >= usart0.txsize)
        {
            uart0_tx_counter = 0;
            usart0.txsize = 0;
            /* disable the USART0 transmit interrupt */
            usart_interrupt_disable(USART0, USART_INT_TBE);
        }
    }
}


/*!
    \brief      this function handles USART1 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART1_IRQHandler(void)
{

    if (RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)) {
        /* read one byte from the receive data register */
        usart1.rxbuffer[uart1_rx_counter++] = (uint8_t)usart_data_receive(USART1);
        if (uart1_rx_counter >= usart1.rxsize)
        {
            /* disable the USART1 receive interrupt */
            usart_interrupt_disable(USART1, USART_INT_RBNE);
        }
    }

    if (RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_TBE)) {
        /* write one byte to the transmit data register */
        usart_data_transmit(USART1, usart1.txbuffer[uart1_tx_counter++]);

        if (uart1_tx_counter >= usart1.txsize)
        {
            uart1_tx_counter = 0;
            usart1.txsize = 0;
            /* disable the USART1 transmit interrupt */
            usart_interrupt_disable(USART1, USART_INT_TBE);
        }
    }
}

/*!
    \brief      this function handles USART3 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UART3_IRQHandler(void)
{
    if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE)) {
        /* read one byte from the receive data register */
        uart3.rxbuffer[uart3_rx_counter++] = (uint8_t)usart_data_receive(UART3);

        if (uart3_rx_counter >= uart3.rxsize)
        {
            /* disable the USART3 receive interrupt */
            usart_interrupt_disable(UART3, USART_INT_RBNE);
        }
    }

    if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_TBE)) {
        /* write one byte to the transmit data register */
        usart_data_transmit(UART3, uart3.txbuffer[uart3_tx_counter++]);
        if (uart3_tx_counter >= uart3.txsize)
        {
            uart3_tx_counter = 0;
            uart3.txsize = 0;
            /* disable the USART3 transmit interrupt */
            usart_interrupt_disable(UART3, USART_INT_TBE);
        }
    }
}

/*!
    \brief      this function handles USART3 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UART4_IRQHandler(void)
{
    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)) {
        /* read one byte from the receive data register */
        uart4.rxbuffer[uart4_rx_counter++] = (uint8_t)usart_data_receive(UART4);

        if (uart4_rx_counter >= uart4.rxsize)
        {
            /* disable the USART3 receive interrupt */
            usart_interrupt_disable(UART4, USART_INT_RBNE);
        }
    }

    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_TBE)) {
        /* write one byte to the transmit data register */
        usart_data_transmit(UART4, uart4.txbuffer[uart4_tx_counter++]);
        if (uart4_tx_counter >= uart4.txsize)
        {
            uart4_tx_counter = 0;
            uart4.txsize = 0;
            /* disable the USART3 transmit interrupt */
            usart_interrupt_disable(UART4, USART_INT_TBE);
        }
    }
}
