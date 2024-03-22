/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "protocol_usart.h"
#include "stdio.h"
#include "string.h"

#define GPIO_USART0_DERE GPIO_PIN_2
#define GPIO_USART1_DERE GPIO_PIN_3
#define GPIO_UART3_DERE  GPIO_PIN_4
#define GPIO_UART4_DERE  GPIO_PIN_5

uart_trans_buffer_t usart0;
uart_trans_buffer_t usart1;
uart_trans_buffer_t uart3;
uart_trans_buffer_t uart4;

volatile uint8_t uart0_tx_counter = 0;
volatile uint8_t uart0_rx_counter = 0;
volatile uint8_t uart1_tx_counter = 0;
volatile uint8_t uart1_rx_counter = 0;
volatile uint8_t uart3_tx_counter = 0;
volatile uint8_t uart3_rx_counter = 0;
volatile uint8_t uart4_tx_counter = 0;
volatile uint8_t uart4_rx_counter = 0;

int usart_send(uint32_t usart_periph, uint8_t* data, uint8_t size)
{
    // TODO: 发送前填充缓存区数据
    switch (usart_periph)
    {
        case USART0:
            /* 如果上一次没有写完，返回-1, txsize清零由中断实现 */
            if (uart0_tx_counter > 0)
                return -1;
            /* 拷贝到txbuffer */
            memcpy(usart0.txbuffer, data, size);
            /* 设置串口0发送数据大小 */
            usart0.txsize = size;
            /* 485向外发送 */
            gpio_bit_write(GPIOC, GPIO_USART0_DERE, SET);
//            usart_interrupt_enable(usart_periph, USART_INT_TBE);
//            while(uart0_tx_counter < usart0.txsize);
//            while(uart0_tx_counter);
            break;
        case USART1:
            if (uart1_tx_counter > 0)
                return -1;
            memcpy(usart1.txbuffer, data, size);
            usart1.txsize = size;
            gpio_bit_write(GPIOC, GPIO_USART1_DERE, SET);
//            usart_interrupt_enable(usart_periph, USART_INT_TBE);
//            while(uart1_tx_counter < usart1.txsize);
//            while(uart1_tx_counter);
            break;
        case UART3:
            if (uart3_tx_counter > 0)
                return -1;
            memcpy(uart3.txbuffer, data, size);
            uart3.txsize = size;
            gpio_bit_write(GPIOC, GPIO_UART3_DERE, SET);
//            usart_interrupt_enable(usart_periph, USART_INT_TBE);
//            while(uart3_tx_counter < uart3.txsize);
//            while(uart3_tx_counter);
            break;
        case UART4:
            if (uart4_tx_counter > 0)
                return -1;
            memcpy(uart4.txbuffer, data, size);
            uart4.txsize = size;
            gpio_bit_write(GPIOC, GPIO_UART4_DERE, SET);
//            usart_interrupt_enable(usart_periph, USART_INT_TBE);
//            while(uart4_tx_counter < uart4.txsize);
//            while(uart4_tx_counter);
            break;
    }
    usart_interrupt_enable(usart_periph, USART_INT_TBE);
    return 0;
}

int usart_recv_pre(uint32_t usart_periph, uint8_t size)
{
    // TODO: 确定中断需要接收的数据大小
    switch (usart_periph)
    {
        case USART0:
            /* 如果上一次没有读完，返回-1 */
            if (uart0_rx_counter)
                return -1;
            usart0.rxsize = size;
            gpio_bit_write(GPIOC, GPIO_USART0_DERE, RESET);
            break;
        case USART1:
            if (uart1_rx_counter)
                return -1;
            usart1.rxsize = size;
            gpio_bit_write(GPIOC, GPIO_USART1_DERE, RESET);
            break;
        case UART3:
            if (uart3_rx_counter)
                return -1;
            uart3.rxsize = size;
            gpio_bit_write(GPIOC, GPIO_UART3_DERE, RESET);
            break;
        case UART4:
            if (uart4_rx_counter)
                return -1;
            uart4.rxsize = size;
            gpio_bit_write(GPIOC, GPIO_UART4_DERE, RESET);
            break;
    }
    usart_interrupt_enable(usart_periph, USART_INT_RBNE);
    return 0;
}

