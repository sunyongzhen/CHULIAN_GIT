#include "gd32f30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "module_audio.h"
#include "protocol_i2c.h"
#include "module_gd25qxx.h"

/* I2S configuration parameters */
#define I2S_STANDARD                        I2S_STD_MSB
#define I2S_MCLKOUTPUT                      I2S_MCKOUT_ENABLE

#define ES8311_ADDRESS         0x30
#define MUTE RESET
#define PLAY SET

#define buf_size  1024
static uint32_t audio_addr[7] = {0x00080000, 0x00100000, 0x00180000,0x00200000, 0x00280000, 0x00300000, 0x00380000};
uint16_t buf1[buf_size / 2] = {0};
uint16_t buf2[buf_size / 2] = {0};
uint8_t num = 0;
volatile uint16_t * buf1_p = buf1;
volatile uint16_t * buf2_p = buf2;
volatile uint8_t buf1_read_enable = 0;  //0 能写  1 能读
volatile uint8_t buf2_read_enable = 0;

uint16_t i2s_audio_freq    = I2S_AUDIOSAMPLE_16K;

void set_num(uint8_t n)
{
	num = n;
}

void read_data_from_flash()
{
	uint16_t i = 0;
	uint16_t temp = (audio_addr[num+1] - audio_addr[num]) / buf_size;
	for(i = 0; i < temp; i ++)
	{
		if(buf1_read_enable == 0){  //不能读buf1 能写
			spi_flash_buffer_read((uint8_t *)buf1, audio_addr[num] + i * buf_size - 0x20000, buf_size);
			while(buf2_read_enable);//等buf2 读完
			buf1_read_enable = 1;
			
		}else if(buf2_read_enable == 0){ //不能读buf2 能写
			spi_flash_buffer_read((uint8_t *)buf2, audio_addr[num] + i * buf_size - 0x20000, buf_size);
			while(buf1_read_enable);//等buf1 读完
			buf2_read_enable = 1;
		}
		vTaskDelay(10);
	}
}

struct module
{
    const char *name;   // 设备名称
    uint8_t volume; // SPK音量
};

//struct module es8311 = {"audio", 181};
struct module es8311 = {"audio", 200};

static void beep_gpio_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOD);
    gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_bit_write(GPIOD, GPIO_PIN_12, RESET);
}

static void beep_set(bit_status status)
{
    gpio_bit_write(GPIOD, GPIO_PIN_12, status);
}

/*!
    \brief      configure the I2S1 and SPI0 GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_i2s_gpio_config(void)
{
    /* enable the GPIO clock */
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
    gpio_pin_remap_config(GPIO_SPI2_REMAP, DISABLE);

    /* configure I2S2 pins: PA15(I2S_WS), PB3(I2S_CK), PB5(I2S_DIN) */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);

    /* PC7(I2S_MCK) */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
}

/*!
    \brief      configure the I2S and SPI0
    \param[in]  standard: I2S standard
      \arg        I2S_STD_PHILLIPS: I2S phillips standard
      \arg        I2S_STD_MSB: I2S MSB standard
      \arg        I2S_STD_LSB: I2S LSB standard
      \arg        I2S_STD_PCMSHORT: I2S PCM short standard
      \arg        I2S_STD_PCMLONG: I2S PCM long standard
     \param[in]  mclk_output: I2S master clock output
      \arg        I2S_MCKOUT_ENABLE: I2S master clock output enable
      \arg        I2S_MCKOUT_DISABLE: I2S master clock output disable
     \param[in]  audio_freq: I2S audio sample rate
      \arg        I2S_AUDIOSAMPLE_8K: audio sample rate is 8KHz
      \arg        I2S_AUDIOSAMPLE_11K: audio sample rate is 11KHz
      \arg        I2S_AUDIOSAMPLE_16K: audio sample rate is 16KHz
      \arg        I2S_AUDIOSAMPLE_22K: audio sample rate is 22KHz
      \arg        I2S_AUDIOSAMPLE_32K: audio sample rate is 32KHz
      \arg        I2S_AUDIOSAMPLE_44K: audio sample rate is 44KHz
      \arg        I2S_AUDIOSAMPLE_48K: audio sample rate is 48KHz
      \arg        I2S_AUDIOSAMPLE_96K: audio sample rate is 96KHz
      \arg        I2S_AUDIOSAMPLE_192K: audio sample rate is 192KHz
    \param[out] none
    \retval     none
*/
void spi_i2s_config(uint16_t standard, uint16_t mclk_output, uint16_t audio_freq)
{
//    spi_parameter_struct spi_init_struct;

    /* enable SPI clock */
    rcu_periph_clock_enable(RCU_SPI2);

    {
        /* deinitialize SPI2 peripheral */
        spi_i2s_deinit(SPI2);
        /* I2S2 peripheral configuration */
        i2s_init(SPI2, I2S_MODE_MASTERTX, standard, I2S_CKPL_HIGH);
        i2s_psc_config(SPI2, audio_freq, I2S_FRAMEFORMAT_DT16B_CH16B, mclk_output);

        /* disable the I2S2 TBE interrupt */
        spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
        /* enable the SPI2 peripheral */
        i2s_enable(SPI2);
    }
}

