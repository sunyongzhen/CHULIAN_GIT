#include <stdio.h>
#include "gd32f30x.h"
#include "protocol_i2c.h"

void i2c_config(void)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* connect PB6 to I2C0_SCL */
    /* connect PB7 to I2C0_SDA */
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    /* enable I2C clock */
    rcu_periph_clock_enable(RCU_I2C0);
    /* configure I2C clock */
    i2c_clock_config(I2C0, I2C0_SPEED, I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2C0_SLAVE_ADDRESS7);
    /* enable I2C0 */
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

int i2c_register_write_byte(unsigned char device_address, unsigned char p_buffer, unsigned char wirte_address)
{
    uint32_t timeout = 0xFFFF;
    /* wait until I2C bus is idle */
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);

    /* wait until SBSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, device_address, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while ( SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send the CODEC's internal address to write to : only one byte address */
    i2c_data_transmit(I2C0, wirte_address);

    /* wait until BTC bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* while there is data to be written */

    i2c_data_transmit(I2C0, p_buffer);

    /* wait until BTC bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C0);

    /* wait until the stop condition is finished */
    while (I2C_CTL0(I2C0) & 0x0200){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    
    return 0;
}

int i2c_register_read(uint8_t device_address, uint8_t* p_buffer, uint8_t read_address, uint16_t number_of_byte)
{
    uint32_t timeout = 0xFFFFF;
    /* wait until I2C bus is idle */
    while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    if (2 == number_of_byte) {
        i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);

    /* wait until SBSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, device_address, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while (SET != i2c_flag_get( I2C0 , I2C_FLAG_TBE)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* enable I2C0*/
    i2c_enable(I2C0);

    /* send the CODEC's internal address to write to */
    i2c_data_transmit(I2C0, read_address);

    /* wait until BTC bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);

    /* wait until SBSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, device_address, I2C_RECEIVER);

    if (number_of_byte < 3) {
        /* disable acknowledge */
        i2c_ack_config(I2C0, I2C_ACK_DISABLE);
    }

    /* wait until ADDSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    if (1 == number_of_byte) {
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(I2C0);
    }

    /* while there is data to be read */
    while (number_of_byte) {
        if (3 == number_of_byte) {
            /* wait until BTC bit is set */
            while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
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
            while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)){
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
    while (I2C_CTL0(I2C0) & 0x0200){
        timeout --;
        if (timeout == 0) {
            return -1;
        }
    }
    timeout = 0xFFFFF;

    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
    return 0;
}