int usart_recv(uint32_t usart_periph, uint8_t *data)
{
    // TODO: 读取读到的数据，复制到data中
    switch (usart_periph)
    {
        case USART0:
            if (uart0_rx_counter < usart0.rxsize)
                return -1;
            memcpy(data, usart0.rxbuffer, usart0.rxsize);
            usart0.rxsize = 0;
            uart0_rx_counter = 0;
            break;
        case USART1:
            if (uart1_rx_counter < usart1.rxsize)
                return -1;
            memcpy(data, usart1.rxbuffer, usart1.rxsize);
            usart1.rxsize = 0;
            uart1_rx_counter = 0;
            break;
        case UART3:
            if (uart3_rx_counter < uart3.rxsize)
                return -1;
            memcpy(data, uart3.rxbuffer, uart3.rxsize);
            uart3.rxsize = 0;
            uart3_rx_counter = 0;
            break;
        case UART4:
            if (uart4_rx_counter < uart4.rxsize)
                return -1;
            memcpy(data, uart4.rxbuffer, uart4.rxsize);
            uart4.rxsize = 0;
            uart4_rx_counter = 0;
            break;
    }
    return 0;
}

int usart_recv_clear(uint32_t usart_periph)
{
    // TODO: 读取读到的数据，复制到data中
    switch (usart_periph)
    {
        case USART0:
            usart0.rxsize = 0;
            uart0_rx_counter = 0;
            break;
        case USART1:
            usart1.rxsize = 0;
            uart1_rx_counter = 0;
            break;
        case UART3:
            uart3.rxsize = 0;
            uart3_rx_counter = 0;
            break;
        case UART4:
            uart4.rxsize = 0;
            uart4_rx_counter = 0;
            break;
    }
    return 0;
}

/**
 * @function [serial task]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-18T14:42:50+0800
 * @version  [1.0.0]
 * @param    pvParameters             [not used]
 */
void usart_task(void * pvParameters)
{
    for ( ;; ) {

        
    }
}

//void usart_test()
//{
//    int i = 0;
//    uint8_t data[32] = {0};
//    uint8_t rdata[32] = {0};
//    
//    printf("test here \r\n");
//    {
//        /* 初始化发送数据 */
//        for (i = 0; i < 32; i++)
//        {
//            data[i] = 0x30 + i;
//        }

//        while(usart_recv_pre(EVAL_COM4, 6) < 0);

//        while(usart_send(EVAL_COM1, (uint8_t *) data, 6) < 0);

//        while (usart_recv(EVAL_COM4, rdata) < 0)
//        {
//            ;
//        }

//        printf("test pass \r\n");
//    }
//   
//}

void usart_init(void)
{
    rcu_periph_clock_enable(RCU_AF);
    /* configure EVAL_COM0 */
    gd_eval_com_init(EVAL_COM0);
    gd_eval_com_init(EVAL_COM1);
    gd_eval_com_init(EVAL_COM3);
    gd_eval_com_init(EVAL_COM4);

    /* USART interrupt configuration */
    nvic_irq_enable(USART0_IRQn, 0, 0);
    nvic_irq_enable(USART1_IRQn, 0, 0);
    nvic_irq_enable(UART3_IRQn, 0, 0);
    nvic_irq_enable(UART4_IRQn, 0, 0);
    
    /* 关闭所有串口中断 */
    usart_interrupt_disable(USART0, USART_INT_TBE);
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    usart_interrupt_disable(USART1, USART_INT_TBE);
    usart_interrupt_disable(USART1, USART_INT_RBNE);
    usart_interrupt_disable(UART3, USART_INT_TBE);
    usart_interrupt_disable(UART3, USART_INT_RBNE);
    usart_interrupt_disable(UART4, USART_INT_TBE);
    usart_interrupt_disable(UART4, USART_INT_RBNE);

    /* 配置 DERE 脚 */
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_USART0_DERE);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_USART1_DERE);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_UART3_DERE);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_UART4_DERE);
    
    /* 拉低读有效 */
    gpio_bit_write(GPIOC, GPIO_USART0_DERE, SET);
    gpio_bit_write(GPIOC, GPIO_USART1_DERE, SET);
    gpio_bit_write(GPIOC, GPIO_UART3_DERE, SET);
    gpio_bit_write(GPIOC, GPIO_UART4_DERE, SET);
//    usart_test();
}
