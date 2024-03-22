#include "gd32f30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "module_display.h"

static int ctrl_cmd;
static int ctrl_data;
static TaskHandle_t xDisplayHandle;
struct module
{
    const char *name;   // 设备名称
    TaskHandle_t * xHandle;
    struct {
        void *data;
        void *ctrl;
    } reg;
};

struct module display_dev = {
    "lcd",
    &xDisplayHandle,
    {
        &ctrl_data,
        &ctrl_cmd,
    },
};

static void lcd_gpio_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);

    /* configure EXMC_D[0~7]*/
    /* PD14(EXMC_D0), PD15(EXMC_D1),PD0(EXMC_D2), PD1(EXMC_D3) */
    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PE7(EXMC_D4), PE8(EXMC_D5), PE9(EXMC_D6), PE10(EXMC_D7) */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

    /* configure PE11(RS) */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

    /* configure PE12(RW) */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);

    /* configure PE15(EN) */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

}

static void lcd_gpio_data_in(void)
{
    //配置引脚为浮空输入
    /* PD14(EXMC_D0), PD15(EXMC_D1),PD0(EXMC_D2), PD1(EXMC_D3) */
    gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PE7(EXMC_D4), PE8(EXMC_D5), PE9(EXMC_D6), PE10(EXMC_D7) */
    gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
}

static void lcd_gpio_data_out(void)
{
    /* PD14(EXMC_D0), PD15(EXMC_D1),PD0(EXMC_D2), PD1(EXMC_D3) */
    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_14 | GPIO_PIN_15);

    /* PE7(EXMC_D4), PE8(EXMC_D5), PE9(EXMC_D6), PE10(EXMC_D7) */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
}

static void LCD_Data_Bus_Out(unsigned char data_val)
{
    gpio_bit_write(GPIOD, GPIO_PIN_14, (bit_status)((data_val >> 0) & 0x01));
    gpio_bit_write(GPIOD, GPIO_PIN_15, (bit_status)((data_val >> 1) & 0x01));
    gpio_bit_write(GPIOD, GPIO_PIN_0, (bit_status)((data_val >> 2) & 0x01));
    gpio_bit_write(GPIOD, GPIO_PIN_1, (bit_status)((data_val >> 3) & 0x01));
    gpio_bit_write(GPIOE, GPIO_PIN_7, (bit_status)((data_val >> 4) & 0x01));
    gpio_bit_write(GPIOE, GPIO_PIN_8, (bit_status)((data_val >> 5) & 0x01));
    gpio_bit_write(GPIOE, GPIO_PIN_9, (bit_status)((data_val >> 6) & 0x01));
    gpio_bit_write(GPIOE, GPIO_PIN_10, (bit_status)((data_val >> 7) & 0x01));
}

static void wait_busy(void)
{
    lcd_gpio_data_in();

    gpio_bit_write(GPIOE, GPIO_PIN_11, RESET);
    gpio_bit_write(GPIOE, GPIO_PIN_12, SET);
    gpio_bit_write(GPIOE, GPIO_PIN_15, SET);
    while (gpio_input_bit_get(GPIOE, GPIO_PIN_10));
    gpio_bit_write(GPIOE, GPIO_PIN_15, RESET);

    lcd_gpio_data_out();
}

static void Write_Command(unsigned char CmdData)
{
    wait_busy();
    gpio_bit_write(GPIOE, GPIO_PIN_11, RESET);
    gpio_bit_write(GPIOE, GPIO_PIN_12, RESET);
    gpio_bit_write(GPIOE, GPIO_PIN_15, SET);
    LCD_Data_Bus_Out(CmdData);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_bit_write(GPIOE, GPIO_PIN_15, RESET);
    vTaskDelay(pdMS_TO_TICKS(1));
}

static void Write_Data(unsigned char Data)
{
    wait_busy();
    gpio_bit_write(GPIOE, GPIO_PIN_11, SET);
    gpio_bit_write(GPIOE, GPIO_PIN_12, RESET);
    gpio_bit_write(GPIOE, GPIO_PIN_15, SET);
    LCD_Data_Bus_Out(Data);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_bit_write(GPIOE, GPIO_PIN_15, RESET);
    vTaskDelay(pdMS_TO_TICKS(1));
}