static void spk_mute_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOE);
    /* configure spk mute pin PE0 */
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
}

static void spk_mute_set(bit_status status)
{
    gpio_bit_write(GPIOE, GPIO_PIN_0, status);
}

static int codec_register_write_byte(unsigned char wirte_address, unsigned char p_buffer)
{
    return i2c_register_write_byte(ES8311_ADDRESS, p_buffer, wirte_address);
}

//static int codec_register_read(uint8_t* p_buffer, uint8_t read_address, uint16_t number_of_byte)
//{
//    return i2c_register_read(ES8311_ADDRESS, p_buffer, read_address, number_of_byte);
//}

static void es8311_gpio_init()
{
    // 初始化I2S中断
    nvic_irq_enable(SPI2_IRQn, 0, 2);

    /* configure the I2S1 and SPI0 GPIO */
    spi_i2s_gpio_config();
    /* configure the I2S1 and SPI0 */
    spi_i2s_config(I2S_STANDARD, I2S_MCLKOUTPUT, i2s_audio_freq);

    /* configure the SPK MUTE GPIO */
    spk_mute_config();
}

static void es8311_codec(void)
{
    codec_register_write_byte(ES8311_GP_REG45, 0x00);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x30);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x10);

    // Ratio=MCLK/LRCK=256：12M288-48K；4M096-16K; 2M048-8K
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x00);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG03, 0x10);
    codec_register_write_byte(ES8311_ADC_REG16, 0x24);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG04, 0x20);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG05, 0x00);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG06, 0x03);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG07, 0x00);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG08, 0xFF);

    // SDP IN
    codec_register_write_byte(ES8311_SDPIN_REG09, DACChannelSel + NORMAL_I2S + Format_Len16);
    // SDP OUT IIS 16BIT
    codec_register_write_byte(ES8311_SDPOUT_REG0A, NORMAL_I2S + Format_Len16);
    // Default
    codec_register_write_byte(ES8311_SYSTEM_REG0B, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0C, 0x00);

    // Unknow
    codec_register_write_byte(ES8311_SYSTEM_REG10, 0x1F + VDDA_VOLTAGE);
    codec_register_write_byte(ES8311_SYSTEM_REG11, 0x7F);

    // Slave  Mode
    codec_register_write_byte(ES8311_RESET_REG00, 0x80 + MSMode);

    // Unknow
    codec_register_write_byte(ES8311_SYSTEM_REG0D, 0x01);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x3F + MCLK_SOURCE);

    // ADC Setting
    codec_register_write_byte(ES8311_SYSTEM_REG14, 0x00 + ADC_PGA_GAIN);

    // Enable DAC
    codec_register_write_byte(ES8311_SYSTEM_REG12, 0x00);
    // default for line out drive
    codec_register_write_byte(ES8311_SYSTEM_REG13, 0x00);
    //
    codec_register_write_byte(ES8311_SYSTEM_REG0E, 0x02);
    codec_register_write_byte(ES8311_SYSTEM_REG0F, 0x5B);
    codec_register_write_byte(ES8311_ADC_REG15, 0x00);
    codec_register_write_byte(ES8311_ADC_REG1B, 0x0A);
    codec_register_write_byte(ES8311_ADC_REG1C, 0x6A);

    codec_register_write_byte(ES8311_DAC_REG37, 0x48);
    codec_register_write_byte(ES8311_DAC_REG32, es8311.volume);
}

static void es8311_resume(void)
{
    codec_register_write_byte(ES8311_SYSTEM_REG0D, 0x01);
    codec_register_write_byte(ES8311_GP_REG45, 0x00);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x3F);
    codec_register_write_byte(ES8311_RESET_REG00, 0x80);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x00);

    codec_register_write_byte(ES8311_DAC_REG37, 0x48);
    codec_register_write_byte(ES8311_ADC_REG15, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG14, 0x10);
    codec_register_write_byte(ES8311_SYSTEM_REG12, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0E, 0x02);

    codec_register_write_byte(ES8311_DAC_REG32, es8311.volume);

}

