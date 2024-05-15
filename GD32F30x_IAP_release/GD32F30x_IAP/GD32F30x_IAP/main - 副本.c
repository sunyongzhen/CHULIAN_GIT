/*!
    \file  main.c
    \brief The main function file
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-06, V1.0.0, firmware for GD32F3x0
*/

#include "gd32f30x.h"
#include <stdlib.h>
#include <stdio.h>
#include "gd32f30x_it.h"
#include "main.h"

void USART_config(void);
void DMA_config(void);
void NVIC_config(void);
void gpio_config(void);


typedef  void (*pFunction)(void);
#define rxbuffersize   0x200
#define ApplicationAddress    0x08002000

pFunction Jump_To_Application;
uint32_t JumpAddress = 0;
uint8_t rxbuffer[rxbuffersize];

uint32_t num_of_data_write = rxbuffersize;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    gpio_config();
    
    if(SET == gpio_input_bit_get(GPIOA, GPIO_PIN_0))
    {
            if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)        //ApplicationAddress为新程序的起始地址，检查栈顶地址是否合法，即栈顶地址是否为0x2000xxxx（内置SRAM）
                {
                    usart_disable(USART0);
                    dma_channel_disable(DMA0, DMA_CH4);
                    JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);               //用户代码区第二个字存储为新程序起始地址（新程序复位向量指针）
                    Jump_To_Application = (pFunction) JumpAddress;                          
                    __set_MSP(*(__IO uint32_t*) ApplicationAddress);                        //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
                    Jump_To_Application();                                                  //设置PC指针为新程序复位中断函数的地址
                }
    }
    
    DMA_config();
    USART_config();
    NVIC_config();
    
    
    while (1){}

}

void USART_config(void)
{ 
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

       /* connect port to USART0_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USART0_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_parity_config(USART0, USART_PM_EVEN);
    usart_word_length_set(USART0, USART_WL_9BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    
    usart_enable(USART0);
    
    /* Enable USART0 DMA Rx request */
    usart_dma_receive_config(USART0, USART_DENR_DISABLE);
    
    usart_interrupt_enable(USART0, USART_INT_RBNE);
    
    usart_enable(USART0);
}

void gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
}

void DMA_config(void)
{
    dma_parameter_struct dma_init_struct;
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);
    
    /* deinitialize DMA channel2 */
    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)&rxbuffer[1];
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = (uint32_t)num_of_data_write;
    
    dma_init_struct.periph_addr = 0x40013804;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    
    dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);
    dma_interrupt_enable(DMA0, DMA_CH4, DMA_INT_FTF);
}

void NVIC_config(void)
{
    /* Enable the USART0 Interrupt */
    nvic_irq_enable(USART0_IRQn, 0, 0);
    
    /* Enable the DMA Interrupt */
    nvic_irq_enable(DMA0_Channel4_IRQn, 0, 0);
}

ErrStatus gd_check_sum(uint8_t *data, uint8_t data_num, uint8_t check_data)
{
    uint8_t check_sum = 0;
    uint8_t *p = data;
    uint8_t i;
    if(1 == data_num){
        check_sum = ~(*p);
    }else{
        for(i = 0; i < data_num; i++){
            check_sum ^= *p;
            p++;
        }
    }
    if(check_sum == check_data){
        return SUCCESS;
    }else{
        return ERROR;
    }
}

