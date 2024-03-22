#include <stdio.h>
#include <string.h>
#include "gd32f30x.h"
#include "app_ota.h"

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
#define FMC_PAGE_SIZE           ((uint16_t)0x800)

#define OTA_HEAD_ADDR_H         0
#define OTA_HEAD_ADDR_L         1
#define OTA_INDEX_ADDR_H        2
#define OTA_INDEX_ADDR_L        3
#define OTA_LENGTH_ADDR_H       4
#define OTA_LENGTH_ADDR_L       5
#define OTA_CHECKSUM_ADDR       6

#define OTA_FIRMWARE_ADDR       7

#define OTA_VERSION_ADDR0       7
#define OTA_VERSION_ADDR1       8
#define OTA_VERSION_ADDR2       9
#define OTA_VERSION_ADDR3       10
#define OTA_TOTAL_LENGTH_ADDR0  11
#define OTA_TOTAL_LENGTH_ADDR1  12
#define OTA_TOTAL_LENGTH_ADDR2  13
#define OTA_TOTAL_LENGTH_ADDR3  14
#define OTA_CRC_ADDR0           15
#define OTA_CRC_ADDR1           16

#define SettingSize             0x800           // 2K
#define SettingAddress          0x08001800      //
#define ApplicationAddress0     0x08002000      //
#define ApplicationAddress1     0x08021000      //
#define ApplicationSize         0x1f000         // 124K

// OTA 数据包解析
// AA 55    00 00   XX XX      XX                  ......
// 头数据 |  序号  |  长度  |  固件数据的checksum  | 固件数据
//
// index0 存放 版本信息 总长度 crc校验码
// index1 存放 392 字节固件
static uint16_t ota_index = 0;
static uint16_t ota_length = 0;
// static uint32_t ota_version = 0;
static uint32_t ota_total_length = 0;
static uint16_t ota_crc = 0;
static uint32_t ota_write_addr = 0;
// static uint32_t ota_left_length = 0;

/**
 * @function [写flash程序]
 * @Author   xujh@deepano.com
 * @DataTime 2024-01-03T17:40:32+0800
 * @version  [1.0.0]
 * @param    WRITE_START_ADDR         [写flash的首地址]
 * @param    Size                     [写flash的大小]
 * @param    data                     [要写入flash]
 * @return                            [写flash状态]
 */
TestStatus FLASH_Program(uint32_t WRITE_START_ADDR, uint16_t Size, uint32_t * data)
{
    uint32_t Address;
    TestStatus TransferStatus = FAILED;
    uint32_t i;
    TransferStatus = PASSED;
    /* Unlock the Flash Bank1 Program Erase controller */
    fmc_unlock();

    /* Clear All pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);


    /* Program Flash Bank1 */
    Address = WRITE_START_ADDR;
    i = 0;
    while (Address < (WRITE_START_ADDR + Size))
    {
        fmc_word_program(Address, data[i]);
        i++;
        Address = Address + 4;
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    }

    fmc_lock();

    return TransferStatus;
}

/**
 * @function [计算crc]
 * @Author   xujh@deepano.com
 * @DataTime 2024-01-03T19:10:37+0800
 * @version  [1.0.0]
 * @param    START_ADDR               [crc计算地址]
 * @param    Size                     [flash size]
 * @param    CRC_Value                [crc值]
 * @return                            [crc计算结果]
 */
TestStatus CRC_FLASH(uint32_t START_ADDR, uint32_t Size, uint16_t CRC_Value)
{
    uint8_t *p = (uint8_t*)START_ADDR;
    uint32_t len = Size / 4 - 1;
    uint8_t temp[4] = {0};
    rcu_periph_clock_enable(RCU_PMU);
    rcu_periph_clock_enable(RCU_CRC);
    CRC_CTL = CRC_CTL_RST;
    do
    {
        temp[0] = *(p + 3);
        temp[1] = *(p + 2);
        temp[2] = *(p + 1);
        temp[3] = *p;
        CRC_DATA = * (uint32_t *)temp;
        p = p + 4;
    }
    while (len--);
    if ((CRC_DATA & 0xFFFF) != (uint32_t)CRC_Value)
    {
        return FAILED;
    }
    else
    {
        return PASSED;
    }
}

/**
 * @function [读内存]
 * @Author   xujh@deepano.com
 * @DataTime 2024-01-03T17:44:23+0800
 * @version  [1.0.0]
 * @param    address                  [地址]
 * @param    data                     [数据目标地址]
 * @param    data_num                 [数据大小]
 */
// void read_memory(uint32_t address, uint8_t *data, uint8_t data_num)
// {
//     uint8_t i;
//     uint8_t *p = (uint8_t *)address;
//     for (i = 0; i < data_num; i++) {
//         data[i] = *p;
//         p++;
//     }
// }

