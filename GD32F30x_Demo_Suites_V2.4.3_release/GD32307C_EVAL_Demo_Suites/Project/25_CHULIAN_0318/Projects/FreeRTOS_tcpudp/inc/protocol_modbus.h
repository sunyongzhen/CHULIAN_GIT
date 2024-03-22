#ifndef PROTOCOL_MODBUS_H
#define PROTOCOL_MODBUS_H

#include <stdint.h>

#define MAX_BYTES 16
typedef struct {
    uint8_t slave_address;          // 从机地址
    uint8_t function_code;          // 功能码
    uint16_t start_address;         // 起始地址
    uint16_t number_of_registers;   // 寄存器数量
    uint16_t crc;                   // CRC校验码
} ModbusRtuFrame;

typedef struct {
    uint8_t slave_address;          // 从机地址
    uint8_t function_code;          // 功能码
    uint8_t byte_count;             // 返回寄存器数量
    uint8_t data[MAX_BYTES];        // 数据值
    uint16_t crc;                   // CRC校验码
} ModbusRtuFrameAck;

ModbusRtuFrame * packed_modbus_rtu_frame(uint8_t address, uint8_t function_code, uint16_t start_address, uint16_t number_of_registers);
ModbusRtuFrameAck * get_modbus_rtu_ackframe_addr(void);
int parse_modbus_rtu_frame(ModbusRtuFrameAck *frame);

#endif /* PROTOCOL_MODBUS_H */
