#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "module_led.h"
#include "module_key.h"
#include "module_display.h"
#include "module_audio.h"
#include "module_rtc.h"
#include "module_adc.h"
#include "app_configure.h"
#include "app_rs485net.h"

typedef char (*f_cmd_handler)(const char *param);

typedef struct cmd
{
    const char *cmd;
    f_cmd_handler handler;
} cmd_t;

/* 命令缓存到fifo */
#if 0
#define CACHE_SIZE  3
#define BUF_SIZE    32
typedef struct str_buf
{
    unsigned char data[CACHE_SIZE][BUF_SIZE];
    unsigned char head;
    unsigned char tail;
} str_buf_t;

static str_buf_t input = {0};
static str_buf_t output = {0};

/**
 * @function [初始化buffer]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-19T17:39:33+0800
 * @version  [1.0.0]
 * @param    buf                      [fifo地址]
 */
void buffer_init(str_buf_t * buf)
{
    buf->head = 0;
    buf->tail = 0;
}

/**
 * @function [判断fifo是否已满]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-19T17:44:15+0800
 * @version  [1.0.0]
 * @param    buf                      [fifo地址]
 */
int buffer_is_full(str_buf_t * buf)
{
    return ((buf->tail + 1) % CACHE_SIZE) == buf->head;
}

/**
 * @function [判断fifo是否为空]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-19T17:59:14+0800
 * @version  [1.0.0]
 * @param    buf                      [fifo地址]
 * @return                            [位置重合为1为空]
 */
int buffer_is_empty(str_buf_t * buf)
{
    return buf->head == buf->tail;
}

/**
 * @function [向缓存区写入一条命令]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-19T18:44:40+0800
 * @version  [1.0.0]
 * @param    buf                      [fifo地址]
 * @param    data                     [写入的字符串地址]
 * @return                            [-1已满，0成功]
 */
int buffer_write(str_buf_t * buf, unsigned char * data)
{
    /* 缓存区已满，无法写入 */
    if (buffer_is_full(buf))
    {
        return -1;
    }
    memcpy(buf->data[buf->tail], data, BUF_SIZE);
    buf->tail = (buf->tail + 1) % CACHE_SIZE;
    return 0;
}

/**
 * @function [从缓存区读取一条命令]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-19T19:40:18+0800
 * @version  [1.0.0]
 * @param    buf                      [fifo地址]
 * @param    data                     [读取的字符串地址]
 * @return                            [-1已满，0成功]
 */
int buffer_read(str_buf_t * buf, unsigned char * data)
{
    /* 缓存区为空，无法读取 */
    if (buffer_is_empty(buf))
    {
        return -1;
    }
    memcpy(data, buf->data[buf->head], BUF_SIZE);
    buf->head = (buf->head + 1) % CACHE_SIZE
    return 0;
}

/**
 * @function [读命令到fifo]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-20T13:18:46+0800
 * @version  [1.0.0]
 * @param    param                    [传入命令的地址]
 * @return                            [-1失败，0成功]
 */
int read_command(unsigned char * param)
{
    int ret = 0;
    ret = buffer_write(&input, param);
    return ret;
}

/**
 * @function [发送指令的应答消息]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-20T13:35:21+0800
 * @version  [1.0.0]
 * @param    param                    [发送位置地址]
 * @return                            [-1失败，0成功]
 */
int ack_command(unsigned char * param)
{
    int ret = 0;
    memset(param, 0, BUF_SIZE);
    ret = buffer_read(&output, param);
    if (ret < 0)
    {
        return ret;
    }
    ret = strlen(param) + 1;
    return ret;
}
#endif

/* --------------------命令列表接口-------------------- */
const static char error[] = "error";
static unsigned char * cmd_ack;

static char cmd_help(const char *param)
{
    char help[32] = "commands:\r\n"
                    "h/?    help\r\n"
                    ;
    printf("%s\r\n", help);
    return 0;
}

static char cmd_version(const char *param)
{
    char *version = "V1.0.0";
//    printf("%s\r\n", version);
    sprintf((char *)cmd_ack, "%s", version);
    return 0;
}

static char cmd_ledctrl(const char *param)
{
    int led_reg = 0;
    if (sscanf(param, "0x%x", &led_reg) != 1)
    {
        printf("%s\r\n", error);
        return 1;
    }
    // TODO:返回执行结果
    led_process_cmd(0, &led_reg, 1);
//    printf("%s:%x\r\n", "ledctrl", led_reg);
    return 0;
}

static char cmd_key_flag(const char *param)
{
    int key_reg = 0;
    // TODO:返回执行结果
    key_reg = key_process_cmd(0, 0, 0);
//    printf("%s:%d\r\n", "keyflag", key_reg);
    
    sprintf((char *)cmd_ack, "keyflag %x", key_reg);
    return 0;
}

static char cmd_displayctrl(const char *param)
{
    int cmd_param;
    if (sscanf(param, "%x", &cmd_param) != 1)//1-display, 2-clear
    {
        printf("%s\r\n", "cmd_displayctrl error");
        return 1;
    }
    // TODO:返回执行结果
    display_process_cmd(cmd_param, 0, 1);
    printf("%s:%x\r\n", "displayctrl", cmd_param);
    return 0;
}

static char cmd_audioctrl(const char *param)
{
    int cmd_param = 0;
    int volume = 0;
	int audio_reg = 0;
    if (sscanf(param, "%x", &cmd_param) == 1) // 2-audiostandby, 3-audiomute, 4-beepset, 5-beepreset
    {
        audio_reg = audio_process_cmd(cmd_param, 0, 0);
		sprintf((char *)cmd_ack, "audioctrl %x", audio_reg);
        printf("%s:%x\r\n", "audioctrl", cmd_param);
    }
    else if (sscanf(param, "%x%d", &cmd_param, &volume) == 2) //6-setvolume 1-audioplay,
    {
        audio_process_cmd(cmd_param, &volume, 1);
        printf("%s:%x %d\r\n", "audioctrl", cmd_param, volume);
    }
    else
    {
        printf("%s\r\n", "cmd_audioctrl error");
        return 1;
    }
    return 0;
}

