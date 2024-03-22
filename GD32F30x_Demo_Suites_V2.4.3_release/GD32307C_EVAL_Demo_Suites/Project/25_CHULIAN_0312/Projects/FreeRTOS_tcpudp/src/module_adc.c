#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "gd32f30x_i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "module_adc.h"
#include "protocol_i2c.h"


#define I2C_SLAVE_ADDRESS 0xD0

//void i2c_master_send_data(uint8_t address, uint8_t data)
//{
//    i2c_register_write_byte(address, data, 0);
//}

int i2c_read_adc_data(uint8_t device_address, uint8_t* p_buffer, uint16_t number_of_byte)
{
    uint32_t timeout = 0xFFFFF;
    /* wait until I2C bus is idle */
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

//    if (2 == number_of_byte) {
//        i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
//    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);

    /* wait until SBSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, device_address, I2C_RECEIVER);

//    if (number_of_byte < 3) {
//        /* disable acknowledge */
//        i2c_ack_config(I2C0, I2C_ACK_DISABLE);
//    }

    /* wait until ADDSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

//    if (1 == number_of_byte) {
//        /* send a stop condition to I2C bus */
//        i2c_stop_on_bus(I2C0);
//    }

    /* while there is data to be read */
    while (number_of_byte) {
        if (3 == number_of_byte) {
            /* wait until BTC bit is set */
            while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
                timeout --;
                if (timeout == 0) {
                    return -1;
                }
            }
            timeout = 0xFFFFF;

            /* disable acknowledge */
            i2c_ack_config(I2C0, I2C_ACK_DISABLE);
        }
        if (2 == number_of_byte) {
            /* wait until BTC bit is set */
            while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
                timeout --;
                if (timeout == 0) {
                    return -1;
                }
            }
            timeout = 0xFFFFF;

            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C0);
        }

        /* wait until the RBNE bit is set and clear it */
        if (i2c_flag_get(I2C0, I2C_FLAG_RBNE)) {
            /* read a byte from the ES8311 */
            *p_buffer = i2c_data_receive(I2C0);

            /* point to the next location where the byte read will be saved */
            p_buffer++;

            /* decrement the read bytes counter */
            number_of_byte--;
        }
    }

    /* wait until the stop condition is finished */
    while (I2C_CTL0(I2C0) & 0x0200) {
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
    return 1;
}


int adc_process_cmd(int cmd_id, int * param, int paramLen)
{
    uint8_t buf[9] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
//    uint16_t hexadc_data;
    int ret;
    ret = i2c_read_adc_data(I2C_SLAVE_ADDRESS, buf, sizeof(buf));

    if (ret == 1)
    {
//        printf("adc buf:%02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
        param[0] = ((uint16_t)buf[1] << 8) | buf[2];
//        printf("adc0 data:%d\r\n", param[0]);
        param[1] = ((uint16_t)buf[3] << 8) | buf[4];
//        printf("adc1 data:%d\n", param[1]);
        param[2] = ((uint16_t)buf[5] << 8) | buf[6];
//        printf("adc2 data:%d\r\n", param[2]);
        param[3] = ((uint16_t)buf[7] << 8) | buf[8];
//        printf("adc3 data:%d\r\n", param[3]);
    }
    else
    {
//        printf("read adc data error\n");
    }
    return 0;
}

