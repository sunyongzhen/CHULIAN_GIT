#include "protocol_modbus.h"
#include <stdio.h>

/**
 * 功能码      名称              数据类型                            作用
 * 0x01     读线圈寄存器            位                       取得一组逻辑线圈的当前状态（ON/OFF )
 * 0x02     读离散输入寄存器        位                       取得一组开关输入的当前状态（ON/OFF )
 * 0x03     读保持寄存器          整型、浮点型、字符型          在一个或多个保持寄存器中取得当前的二进制值
 * 0x04     读输入寄存器          整型、浮点型、字符型          在一个或多个输入寄存器中取得当前的二进制值
 * 0x05     写单个线圈寄存器        位                       强置一个逻辑线圈的通断状态
 * 0x06     写单个保持寄存器        整型、浮点型、字符型        把具体二进值装入一个保持寄存器
 * 0x0f     写多个线圈寄存器        位                       强置一串连续逻辑线圈的通断
 * 0x10     写多个保持寄存器        整型、浮点型、字符型        把具体的二进制值装入一串连续的保持寄存器
 */

/**
 * 本应用针对烟雾，人体，温度，燃气传感器，不考虑功能码0x0f 0x10，只实现0x03 0x06，且为了节省空间最多只接收16字节数据
 */

#define SWAP_BYTES(x) (((x) << 8) | ((x) >> 8)) 
#define POLY 0xA001

ModbusRtuFrame      frame_send = {0};
ModbusRtuFrameAck   frame_recv = {0};

/**
 * @function [计算冗余码]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-25T14:38:23+0800
 * @version  [1.0.0]
 * @param    data                     [计算的字节数组首地址]
 * @param    len                      [数组长度]
 * @return                            [crc结果]
 */
unsigned short crc16_modbus(unsigned char *data, unsigned char len)
{
    unsigned short crc = 0xFFFF;
    unsigned char i;
    unsigned char j;
    for (i = 0; i < len; i++)
    {
        crc ^= (unsigned short)data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= POLY;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @function [打包发送的modbus帧]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-25T14:45:03+0800
 * @version  [1.0.0]
 * @param    address                  [从机地址]
 * @param    function_code            [功能码]
 * @param    start_address            [寄存器起始地址]
 * @param    number_of_registers      [寄存器数量]
 * @return                            [modbus发送地址]
 */
ModbusRtuFrame * packed_modbus_rtu_frame(uint8_t address, uint8_t function_code, uint16_t start_address, uint16_t number_of_registers) {
    ModbusRtuFrame * frame = &frame_send;
    // 设置从机地址
    frame->slave_address = address;
    // 设置功能码
    frame->function_code = function_code;
    // 设置起始地址和寄存器数量
    frame->start_address = SWAP_BYTES(start_address);
    frame->number_of_registers = SWAP_BYTES(number_of_registers);
    // 计算并设置CRC校验码
    frame->crc = crc16_modbus((uint8_t *)frame, 6);//sizeof(ModbusRtuFrame) - 2);
    return frame;
}

/**
 * @function [返回接收modbus应答的地址]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-25T15:45:12+0800
 * @version  [1.0.0]
 * @return                            [modbus应答地址]
 */
ModbusRtuFrameAck * get_modbus_rtu_ackframe_addr(void) {
    return &frame_recv;
}

/**
 * @function [解析接收到的modbus帧]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-25T15:36:58+0800
 * @version  [1.0.0]
 * @param    frame                    [frame地址]
 */
int parse_modbus_rtu_frame(ModbusRtuFrameAck *frame) {
    uint8_t address, function_code, byte_count;
    uint8_t *data;
    uint16_t calculated_crc;
//    uint8_t i;

    if (frame == NULL) {
        printf("recv frame not found\r\n");
        return -1;
    }
    // 获取从机地址
    address = frame->slave_address;
//    printf("addr :%02x\r\n", address);

    // 获取功能码
    function_code = frame->function_code;
//    printf("function_code :%02x\r\n", function_code);

    // 获取字节数
    byte_count = frame->byte_count;
    if (byte_count > MAX_BYTES) {
        printf("byte_count overflow:%d\r\n", byte_count);
        return -1;
    }
//    printf("byte_count :%02x\r\n", byte_count);
    // 获取crc码
    frame->crc = frame->data[2 * byte_count] + frame->data[2 * byte_count + 1] * 256;
//    printf("frame->crc :%04x\r\n", frame->crc);
    // 解析数据
    data = (uint8_t *)frame;
    // 解析CRC校验码
    calculated_crc = crc16_modbus(data, 3 + byte_count * 2);
    if (calculated_crc == frame->crc) {
        // 校验正确，解析数据

        // 处理解析后的数据，例如：打印寄存器值
//        for (i = 0; i < byte_count * 2; ++i)
//        {
//            printf("Registers[%d] parsed: %02x\n", i, frame->data[i]);
//        }
    } else {
        printf("CRC error: %04x\n", calculated_crc);
        return -1;
    }
    return 0;
}
