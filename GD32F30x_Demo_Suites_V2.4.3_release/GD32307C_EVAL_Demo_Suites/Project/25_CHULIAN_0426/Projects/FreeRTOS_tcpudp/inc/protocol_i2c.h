#ifndef _PROTOCOL_I2C_H_
#define _PROTOCOL_I2C_H_
#include "gd32f30x.h"

#define I2C_OK                 0
#define I2C_FAIL               1
#define I2C0_SPEED             400000
#define I2C0_SLAVE_ADDRESS7    0xA0
#define I2C_PAGE_SIZE          8

void i2c_config(void);
int i2c_register_read(uint8_t device_address, uint8_t* p_buffer, uint8_t read_address, uint16_t number_of_byte);
int i2c_register_write_byte(unsigned char device_address, unsigned char p_buffer, unsigned char wirte_address);

#endif
