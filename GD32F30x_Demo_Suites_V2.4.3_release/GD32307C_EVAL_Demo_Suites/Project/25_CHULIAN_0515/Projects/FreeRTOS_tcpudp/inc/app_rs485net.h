#include <stdio.h>
#include <stdint.h>

//void scan_dev(uint32_t usart_periph);
void cancel_alarm(void);
void rs485net_task_init(void);
void rs485net_task_start(void);
void rs485net_task_stop(void);
int uart0_get_tmperature_data(void);
int uart1_get_tmperature_data(void);
int uart3_get_tmperature_data(void);
int uart4_get_tmperature_data(void);
int get_tmperature_data(void);
void set_camera_status(uint16_t channel, uint16_t status);
