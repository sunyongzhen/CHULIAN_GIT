#ifndef _MODULE_ADC_H
#define _MODULE_ADC_H
#include <stdint.h>

//void i2c_master_send_data(uint8_t address, uint8_t data);
//int i2c_read_adc_data(uint8_t device_address, uint8_t* p_buffer, uint16_t number_of_byte);
int adc_process_cmd(int cmd_id, int * param, int paramLen);

#endif
