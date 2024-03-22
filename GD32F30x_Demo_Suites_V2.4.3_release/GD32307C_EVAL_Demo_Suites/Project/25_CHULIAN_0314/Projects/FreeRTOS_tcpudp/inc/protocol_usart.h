/*!
    \file  protocol_usart.h
    \brief the header file of netconf
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef PROTOCOL_USART_H
#define PROTOCOL_USART_H

#include "stdint.h"

typedef struct uart_trans_buffer
{
    uint8_t txbuffer[32];
    uint8_t rxbuffer[32];
    uint8_t txsize;
    uint8_t rxsize;
} uart_trans_buffer_t;

extern uart_trans_buffer_t usart0;
extern uart_trans_buffer_t usart1;
extern uart_trans_buffer_t uart3;
extern uart_trans_buffer_t uart4;

extern volatile uint8_t uart0_tx_counter;
extern volatile uint8_t uart0_rx_counter;
extern volatile uint8_t uart1_tx_counter;
extern volatile uint8_t uart1_rx_counter;
extern volatile uint8_t uart3_tx_counter;
extern volatile uint8_t uart3_rx_counter;
extern volatile uint8_t uart4_tx_counter;
extern volatile uint8_t uart4_rx_counter;

/* function declarations */
void usart_task(void * pvParameters);
void usart_init(void);
int usart_send(uint32_t usart_periph, uint8_t* data, uint8_t size);
int usart_recv_pre(uint32_t usart_periph, uint8_t size);
int usart_recv(uint32_t usart_periph, uint8_t *data);
int usart_recv_clear(uint32_t usart_periph);

#endif /* PROTOCOL_USART_H */