static char cmd_timeset(const char *param)
{
    int hour = 0, minute = 0, second = 0;
    int year = 0, month = 0, day = 0;
    int param_count = 0;
    param_count = sscanf(param, "0x%04d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &minute, &second);
    if (param_count == 6)
    {
        // TODO:返回执行结果
        time_process_cmd(year, month, day, hour, minute, second);
        time_show();
    }
    else {
        printf("%s\r\n", "cmd_timeset error");
        return 1;
    }
    return 0;
}

static char cmd_timeshow(const char *param)
{
    time_show();
    return 0;
}

static char cmd_adcread(const char *param)
{
    int adc_group[4] = {0};
    adc_process_cmd(0, adc_group, 4);
    sprintf((char *)cmd_ack, "adc %d %d %d %d", adc_group[0], adc_group[1], adc_group[2], adc_group[3]);
    return 0;
}

static char cmd_check_configuration(const char *param)
{
    read_data_and_analyze(246);
    return 0;
}

static char cmd_get_uart0_data(const char *param)
{
	int tmpe_data;
    tmpe_data = uart0_get_tmperature_data();
	sprintf((char *)cmd_ack, "uart0 %x", tmpe_data);
    return 0;
}

static char cmd_get_uart1_data(const char *param)
{
	int tmpe_data;
    tmpe_data = uart1_get_tmperature_data();
	sprintf((char *)cmd_ack, "uart1 %x", tmpe_data);
    return 0;
}

static char cmd_get_uart3_data(const char *param)
{
	int tmpe_data;
    tmpe_data = uart3_get_tmperature_data();
	sprintf((char *)cmd_ack, "uart3 %x", tmpe_data);
    return 0;
}

static char cmd_get_uart4_data(const char *param)
{
	int tmpe_data;
    tmpe_data = uart4_get_tmperature_data();
	sprintf((char *)cmd_ack, "uart4 %x", tmpe_data);
    return 0;
}

static char get_uart_data(const char *param)
{
	int tmpe_data;
    tmpe_data = get_tmperature_data();
	sprintf((char *)cmd_ack, "uart %x", tmpe_data);
    return 0;
}

static char set_sn(const char *param)
{
	int tmpe_data;
	int i;
	uint8_t cmd_param[6] = {0};
	
	for (i = 0; i < strlen(param); i++) {
		cmd_param[i] = (uint8_t)param[i];
		//printf("cmd_param:%x\n", cmd_param[i]);
	}
		
    tmpe_data = write_sn_to_flash(cmd_param, strlen(param));
	sprintf((char *)cmd_ack, "set_sn %x", tmpe_data);
    return 0;
}

static char read_sn(const char *param)
{
	int i;
	uint8_t read_buf[6] = {0};
	read_sn_from_flash(read_buf, 6);
	
	for(i = 0; i < 6; i++)
	{
		printf("%x\n", read_buf[i]);
	}

	sprintf((char *)cmd_ack, "read_sn %x,%x,%x,%x,%x,%x", read_buf[0],read_buf[1],read_buf[2],read_buf[3],read_buf[4],read_buf[5]);
    return 0;
}

static cmd_t cmds[] =
{
    {
        "h",
        cmd_help
    },
    {
        "ver",
        cmd_version
    },
    {
        "ledctrl",
        cmd_ledctrl
    },
    {
        "keyflag",
        cmd_key_flag
    },
    {
        "displayctrl",
        cmd_displayctrl
    },
    {
        "audioctrl",
        cmd_audioctrl
    },
    {
        "timeset",
        cmd_timeset
    },
    {
        "timeshow",
        cmd_timeshow
    },
    {
        "adcread",
        cmd_adcread
    },
    {
        "checkconf",
        cmd_check_configuration
    },
	{
		"uart0_test",
		cmd_get_uart0_data
	},
	{
		"uart1_test",
		cmd_get_uart1_data
	},
	{
		"uart3_test",
		cmd_get_uart3_data
	},
	{
		"uart4_test",
		cmd_get_uart4_data
	},
	{
		"uart_test",
		get_uart_data
	},
	{
		"ssn",
		set_sn
	},
	{
		"rsn",
		read_sn
	}
		
	
};

/**
 * @function [处理命令]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-21T11:13:52+0800
 * @version  [1.0.0]
 * @param    cmd_line                 [命令]
 * @return                            [返回值,-1失败，0成功]
 */
char handle_command(unsigned char *cmd_line)
{
    char i, ret;
    unsigned char *param = cmd_line;
    cmd_ack = cmd_line;
    // 处理传入的字符串，截取被' '和'\0'分隔的第一段，此段作为命令
    while ((*param != ' ') && (*param != '\0')) param++;
    *param = '\0'; param++;
	printf("cmd_line:%s\n", cmd_line);
	

    // 忽略大小写比较字符串
    for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++)
    {
		printf("cmds[i].cmd: %s\n", cmds[i].cmd);
        if (strcasecmp(cmds[i].cmd, (const char *)cmd_line) == 0)
        {
            break;
        }
    }

    // 超过命令列表大小
    if (i >= sizeof(cmds) / sizeof(cmds[0]))
    {
//		printf("444444444\n");
        goto show_help;
//		return 1;
    }

    // 处理命令，传入后续参数字符串
    ret = cmds[i].handler((const char *)param);
    return ret;

show_help:
    // 发送数据
    cmd_help(NULL);
    return 1;
}