static void clear_screen(void)
{
    Write_Command(0x01);
}

static void st7920_init(void)
{
    lcd_gpio_init();    // LCD所有的引脚初始化
    Write_Command(0x30);// 功能设定，8bit控制
    vTaskDelay(pdMS_TO_TICKS(2));
    Write_Command(0x064);// 指定资料写入,ACC地址横向加1
    vTaskDelay(pdMS_TO_TICKS(2));
    Write_Command(0x0c);// 开显示，关光标，不闪烁
    vTaskDelay(pdMS_TO_TICKS(2));
    Write_Command(0x80);// 将DDRAM地址计数器AC设置为0
    vTaskDelay(pdMS_TO_TICKS(2));
}

#if defined DRAW_PICTURE
void update_display_screen(void)
{
    unsigned char row, i, j;
    const unsigned char * ptr = display_buffer;
    Write_Command(0x34);
    for (i = 0; i < 2; i++)
    {
        for (row = 0; row < 32; row++)
        {
            Write_Command(0x80 + row);//先写行地址
            Write_Command(0x80 + 8 * i);    //再写列地址
            for (j = 0; j < 16; j++)
            {
                Write_Data(*ptr++);
            }
        }
    }
    Write_Command(0x36);
}
#endif

void lcd_display(unsigned char row, unsigned char col, const char *string)
{
    switch (row)
    {
    case 1: row = 0x80; break;
    case 2: row = 0x90; break;
    case 3: row = 0x88; break;
    case 4: row = 0x98; break;
    default: row = 0x80;
    }
    Write_Command(row + col);
    while (*string != '\0')
    {
        Write_Data(*string++);
    }
}

/**
 * @function [init lcd device]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:09:12+0800
 * @version  [1.0.0]
 * @return                            [deal result]
 */
const char dhlr[] = {0xb6, 0xaf, 0xbb, 0xf0, 0xc0, 0xeb, 0xc8, 0xcb, ':', 0x00};
const char wd[] = {0xce, 0xc2, 0xb6, 0xc8, ':', 0x00};
const char rq[] = {0xC8, 0xBC, 0xC6, 0xF8, ':', 0x00};
const char yw[] = {0xD1, 0xCC, 0xCE, 0xED, ':', 0x00};
const char dhlr_end[] = {' ', '0', '0', '0', 0xC3, 0xEB, 0x00};
const char wd_end[] = {' ', '0', '0', '0', '.', '0', 0xA1, 0xE6, 0x00};
const char rq_end[] = {'0', '0', '.', '0', '0', '%', 0x00};
const char yw_end[] = {'0', '0', '.', '0', '0', '%', 0x00};
int display_init(void)
{
    st7920_init();
    clear_screen();
    // update_display_screen();
    lcd_display(1, 0, dhlr);
    lcd_display(1, 5, dhlr_end);
    lcd_display(2, 1, wd);
    lcd_display(2, 4, wd_end);
    lcd_display(3, 1, rq);
    lcd_display(3, 5, rq_end);
    lcd_display(4, 1, yw);
    lcd_display(4, 5, yw_end);
    
    // 动火离人3 温度2 燃气1 烟雾0
    return 0;
}

/**
 * @function [deal lcd commands]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:03:08+0800
 * @version  [1.0.0]
 * @param    cmd_id                   [command id]
 * @param    param                    [param]
 * @param    paramLen                 [param length]
 * @return                            [deal result]
 */
int display_process_cmd(int cmd_id, int * param, int paramLen)
{
    if (cmd_id == 1)
    {
        clear_screen();
    }
    else if (cmd_id == 2)
    {
        lcd_display(1, 0,  "1111111122222222333333334444444455555555666666667777777788888888");
    }
    else {
        printf("command error\r\n");
    }
    return 0;
}