static void es8311_set_volume(unsigned char volume)
{
    es8311.volume = volume;
}

static void es8311_standby()
{
    codec_register_write_byte(ES8311_DAC_REG32, 0x00);
    codec_register_write_byte(ES8311_ADC_REG17, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0E, 0xFF);
    codec_register_write_byte(ES8311_SYSTEM_REG12, 0x02);
    codec_register_write_byte(ES8311_SYSTEM_REG14, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0D, 0xFA);
    codec_register_write_byte(ES8311_ADC_REG15, 0x00);
    codec_register_write_byte(ES8311_DAC_REG37, 0x08);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x10);
    codec_register_write_byte(ES8311_RESET_REG00, 0x00);
    codec_register_write_byte(ES8311_RESET_REG00, 0x1F);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x30);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x00);
    codec_register_write_byte(ES8311_GP_REG45, 0x00);
}

static void es8311_powerdown()
{
	printf("es8311_powerdown\n");
    codec_register_write_byte(ES8311_DAC_REG32, 0x00);
    codec_register_write_byte(ES8311_ADC_REG17, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0E, 0xFF);
    codec_register_write_byte(ES8311_SYSTEM_REG12, 0x02);
    codec_register_write_byte(ES8311_SYSTEM_REG14, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0D, 0xFA);
    codec_register_write_byte(ES8311_ADC_REG15, 0x00);
    codec_register_write_byte(ES8311_DAC_REG37, 0x08);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x10);
    codec_register_write_byte(ES8311_RESET_REG00, 0x00);
    codec_register_write_byte(ES8311_RESET_REG00, 0x1F);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x30);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG01, 0x00);
    codec_register_write_byte(ES8311_GP_REG45, 0x00);
    codec_register_write_byte(ES8311_SYSTEM_REG0D, 0xFC);
    codec_register_write_byte(ES8311_CLK_MANAGER_REG02, 0x00);
}

static void es8311_play()
{
//	uint16_t i;
	spi_flash_buffer_read((uint8_t *)buf1, audio_addr[num] - buf_size, buf_size); //写buf1
	buf1_read_enable = 1;  //buf1 能读
//	for(i=0; i<256; i ++){
//		printf("%x ", buf1[i]);
//		if(15 == i% 16){
//			printf("\r\n");
//		}
//	}
    es8311_resume();
    spi_i2s_interrupt_enable(SPI2, SPI_I2S_INT_TBE);
    spk_mute_set(PLAY);
	read_data_from_flash();
}

static void es8311_mute(void)
{

    spk_mute_set(MUTE);
	printf("spk_mute_set\n");
    es8311_powerdown();
	printf("es8311_powerdown\n");
    spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
	printf("spi_i2s_interrupt_disable\n");
	buf1_read_enable = 0;
	buf2_read_enable = 0;
}


/**
 * @function [deal audio related commands]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:03:08+0800
 * @version  [1.0.0]
 * @param    cmd_id                   [command id]
 * @param    param                    [param]
 * @param    paramLen                 [param length]
 * @return                            [deal result]
 */
int audio_process_cmd(int cmd_id, int * param, int paramLen)
{
    // 0:audiomute, 1:audioplay, 2:audiostandby, 3:setvolume, 4:beepset, 5:beepreset
	
	printf("cmd_id = %d\n", cmd_id);
    switch (cmd_id) {
    case 0: es8311_mute();              break;

    case 1: es8311_play();        		break;

    case 2: es8311_standby();           break;

    case 3: es8311_set_volume(*param);  break;

    case 4: beep_set(SET);              break;

    case 5: beep_set(RESET);            break;

    default:
        printf("%s\r\n", "error command");
        break;
    }
    return 0;
}

/**
 * @function [init audio device]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:09:12+0800
 * @version  [1.0.0]
 * @return                            [deal result]
 */
int audio_init(void)
{
    beep_gpio_init();

    es8311_gpio_init();
    es8311_codec();

    // TEST
//    es8311_play();

//    vTaskDelay(pdMS_TO_TICKS(10000));

//    es8311_mute();

//    vTaskDelay(pdMS_TO_TICKS(10000));

//    es8311_set_volume(181);

//    es8311_play();

//    vTaskDelay(pdMS_TO_TICKS(10000));

//    es8311_mute();

//    beep_set(SET);
    return 0;
}