// void read_fmc_pid(uint8_t *data)
// {
//     uint32_t temp = FMC_PID;
//     data[0] = (uint8_t)(temp & 0XFF);
//     data[1] = (uint8_t)((temp >> 8) & 0XFF);
//     data[2] = (uint8_t)((temp >> 16) & 0XFF);
//     data[3] = (uint8_t)((temp >> 24) & 0XFF);
// }

void erase_page(uint16_t num_pages, uint16_t page_num)
{
    uint16_t i;

    uint32_t base_address = 0x08000000;
    uint32_t page_address = base_address + (FMC_PAGE_SIZE * page_num);
    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    for (i = 0; i < num_pages; i++) {
        fmc_page_erase(page_address);
        page_address = page_address + FMC_PAGE_SIZE;
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
    }
}

ErrStatus gd_check_sum(uint8_t *data, uint8_t data_num, uint8_t check_data)
{
    uint8_t check_sum = 0;
    uint8_t *p = data;
    uint8_t i;
    if (1 == data_num) {
        check_sum = ~(*p);
    } else {
        for (i = 0; i < data_num; i++) {
            check_sum ^= *p;
            p++;
        }
    }
    if (check_sum == check_data) {
        return SUCCESS;
    } else {
        return ERROR;
    }
}

int handle_otadata(uint8_t *data)
{
    uint16_t ota_index_temp = 0;
    uint16_t ota_package_length = 0;
    if ((data[OTA_HEAD_ADDR_H] == 0xAA) && (data[OTA_HEAD_ADDR_L] == 0x55))
    {
        ota_index_temp = data[OTA_INDEX_ADDR_H] * 256 + data[OTA_INDEX_ADDR_L];
        ota_length = data[OTA_LENGTH_ADDR_H] * 256 + data[OTA_LENGTH_ADDR_L];
        printf("ota_index_temp:%d\r\n", ota_index_temp);
        printf("ota_length:%d\r\n", ota_length);
        /* ota index为0，解析版本信息 总长度 crc校验码 */
        if (ota_index_temp == 0x00)
        {
            // ota_version = (data[OTA_VERSION_ADDR0] << 24) + (data[OTA_VERSION_ADDR1] << 16) + (data[OTA_VERSION_ADDR2] << 8) + data[OTA_VERSION_ADDR3];
            ota_total_length = (data[OTA_TOTAL_LENGTH_ADDR0] << 24) + (data[OTA_TOTAL_LENGTH_ADDR1] << 16) + (data[OTA_TOTAL_LENGTH_ADDR2] << 8) + data[OTA_TOTAL_LENGTH_ADDR3];
            ota_crc = (data[OTA_CRC_ADDR0] << 8) + data[OTA_CRC_ADDR1];
            ota_index = 0;
            printf("version:%d.%d.%d.%d\r\n", data[OTA_VERSION_ADDR0], data[OTA_VERSION_ADDR1], data[OTA_VERSION_ADDR2], data[OTA_VERSION_ADDR3]);
            printf("ota_total_length:%d\r\n", ota_total_length);
            printf("ota_crc:%d\r\n", ota_crc);
            // TODO: 判断要写入的代码位置，擦除flash
            if ((*(__IO uint32_t*)SettingAddress) == 0xffffffff) // 设置寄存器位置没有写入数据，擦除ApplicationAddress1
            {
                erase_page(62, 66);
                ota_write_addr = ApplicationAddress1;
            }
            else if ((*(__IO uint32_t*)SettingAddress) == 0x00000000)
            {
                erase_page(62, 4);
                ota_write_addr = ApplicationAddress0;
            }
        }
        else if (ota_index_temp == (ota_index + 1)) // 确认ota分段数据是连续的
        {
            // 解析此次包长
            ota_package_length = (data[OTA_LENGTH_ADDR_H] << 8) + data[OTA_LENGTH_ADDR_L];
            printf("ota_package_length:%d\r\n", ota_package_length);
            // 检查这次数据是否传输正确
            if (gd_check_sum(&data[OTA_FIRMWARE_ADDR], ota_package_length, data[OTA_CHECKSUM_ADDR]))
            {
                // TODO: 开始写入固件数据
                printf("%s\r\n", "start write ota data");
                FLASH_Program(ota_write_addr + ota_index * 392, ota_package_length, (uint32_t *)&data[OTA_FIRMWARE_ADDR]);
                ota_index = ota_index_temp;
            }
            else
            {
                printf("%s\r\n", "error checksum");
                return 0;
            }
        }
        else // ota数据包丢失
        {
            printf("%s %d\r\n", "ota lost", ota_index);
            memset(data, 0, 400);
            data[0] = 0x55;
            data[1] = 0xAA;
            data[2] = 0x01;
            return 0;
        }
        memset(data, 0, 400);
        data[0] = 0x55;
        data[1] = 0xAA;
        data[2] = 0x00;
        return 1;
    }
    return 2;
}
