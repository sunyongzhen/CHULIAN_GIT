/*!
    \file  main.c
    \brief The main function file
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-06, V1.0.0, firmware for GD32F3x0
*/

#include "gd32f30x.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gd32f30x_it.h"
#include "main.h"

void gpio_config(void);
typedef  void (*pFunction)(void);
#define SettingSize             0x800           // 2K
#define SettingAddress          0x08001800      //
#define ApplicationAddress0     0x08002000      //
#define ApplicationAddress1     0x08021000      //
#define ApplicationSize         0x1f000         // 124K

#define FMC_PAGE_SIZE           ((uint16_t)0x800)
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;


pFunction Jump_To_Application;
uint32_t JumpAddress = 0;





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

static uint8_t flash_data[2*1024];
int copy_flash()
{
    int count = 62;
    erase_page(62, 4);
    for (count = 0; count < 62; count++)
    {
        memcpy(flash_data, (void *)(ApplicationAddress1 + 0x800 * count), 0x800);
        FLASH_Program(ApplicationAddress0 + 0x800 * count, 0x800, (uint32_t *)flash_data);
    }
}






/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
     gpio_config();
    while (1)
    {
        if (1)//(SET != gpio_input_bit_get(GPIOC, GPIO_PIN_8))
        {
            /* SettingAddress中存放加载标志位：APP是APP0 或 APP1 */
            if ((*(__IO uint32_t*)SettingAddress) == 0xffffffff)
            {
                /* 配置地址未设置值 */
                /* 检查两个APP address栈顶地址是否合法 */
//                if (((*(__IO uint32_t*)ApplicationAddress1) & 0x2FFE0000 ) == 0x20000000)
//                {
//                    JumpAddress = *(__IO uint32_t*) (ApplicationAddress1 + 4);
//                    Jump_To_Application = (pFunction) JumpAddress;
//                    __set_MSP(*(__IO uint32_t*) ApplicationAddress1);
//                    Jump_To_Application();
//                }
//                
//                copy_flash();
                if (((*(__IO uint32_t*)ApplicationAddress0) & 0x2FFE0000 ) == 0x20000000)
                {
                    JumpAddress = *(__IO uint32_t*) (ApplicationAddress0 + 4);
                    Jump_To_Application = (pFunction) JumpAddress;
                    __set_MSP(*(__IO uint32_t*) ApplicationAddress0);
                    Jump_To_Application();
                }
            }
            else
            {
                copy_flash();
                if (((*(__IO uint32_t*)ApplicationAddress0) & 0x2FFE0000 ) == 0x20000000)
                {
                    JumpAddress = *(__IO uint32_t*) (ApplicationAddress0 + 4);
                    Jump_To_Application = (pFunction) JumpAddress;
                    __set_MSP(*(__IO uint32_t*) ApplicationAddress0);
                    Jump_To_Application();
                }
//                else 
//                if (((*(__IO uint32_t*)ApplicationAddress1) & 0x2FFE0000 ) == 0x20000000)
//                {
//                    JumpAddress = *(__IO uint32_t*) (ApplicationAddress1 + 4);
//                    Jump_To_Application = (pFunction) JumpAddress;
//                    __set_MSP(*(__IO uint32_t*) ApplicationAddress1);
//                    Jump_To_Application();
//                }
            }
        }
    }

    while (1) {}
}

void gpio_config(void)
{
	
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
}
