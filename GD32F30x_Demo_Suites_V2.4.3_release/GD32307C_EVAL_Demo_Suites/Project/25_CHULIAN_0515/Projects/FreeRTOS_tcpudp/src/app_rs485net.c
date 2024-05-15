#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include "lwip/api.h"
#include "protocol_modbus.h"
#include "protocol_usart.h"
#include "app_rs485net.h"
#include "module_display.h"
#include "app_configure.h"
#include "app_format.h"
#include "module_audio.h"
#include "module_led.h"
#include "module_key.h"
#include "module_adc.h"
#include <string.h>
#include "systick.h"
#include <math.h>

#define RS485NET_TASK_PRIO  ( tskIDLE_PRIORITY + 5 )

extern struct config_data *p_configuration_data;

static char NettoStop = 0;
TaskHandle_t NetTaskHandle = NULL;



struct rs485net_timing_t
{
    uint32_t bus;
    uint16_t frequency;
    uint16_t address;
} deviceList[32];

void Delay_ms(uint16_t nms)
{
    uint16_t i, j;

    for(i = 0; i < nms * 1024; i++)
    {
        for(j = 0; j < nms * 1024; j++)
        {
            ;
        }
    }
}
uint8_t device_num = 0;

#if 0
void scan_dev(uint32_t usart_periph)
{
    unsigned char dev = 0;
    unsigned char rdata[8] = {0};
    int retry = 0;

    for (dev = 0; dev < 0xff; dev++)
    {
        while (usart_send(usart_periph, (uint8_t *)packed_modbus_rtu_frame(dev, 0x03, 0x0000, 0x0001), 8) < 0)
        {
            retry++;
            if (retry > 100)
            {
                retry = 0;
//                printf("usart_send retry over 100 times \r\n");
                break;
            }
        }
        while (usart_recv_pre(usart_periph, 7) < 0)
        {
            retry++;
            if (retry > 100)
            {
                retry = 0;
//                printf("usart_recv_pre retry over 100 times \r\n");
                break;
            }
        }
        while (usart_recv(usart_periph, rdata) < 0)
        {
            retry++;
            if (retry > 100)
            {
                retry = 0;
//                printf("usart_recv retry over 100 times \r\n");
                break;
            }
        }
    }
}
#endif

uint16_t smoke_alarm_status = 0, gas_alarm_status = 0;
uint16_t temp_warn_status = 0, temp_alarm_status = 0;
uint8_t dhlr_warn_status = 0, dhlr_alarm_status = 0;
uint32_t smoke_alarm_ltimestamp = 0, gas_alarm_ltimestamp = 0;
uint32_t temp_warn_ltimestamp = 0, temp_alarm_ltimestamp = 0;
uint32_t dhlr_warn_ltimestamp = 0, dhlr_alarm_ltimestamp = 0;

uint8_t smoke_num = 0;
uint8_t gas_num = 0;
uint8_t temp_num = 0;
uint8_t pir_num = 0;
static uint32_t EVAL_COM[4] = {EVAL_COM0, EVAL_COM1, EVAL_COM3, EVAL_COM4};
uint8_t smoke_status[MAX_SENSOR_NUM] = {0};
uint8_t gas_status[MAX_SENSOR_NUM] = {0};
uint8_t temp_status[MAX_SENSOR_NUM] = {0};
uint8_t pir_status[4] = {0}, camera_status[4] = {0};
int led_status = 0;
int audio_num = 0;
uint8_t temp_count = 0;
uint8_t smoke_count = 0;
uint8_t gas_count = 0;
int start_record = 0;
int  stop_record= 0;
uint8_t cancel_flag = 0;
uint8_t lr_status[4] = {0}, dh_status[4] = {0}, dhlr_status[4] = {0};

void cancel_alarm()
{	
	printf("----- cancel alarm -----\r\n");
	led_status &= 0xdf;
	led_process_cmd(0, &led_status, 1);
	audio_process_cmd(0, 0, 0);
	audio_process_cmd(5, 0, 0);
	stop_record = 1;
}

void alarm_action(uint8_t n)
{
	start_record = 1;
	//报警灯
	led_status |= 0x30;
//	printf("led status %d\r\n", led_status);
	led_process_cmd(0, &led_status, 1);
//	vTaskDelay(pdMS_TO_TICKS(300));
	//蜂鸣器
	audio_process_cmd(4, 0, 0);
	//语音播放
	set_num(n);  //动火离人预警
	audio_process_cmd(1, 0, 0);
}

void set_alarm_data()
{
	alarmdata_report.data_type = 1;
	alarmdata_report.alarm_type.time_stamp = rtc_counter_get();
	alarmdata_report.alarm_param.time_stamp = alarmdata_report.alarm_type.time_stamp;
	alarmdata_report.serial_number.time_stamp = alarmdata_report.alarm_type.time_stamp;
	alarmdata_report.sensor_id.time_stamp = alarmdata_report.alarm_type.time_stamp;
	
	memcpy(alarmdata_report.serial_number.value, heartbeat_reportdata.heart_data.serial_number.value, 21);
	alarmdata_report.data_type = 1;
}

	
static void rs485net_task(void *arg)
{
	uint32_t NetNotifyValue = 0;
	uint32_t count = 0;
	uint32_t ltimestamp = 0, temp_warn_time = 0, temp_alarm_time = 0, left_ltimestamp = 0;
	uint32_t dhlr_warn_time = 0, dhlr_alarm_time = 0, gas_alarm_time = 0, smoke_alarm_time = 0;
	uint8_t index = 0;
	uint16_t retry = 0;
	unsigned char rdata[8] = {0};
	char display_buf[7];
	int key_status = 0, key_status_back = 0;
	int adc_value[4] = {0};

	
    memcpy(heartbeat_reportdata.heart_data.serial_number.value, p_configuration_data->host_serial, sizeof(p_configuration_data->host_serial));
    ltimestamp = rtc_counter_get();
    while (1)
    {
		if (xTaskNotifyWait(0UL, 0xffffffffUL, &NetNotifyValue, portMAX_DELAY) == pdTRUE) {}
		while(!NettoStop)
		{
	        // get key status
	        key_status = get_key_value();
	        //printf("\r\nkeybitmap:%02x\tkeybitmap_back:%02x\tcount:%d\r\n", key_status, key_status_back, count);
	        if (key_status != key_status_back)
	        {	
				printf("---- key_status cancel alarm----\r\n");
				key_status_back = key_status;
				cancel_alarm();
				smoke_alarm_status = 0;
				gas_alarm_status = 0;
				temp_warn_status = 0;
				temp_alarm_status = 0;
				dhlr_warn_status = 0;
				dhlr_alarm_status = 0;
				memset(pir_status, 0, 4);
				memset(camera_status, 0, 4);
				memset(dhlr_status, 0, 4);
				memset(temp_status, 0, MAX_SENSOR_NUM);
				memset(smoke_status, 0, MAX_SENSOR_NUM);
				memset(gas_status, 0, MAX_SENSOR_NUM);
	        }
        
	        memset(rdata, 0x00, 8);
			smoke_num = 0; gas_num = 0; temp_num = 0, pir_num = 0;
	        heartbeat_reportdata.heart_data.serial_number.time_stamp = rtc_counter_get();
			
	        for (index = 0; index < device_num; index++)
	        {
	            if ((count % deviceList[index].frequency) == 0)
	            {
	                while (usart_send(deviceList[index].bus, (uint8_t *)packed_modbus_rtu_frame(deviceList[index].address, 0x03, 0x0000, 0x0001), 8) < 0){}
	                // delay 1ms to make sure send finish
	                vTaskDelay(pdMS_TO_TICKS(1)); // 不能改 
				
	                usart_recv_clear(deviceList[index].bus);
	                while (usart_recv_pre(deviceList[index].bus, 7) < 0)
	                {
	                    retry++;
	                    vTaskDelay(pdMS_TO_TICKS(10));
	                    if (retry > 100)
	                    {
	                        retry = 0;
	                        //printf("usart_recv_pre retry over 100 times \r\n");
	                        break;
	                    }
	                }
				
	                while (usart_recv(deviceList[index].bus, rdata) < 0)
	                {
						if(index == 0){
							retry = 101;
							break;
						}
	                    retry++;
	                    vTaskDelay(pdMS_TO_TICKS(10));
	                    if (retry > 100)
	                    {
	                        printf("usart_recv %d retry over 100 times \r\n", index);
	                        break;
	                    }
	                }

	                if (retry > 100)
	                {
	                    retry = 0;
	                }
	                else
	                {
						if (parse_modbus_rtu_frame((ModbusRtuFrameAck *)rdata) == 0)
						{
							led_status |= 0x08;
							led_process_cmd(0, &led_status,1);
							// 动火离人模式 电流判断
							if(p_configuration_data->hotwork_noperson_mode == 0)
							{
								uint8_t temp_addr = 0;
								switch (((ModbusRtuFrameAck *)rdata)->slave_address >> 6)
								{
								case 0x02: // smoker
									
									sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
									// TODO:display smoker value
									lcd_display(4, 5, display_buf);

									// TODO: upload smoker sensor's value
									heartbeat_reportdata.heart_data.smoke_detector.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									//sprintf(heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].device_id, "%d", temp_addr);
									heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];

									// TODO: compare with alram th
									printf("smoker_sensor[%d] value: %d\r\n", smoke_num, heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param);
									if ((key_status == key_status_back) && (heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param > p_configuration_data->smoke_th * 100))//2000))
									{
										if(smoke_alarm_status == 0 
											&& gas_alarm_status == 0 && temp_alarm_status == 0 && dhlr_warn_status == 0 && dhlr_alarm_status == 0)
										{
											//printf("*************smoke************\r\n");
											smoke_alarm_status = smoke_num + 1;
											smoke_alarm_ltimestamp = rtc_counter_get();
										}
										smoke_status[smoke_num] = 1;
									}else{
										smoke_status[smoke_num] = 0;
									}
									smoke_num++;
									break;
								
								case 0x01: // gas
									sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
									// TODO:display gas value
									lcd_display(3, 5, display_buf);
									// TODO: compare with alram th
									// TODO: upload smoker sensor's value
									heartbeat_reportdata.heart_data.gas_detector.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									//sprintf(heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].device_id, "%d", temp_addr);
									heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];
									printf("gas_sensor[%d] value %d\r\n", gas_num, heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param);
									
									if ((key_status == key_status_back) && (heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param > p_configuration_data->gas_th * 100))//2000))
									{
										if(gas_alarm_status == 0)
//											&& smoke_alarm_status == 0 && temp_alarm_status == 0 && dhlr_warn_status == 0 && dhlr_alarm_status == 0)
										{
											//printf("************gas************\r\n");
											gas_alarm_status = gas_num + 1;
											gas_alarm_ltimestamp = rtc_counter_get();
										}
										gas_status[gas_num] = 1;
									}else{
										gas_status[gas_num] = 0;
									}
									gas_num ++;
									break;
								
								case 0x00: // tempeture
									heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] << 8 | ((ModbusRtuFrameAck *)rdata)->data[1];
									sprintf(display_buf, " %03d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) % 1000, heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param % 10);
									// TODO:display tempeture value
									lcd_display(2, 4, display_buf);

									// TODO: upload tempeture sensor's value
									heartbeat_reportdata.heart_data.temperature.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.temperature.sensors[temp_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									//sprintf(heartbeat_reportdata.heart_data.temperature.sensors[temp_num].device_id, "%d", temp_addr);
									// TODO: compare with alram th
									printf("temperature_sensor[%d]  value %d \r\n", temp_num, heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param);
									//温度大于报警温度 不管有无人 报警
									if ((key_status == key_status_back) && ((heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) > (p_configuration_data->alarm_temp - p_configuration_data->temp_calibrate)))//200))
									{
										if(temp_alarm_status == 0
											&& dhlr_warn_status == 0 && dhlr_alarm_status == 0 && smoke_alarm_status == 0 && gas_alarm_status == 0 )
										{
											temp_alarm_status = temp_num + 1;
										
											if(temp_warn_status != 0){
												temp_warn_status = 0;
												temp_alarm_ltimestamp = temp_warn_ltimestamp;
											}else{
												temp_alarm_ltimestamp = rtc_counter_get();
											}
										}
										temp_status[temp_num] = 1;
									}
									else if((key_status == key_status_back) && temp_alarm_status == 0 
										&& ((heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) > (p_configuration_data->warn_temp - p_configuration_data->temp_calibrate)))//100))
									{
										if(temp_warn_status == 0){
											temp_warn_status = temp_num + 1;
											temp_warn_ltimestamp = rtc_counter_get();
										}
										temp_status[temp_num] = 2;
									}
									else
									{
										temp_status[temp_num] = 0;
									}
									temp_num++;
									break;
								case 0x03: // pir
									// TODO:display pir value
									temp_addr = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									sprintf(heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].device_id, "%d", temp_addr);
									if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel0.pir_id){
										pir_num = 0;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel1.pir_id){
										pir_num = 1;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel2.pir_id){
										pir_num = 2;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel3.pir_id){
										pir_num = 3;
									}
									if (((ModbusRtuFrameAck *)rdata)->data[1] == 1)
									{ // 有人，清空离人时间
										
//										vTaskDelay(pdMS_TO_TICKS(20));
										heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status = ((ModbusRtuFrameAck *)rdata)->data[1];;
										heartbeat_reportdata.heart_data.body_detector.time_stamp = 0;
										sprintf(display_buf, " 000");
										lcd_display(1, 5, display_buf);
										pir_status[pir_num] = 0;
									}
									else
									{ // 没人，且上次也没人
										if (heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status == ((ModbusRtuFrameAck *)rdata)->data[1])
										{
											heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp = heartbeat_reportdata.heart_data.body_detector.time_stamp - left_ltimestamp;
											heartbeat_reportdata.heart_data.body_detector.leave_time = heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp;
											if(left_ltimestamp == 0){
												left_ltimestamp = rtc_counter_get();
											}
										}
										else // 没人，上次有人
										{	
											// person left detected, save timestamp
											left_ltimestamp = rtc_counter_get();
										}
										pir_status[pir_num] = 1;
										heartbeat_reportdata.heart_data.body_detector.status = pir_num + 1;
										heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status = ((ModbusRtuFrameAck *)rdata)->data[1];
										heartbeat_reportdata.heart_data.body_detector.time_stamp = rtc_counter_get();
									}
									printf("pir_sensor[%d] status %d\r\n", pir_num, heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status);
									// 显示时间
									if ((heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp > 0) && (heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp < 1000))
									{
										sprintf(display_buf, " %03d", heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp);
										lcd_display(1, 5, display_buf);
										printf("left time %d\r\n", heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp);
									}
									break;
								default:
									break;
								}

							}
							// 动火离人模式 温度判断
							else if(p_configuration_data->hotwork_noperson_mode == 1)
							{
								uint8_t temp_addr = 0;
								switch (((ModbusRtuFrameAck *)rdata)->slave_address >> 6)
								{
								case 0x02: // smoker
									sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
									// TODO:display smoker value
									lcd_display(4, 5, display_buf);

									// TODO: upload smoker sensor's value
									heartbeat_reportdata.heart_data.smoke_detector.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
						
									heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];

									// TODO: compare with alram th
									printf("smoker_sensor[%d] value: %d\r\n", smoke_num, heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param);
								
									if ((key_status == key_status_back) && (heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_num].param > p_configuration_data->smoke_th * 100))//2000))
									{
										if(smoke_alarm_status == 0 && gas_alarm_status == 0 
											&& temp_alarm_status == 0 && dhlr_alarm_status == 0&& dhlr_warn_status == 0)
										{
											smoke_alarm_status = smoke_num + 1;
											smoke_alarm_ltimestamp = rtc_counter_get();
										}
										smoke_status[smoke_num] = 1;
									}else{
										smoke_status[smoke_num] = 0;
									}
									smoke_num++;
									break;
								
								case 0x01: // gas
									sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
									// TODO:display gas value
									lcd_display(3, 5, display_buf);
									// TODO: compare with alram th
									// TODO: upload smoker sensor's value
									heartbeat_reportdata.heart_data.gas_detector.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									
									heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];
									printf("gas_sensor[%d] value %d\r\n", gas_num, heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param);
							
									if ((key_status == key_status_back) && (heartbeat_reportdata.heart_data.gas_detector.sensors[gas_num].param > p_configuration_data->gas_th * 100))//2000))
									{
										if(gas_alarm_status == 0 )
//											&& smoke_alarm_status == 0 && temp_alarm_status == 0 && dhlr_alarm_status == 0 && dhlr_warn_status == 0)
										{
											//printf("*************gas****************\r\n");
											gas_alarm_status = gas_num + 1;
											gas_alarm_ltimestamp = rtc_counter_get();
										}
										gas_status[gas_num] = 1;
									}else{
										gas_status[gas_num] = 0;
									}
									gas_num ++;
									break;
								
								case 0x00: // tempeture
									heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param = ((ModbusRtuFrameAck *)rdata)->data[0] << 8 | ((ModbusRtuFrameAck *)rdata)->data[1];
								
									sprintf(display_buf, " %03d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) % 1000, heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param % 10);
									// TODO:display tempeture value
									lcd_display(2, 4, display_buf);

									// TODO: upload tempeture sensor's value
									heartbeat_reportdata.heart_data.temperature.time_stamp = rtc_counter_get();
									heartbeat_reportdata.heart_data.temperature.sensors[temp_num].device_id = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									// TODO: compare with alram th
									printf("temperature_sensor[%d]  value %d \r\n", temp_num, heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param);
									//温度大于报警温度 不管有无人 报警
									if ((key_status == key_status_back) && ((heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) > (p_configuration_data->alarm_temp - p_configuration_data->temp_calibrate)))//200))
									{
										if(smoke_alarm_status == 0 && gas_alarm_status == 0 
											&& temp_alarm_status == 0 && dhlr_alarm_status == 0 && dhlr_warn_status == 0)
										{
											temp_alarm_status = temp_num + 1;
											if(temp_warn_status != 0){
												temp_warn_status = 0;
												temp_alarm_ltimestamp = temp_warn_ltimestamp;
											}else{
												temp_alarm_ltimestamp = rtc_counter_get();
											}
										}
										temp_status[temp_num] = 1;
									}
									else if((key_status == key_status_back) && ((heartbeat_reportdata.heart_data.temperature.sensors[temp_num].param / 10) > (p_configuration_data->warn_temp - p_configuration_data->temp_calibrate)))//100))
									{
										if(temp_warn_status == 0 && smoke_alarm_status == 0 
											&& gas_alarm_status == 0 && dhlr_alarm_status == 0 && dhlr_warn_status == 0){
											temp_warn_status = temp_num + 1;
											temp_warn_ltimestamp = rtc_counter_get();
										}
										temp_status[temp_num] = 2;
									}
									else{
										temp_status[temp_num] = 0;
									}
									temp_num++;
									break;
								case 0x03: // pir
									// TODO:display pir value
									temp_addr = ((ModbusRtuFrameAck *)rdata)->slave_address + 1;
									sprintf(heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].device_id, "%d", temp_addr);
									if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel0.pir_id){
										pir_num = 0;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel1.pir_id){
										pir_num = 1;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel2.pir_id){
										pir_num = 2;
									}
									else if((((ModbusRtuFrameAck *)rdata)->slave_address & 0x0f) == p_configuration_data->channel3.pir_id){
										pir_num = 3;
									}
									if (((ModbusRtuFrameAck *)rdata)->data[1] == 1)
									{ // 有人，清空离人时间
										
//										vTaskDelay(pdMS_TO_TICKS(20));
										heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status = ((ModbusRtuFrameAck *)rdata)->data[1];;
										heartbeat_reportdata.heart_data.body_detector.time_stamp = 0;
//										left_ltimestamp = 0;
//										heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp = 0;
										sprintf(display_buf, " 000");
										lcd_display(1, 5, display_buf);
										pir_status[pir_num] = 0;
									}
									else
									{ // 没人，且上次也没人
										if (heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status == ((ModbusRtuFrameAck *)rdata)->data[1])
										{
											heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp = heartbeat_reportdata.heart_data.body_detector.time_stamp - left_ltimestamp;
											heartbeat_reportdata.heart_data.body_detector.leave_time = heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp;
											if(left_ltimestamp == 0){
												left_ltimestamp = rtc_counter_get();
											}
										}
										else // 没人，上次有人
										{	
											// person left detected, save timestamp
											left_ltimestamp = rtc_counter_get();
										}
										pir_status[pir_num] = 1;
										heartbeat_reportdata.heart_data.body_detector.status = pir_num + 1;
										heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status = ((ModbusRtuFrameAck *)rdata)->data[1];
										heartbeat_reportdata.heart_data.body_detector.time_stamp = rtc_counter_get();
									}
									printf("pir_sensor[%d] status %d\r\n", pir_num, heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].status);
									// 显示时间
									if ((heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp > 0) && (heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp < 1000))
									{
										sprintf(display_buf, " %03d", heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp);
										lcd_display(1, 5, display_buf);
										printf("left time %d\r\n", heartbeat_reportdata.heart_data.body_detector.sensors[pir_num].leave_time_stamp);
									}
									break;
								default:
									break;
								}
							}
						}
	                }
				}
	            vTaskDelay(pdMS_TO_TICKS(200));
	        }
			temp_count = temp_num;
			gas_count = gas_num;
			smoke_count = smoke_num;
		
	        count++;
			if(count == 1000){
				count = 0;
			}
			
/*------------------------ 离人开关量 --------------------------*/
			if(p_configuration_data->camera_type == 0){
				key_status = get_key_value();
				
				for(index = 0; index < 4; index++){
					if(!(key_status & (0x01 << (index + 1)))){ //低电平
						camera_status[index] = 1;
						printf("switch[%d] camera_status[%d] %d ----- 离人 -----\r\n", index+1,index, camera_status[index]);
					}else{
						camera_status[index] = 0; 
						printf("switch[%d] camera_status[%d] %d ----- 有人 -----\r\n", index+1,index, camera_status[index]);
					}
				}
			}
/*------------------------ 离人开关量 --------------------------*/		
			if(p_configuration_data->camera_type == 1){
				for(index = 0; index < 4; index++){
					printf("camera_status[%d] %d\t", index, camera_status[index]);
				}
				printf("\r\n");
			}
			if(p_configuration_data->hotwork_noperson_mode == 0)
			{
/*****************************传感器状态判断**************************************/
				/* 摄像头，pir，互感绑定 */
				// TODO:ADC read  动火状态
				adc_process_cmd(0, adc_value, 4);
				/* 判断离人、动火状态*/
				for(index = 0; index < 4; index ++){
					if(adc_value[index] > p_configuration_data->hot_work_threshold){
						printf("-----dh_status  %d -----\r\n", index);
						dh_status[index] = 1;
						heartbeat_reportdata.heart_data.fire_detector.status = index + 1;
						heartbeat_reportdata.heart_data.fire_detector.time_stamp = rtc_counter_get();
					}else{
						dh_status[index] = 0;
					}
					if(pir_status[index] == 1 && camera_status[index] == 1){
						printf("-----lr_status   %d  -----\r\n", index);
						lr_status[index] = 1;
					}else{
						lr_status[index] = 0;
					}
				}
				/* 动火离人状态 */
				for(index = 0; index < 4; index++){
					if(dh_status[index] == 1  //动火
					&& lr_status[index] == 1) //离人
					{
						printf("-----dhlr_status   %d   -----\r\n", index);
						dhlr_status[index] = 1;
						if(dhlr_warn_status == 0 && dhlr_alarm_status == 0){
							dhlr_warn_status = index + 1;
							dhlr_warn_ltimestamp = rtc_counter_get();
						}
					}else{
						dhlr_status[index] = 0;
					}
				}	

				/* 判断动火状态 */
				if(heartbeat_reportdata.heart_data.fire_detector.status != 0){
					for(index = 0; index < 4; index ++){
						if(dh_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						heartbeat_reportdata.heart_data.fire_detector.status = 0;
					}
				}
				/* 判断离人状态 */
				if(heartbeat_reportdata.heart_data.body_detector.status != 0){
					for(index = 0; index < 4; index ++){
						if(lr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						heartbeat_reportdata.heart_data.body_detector.status = 0;
						heartbeat_reportdata.heart_data.body_detector.leave_time = 0;
					}
				}
				/* 判断动火离人报警 */
				if(dhlr_alarm_status != 0){
					for(index = 0; index < 4; index ++){
						if(dhlr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						printf("dhlr_alarm cancel----\r\n");
						dhlr_alarm_status = 0;
						dhlr_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
				/* 判断动火离人预警 */
				if(dhlr_warn_status != 0 && dhlr_alarm_status == 0){
					for(index = 0; index < 4; index ++){
						if(dhlr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						printf("dhlr_warn cancel --- \r\n");
						dhlr_warn_status = 0;
						dhlr_warn_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
				/* 判断燃气状态 */
				if(gas_alarm_status != 0){
					for(index = 0; index < gas_num; index ++){
						if(gas_status[index] == 1){
							break;
						}
					}
					if(index == gas_num){
						gas_alarm_status = 0;
						gas_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}	
				/* 判断烟雾状态 */
				if(smoke_alarm_status != 0){
					for(index = 0; index < smoke_num; index ++){
						if(smoke_status[index] == 1){
							break;
						}
					}
					if(index == smoke_num){
						smoke_alarm_status = 0;
						smoke_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
			
				/* 判断温度状态 */
				if(temp_alarm_status != 0){
					for(index = 0; index < temp_num; index ++){
						if(temp_status[index] != 0){
							break;
						}
					}
					if(index == temp_num){
						temp_alarm_status = 0;
						temp_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
				/* 判断高温预警 */
				if(temp_warn_status != 0 && temp_alarm_status == 0){
					for(index = 0; index < temp_num; index ++){
						if(temp_status[index] == 1){
							temp_alarm_status = temp_warn_status;
							temp_warn_status = 0;
							break;
						}
						if(temp_status[index] != 0){
							break;
						}
					}
					if(index == temp_num){
						temp_warn_status = 0;
						temp_warn_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}

	/****************************************************************************************/
				if(dhlr_warn_status != 0 && dhlr_alarm_status == 0 && (key_status == key_status_back))//动火预警
				{	//时间大于预警延迟//无动火预警行为
					//printf("-----------------dhlr----------------\r\n");
					ltimestamp = rtc_counter_get();
					if((dhlr_warn_time == 0) && (ltimestamp - dhlr_warn_ltimestamp > p_configuration_data->warn_delay))
					{	
						printf("dhlr warn > delay action\r\n");
						//记录预警时间
						dhlr_warn_time = rtc_counter_get();
						//上报预警信息
						sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_WARN");
						sprintf(alarmdata_report.sensor_id.value, "%s", heartbeat_reportdata.heart_data.body_detector.sensors[dhlr_warn_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d", ltimestamp - dhlr_warn_ltimestamp);
						set_alarm_data();
						//报警
						alarm_action(0);
						cancel_flag = 0;
					}
					//有动火预警行为// 动火预警时间 大于 预警时长 取消预警
					ltimestamp = rtc_counter_get();
					if((dhlr_warn_time != 0) && (dhlr_alarm_time == 0) && cancel_flag == 0 &&
						(ltimestamp - dhlr_warn_time > p_configuration_data->warn_time))
					{  
						printf("dhlr warn cancel action\r\n");
						cancel_alarm();
						clearalarm();
						cancel_flag = 1;
					}
				
					// 动火时间 大于 报警延迟 报警
					ltimestamp = rtc_counter_get();
					if((dhlr_warn_time != 0) && (dhlr_alarm_time == 0) && 
						(ltimestamp - dhlr_warn_time > p_configuration_data->alarm_delay))
					{
						printf("dhlr warn -> alarm action\r\n");
						//进入动火离人报警状态
						dhlr_alarm_status = dhlr_warn_status;
						dhlr_alarm_time = rtc_counter_get();
						// 报警
						sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_ALARM");
						sprintf(alarmdata_report.sensor_id.value, "%s", heartbeat_reportdata.heart_data.body_detector.sensors[dhlr_alarm_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d", ltimestamp - dhlr_warn_time);
						//退出预警状态
						dhlr_warn_status = 0;
						dhlr_warn_time = 0;
						set_alarm_data();
						alarm_action(1);
						cancel_flag = 0;
					}
				}
				else if(dhlr_alarm_status != 0 && (key_status == key_status_back)) // 动火报警
				{	
					// 断电断气
					ltimestamp = rtc_counter_get();
					if((dhlr_alarm_time != 0) && (ltimestamp - dhlr_alarm_time > p_configuration_data->break_delay))
					{
						key_process_cmd(0, 0, 0); //断电断气
					}
					// 动火报警时间大于报警时长
					ltimestamp = rtc_counter_get();
					if((dhlr_alarm_time != 0) && cancel_flag == 0 &&(ltimestamp - dhlr_alarm_time > p_configuration_data->alarm_time))
					{
						printf("dhlr alarm cancel \r\n");
						cancel_alarm(); //取消报警
						cancel_flag = 1;
						clearalarm();
						cancel_flag = 1;
						dhlr_alarm_status = 0;
						dhlr_alarm_time = 0;
						dhlr_warn_status = 0;
						dhlr_warn_time = 0;
						memset(dhlr_status, 0, 4);
					}
				}  // end  if(dhlr_alarm_status == 1)

				/* 高温预警 */
				if(temp_warn_status != 0 && temp_alarm_status == 0 && (key_status == key_status_back))
				{
					//温度预警时间大于预警延迟 
					ltimestamp = rtc_counter_get();
					if((temp_warn_time == 0) && (ltimestamp - temp_warn_ltimestamp > p_configuration_data->warn_delay))
					{  
						printf("temp warn > delay \r\n");
						temp_warn_time = rtc_counter_get();
						//预警
						sprintf(alarmdata_report.alarm_type.value, "%s", "OIL_TEMP_WARN");
						sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C",heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].param / 10) % 1000, 
						heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].param % 10);
						set_alarm_data();
						alarm_action(2);
						cancel_flag = 0;
					}
					// 有温度预警行为//温度预警时间大于预警时长
					ltimestamp = rtc_counter_get();
					if((temp_warn_time != 0) && cancel_flag == 0 &&(ltimestamp - temp_warn_time > p_configuration_data->warn_time))
					{
						printf("cancel temp warn \r\n");
						cancel_alarm();
						cancel_flag = 1;
						clearalarm();
					}
				
					//温度预警时间大于报警延迟
					ltimestamp = rtc_counter_get();
					if((temp_warn_time != 0) && (ltimestamp - temp_warn_time > p_configuration_data->alarm_delay))
					{	
						printf("temp warn -> alarm action \r\n");
						//进入高温报警状态
						temp_alarm_status = temp_warn_status;
						temp_alarm_time = rtc_counter_get();
						//退出预警状态
						temp_warn_status = 0;
						temp_warn_time = 0;
						//报警
						sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
						sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param / 10) % 1000, 
							heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param % 10);
						set_alarm_data();
						alarm_action(3);
						cancel_flag = 0;
					}
				}
				
				/* 高温报警 */
				if(temp_alarm_status != 0 && (key_status == key_status_back))
				{	//高温报警时间大于报警延迟//无报警行为
					ltimestamp = rtc_counter_get();
					if((temp_alarm_time == 0) && (ltimestamp - temp_alarm_ltimestamp > p_configuration_data->alarm_delay))
					{
						printf("temp alarm action start \r\n");
						temp_alarm_time = rtc_counter_get();
						sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
						sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param / 10) % 1000, 
							heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param % 10);
						set_alarm_data();
						alarm_action(3);
						cancel_flag = 0;
					}
					// 断电断气
					ltimestamp = rtc_counter_get();
					if((temp_alarm_time != 0) && (ltimestamp - temp_alarm_time > p_configuration_data->break_delay))
					{
						//printf("--------------------断电断气---------------------\r\n");
						key_process_cmd(0, 0, 0); //断电断气
					}
					//报警时间大于报警时长
					ltimestamp = rtc_counter_get();
					if((temp_alarm_time != 0) && cancel_flag == 0 && (ltimestamp - temp_alarm_time > p_configuration_data->alarm_time))
					{
						printf("cancel temp alarm action\r\n");
						cancel_alarm();
						cancel_flag = 1;
						clearalarm();
						temp_alarm_status = 0;
						temp_alarm_time = 0;
						temp_warn_status = 0;
						temp_warn_time = 0;
						memset(temp_status, 0, MAX_SENSOR_NUM);
					}
				}
			
				/* 燃气 报警 */
				if(gas_alarm_status != 0 && (key_status == key_status_back))
				{
					//printf("**************************gas aram\r\n");
					//报警时间大于报警延迟//没有报警行为
					ltimestamp = rtc_counter_get();
					if((gas_alarm_time == 0) && (ltimestamp - gas_alarm_ltimestamp > p_configuration_data->alarm_delay))
					{
						printf("gas alarm action\r\n");
						gas_alarm_time = rtc_counter_get();
					
						sprintf(alarmdata_report.alarm_type.value, "%s", "GAS_ALARM");
						sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "R", heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d.%d", heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].param / 100, 
								heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].param % 100);
						set_alarm_data();
						alarm_action(4);
						cancel_flag = 0;
					}	
					// 断电断气
					ltimestamp = rtc_counter_get();
					if((gas_alarm_time != 0) && (ltimestamp - gas_alarm_time > p_configuration_data->break_delay))
					{
						key_process_cmd(0, 0, 0); //断电断气
					}
					//报警时间大于报警时长
					ltimestamp = rtc_counter_get();
					if((gas_alarm_time != 0) && cancel_flag == 0&& (ltimestamp - gas_alarm_time > p_configuration_data->alarm_time))
					{
						printf("cancel gas alarm action\r\n");
						cancel_alarm();
						cancel_flag = 1;
						clearalarm();

						gas_alarm_status = 0;
						gas_alarm_time = 0;
						memset(gas_status, 0, MAX_SENSOR_NUM);
					}
				}
			
				/* 烟雾报警 */
				if(smoke_alarm_status != 0 && (key_status == key_status_back))
				{
					//报警时间大于报警延迟//没有报警行为
					ltimestamp = rtc_counter_get();
					if((smoke_alarm_time == 0) && (ltimestamp - smoke_alarm_ltimestamp > p_configuration_data->alarm_delay))
					{
						printf("smoke alarm action\r\n");
						smoke_alarm_time = rtc_counter_get();
					
						sprintf(alarmdata_report.alarm_type.value, "%s", "SMOKE_ALARM");
						sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "Y",heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].device_id + 1);
						sprintf(alarmdata_report.alarm_param.value, "%d.%d", heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].param / 100, 
								heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].param % 100);
						set_alarm_data();
						alarm_action(5);
						cancel_flag = 0;
					}
					// 断电断气
					ltimestamp = rtc_counter_get();
					if((smoke_alarm_time != 0) && (ltimestamp - smoke_alarm_time > p_configuration_data->break_delay))
					{
						key_process_cmd(0, 0, 0); //断电断气
					}
					//报警时间大于报警时长
					ltimestamp = rtc_counter_get();
					if((smoke_alarm_time != 0) && cancel_flag == 0 && (ltimestamp - smoke_alarm_time > p_configuration_data->alarm_time))
					{
						printf("cancel smoke alarm action\r\n");
						cancel_alarm();
						cancel_flag = 1;
						clearalarm();
						smoke_alarm_status = 0;
						smoke_alarm_time = 0;
						memset(smoke_status, 0, MAX_SENSOR_NUM);
					}
				}
				vTaskDelay(pdMS_TO_TICKS(100));
			}
			else if(p_configuration_data->hotwork_noperson_mode == 1)
			{
				/* -------- 动火离人状态判断 -------- */
				for(index = 0; index < 4; index++){
					if(pir_status[index] == 1 && camera_status[index] == 1)
					{
						lr_status[index] = 1;
					}else{
						lr_status[index] = 0;
					}
					
					if((temp_alarm_status != 0 || temp_warn_status != 0)  //动火
					&& lr_status[index] == 1) //离人
					{  
						dhlr_status[index] = 1;
						if(dhlr_warn_status == 0 && dhlr_alarm_status == 0){
							dhlr_warn_status = index + 1;
							dhlr_warn_ltimestamp = rtc_counter_get();
							heartbeat_reportdata.heart_data.fire_detector.status = index + 1;
							heartbeat_reportdata.heart_data.fire_detector.time_stamp = rtc_counter_get();
						}
					}else{
						dhlr_status[index] = 0;
					}
				}
		/*****************************传感器状态判断**************************************/
				/* 判断离人状态 */
				if(heartbeat_reportdata.heart_data.body_detector.status != 0){
					for(index = 0; index < 4; index ++){
						if(lr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						heartbeat_reportdata.heart_data.body_detector.status = 0;
						heartbeat_reportdata.heart_data.body_detector.leave_time = 0;
					}
				}
				/* 判断动火离人报警 */
				if(dhlr_alarm_status != 0){
					for(index = 0; index < 4; index ++){
						if(dhlr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						dhlr_alarm_status = 0;
						dhlr_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
				/* 判断动火离人预警 */
				if(dhlr_warn_status != 0 && dhlr_alarm_status == 0){
					for(index = 0; index < 4; index ++){
						if(dhlr_status[index] != 0){
							break;
						}
					}
					if(index == 4){
						dhlr_warn_status = 0;
						dhlr_warn_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
				/* 判断燃气状态 有报警 则报警 都不报警 则不报警 */
				if(gas_alarm_status != 0)
				{
					for(index = 0; index < gas_num; index ++){
						if(gas_status[index] == 1){
							break;
						}
					}
					if(index == gas_num){
						gas_alarm_status = 0;
						gas_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}	
				/* 判断烟雾状态 有报警 则报警 都不报警 则不报警*/
				if(smoke_alarm_status != 0)
				{
					for(index = 0; index < smoke_num; index ++){
						if(smoke_status[index] == 1){
							break;
						}
					}
					if(index == smoke_num){
						smoke_alarm_status = 0;
						smoke_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
			
				/* 判断温度状态 有报警 则报警 都不报警 则不报警*/
				if(temp_alarm_status != 0)
				{
					for(index = 0; index < temp_num; index ++){
						if(temp_status[index] != 0){
							break;
						}
					}
					if(index == temp_num){
						temp_alarm_status = 0;
						temp_alarm_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}
			
				if(temp_warn_status != 0 && temp_alarm_status == 0)
				{
					for(index = 0; index < temp_num; index ++){
						if(temp_status[index] == 1){
							temp_alarm_status = temp_warn_status;
							temp_warn_status = 0;
							break;
						}
						if(temp_status[index] != 0){
							break;
						}
					}
					if(index == temp_num){
						temp_warn_status = 0;
						temp_warn_time = 0;
						clearalarm();
						cancel_alarm();
					}
				}

		/****************************************************************************************/
				/*     开始进行预警报警行动判断     */
				if(dhlr_warn_status != 0 || dhlr_alarm_status != 0) //动火离人
				{
					if(dhlr_warn_status != 0)//动火预警
					{
						//时间大于预警延迟//无动火预警行为
						ltimestamp = rtc_counter_get();
						if((dhlr_warn_time == 0) && (ltimestamp - dhlr_warn_ltimestamp > p_configuration_data->warn_delay))
						{	
							printf("dhlr warn > delay action\r\n");
						
							//记录预警时间
							dhlr_warn_time = rtc_counter_get();
							//上报预警信息
							sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_WARN");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d", ltimestamp - dhlr_warn_ltimestamp);
							set_alarm_data();
							//报警
							alarm_action(0);
							cancel_flag = 0;
						}
						//有动火预警行为
						// 动火预警时间 大于 预警时长 取消预警
						ltimestamp = rtc_counter_get();
						if((dhlr_warn_time != 0) && (dhlr_alarm_time == 0) && cancel_flag == 0 &&
							(ltimestamp - dhlr_warn_time > p_configuration_data->warn_time))
						{  
							printf("dhlr warn cancel action\r\n");
							cancel_alarm();
							cancel_flag = 1;
							clearalarm();
						}
					
						// 动火时间 大于 报警延迟 报警
						ltimestamp = rtc_counter_get();
						if((dhlr_warn_time != 0) && (dhlr_alarm_time == 0) && 
							(ltimestamp - dhlr_warn_time > p_configuration_data->alarm_delay))
						{
							printf("dhlr warn -> alarm action\r\n");
							//进入动火离人报警状态
							dhlr_alarm_status = dhlr_warn_status;
							dhlr_alarm_time = rtc_counter_get();
							// 报警
							sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_ALARM");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d", ltimestamp - dhlr_warn_time);
							//退出预警状态
							dhlr_warn_status = 0;
							dhlr_warn_time = 0;
							temp_warn_status = 0;
							temp_warn_time = 0;
							set_alarm_data();
							alarm_action(1);
							cancel_flag = 0;
						}
					}  // end if(dhlr_warn_status == 1)
					else if(dhlr_alarm_status != 0) // 动火报警
					{
						// 断电断气
						ltimestamp = rtc_counter_get();
						if((dhlr_alarm_time != 0) && (ltimestamp - dhlr_alarm_time > p_configuration_data->break_delay))
						{
							key_process_cmd(0, 0, 0); //断电断气
						}
						// 动火报警时间大于报警时长
						ltimestamp = rtc_counter_get();
						if((dhlr_alarm_time != 0) && cancel_flag == 0 && (ltimestamp - dhlr_alarm_time > p_configuration_data->alarm_time))
						{
							printf("dhlr alarm cancel \r\n");
							cancel_alarm(); //取消报警
							cancel_flag = 1;
							clearalarm();
						
							dhlr_alarm_status = 0;
							dhlr_alarm_time = 0;
							dhlr_warn_status = 0;
							dhlr_warn_time = 0;

							temp_alarm_status = 0;
							temp_alarm_time = 0;
							temp_warn_status = 0;
							temp_warn_time = 0;
						}
					}  // end else if(dhlr_alarm_status == 1)
				} // end if(dhlr_warn_status == 1 || dhlr_alarm_status == 1)
			
				else  // 没有动火
				{
					/* 高温预警 */
					if(temp_warn_status != 0 && dhlr_alarm_status == 0 && gas_alarm_status == 0 && smoke_alarm_status == 0
						&& temp_alarm_status == 0 && dhlr_warn_status == 0 && (key_status == key_status_back))
					{
						//温度预警时间大于预警延迟 
						ltimestamp = rtc_counter_get();
						if((temp_warn_time == 0) && (ltimestamp - temp_warn_ltimestamp > p_configuration_data->warn_delay))
						{  
							printf("temp warn > delay \r\n");
							//预警
							temp_warn_time = rtc_counter_get();
						
							sprintf(alarmdata_report.alarm_type.value, "%s", "OIL_TEMP_WARN");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].param / 10) % 1000, 
							heartbeat_reportdata.heart_data.temperature.sensors[temp_warn_status - 1].param % 10);
							set_alarm_data();
							alarm_action(2);
							cancel_flag = 0;
						}
						// 有温度预警行为//温度预警时间大于预警时长
						ltimestamp = rtc_counter_get();
						if((temp_warn_time != 0) && cancel_flag == 0 && (ltimestamp - temp_warn_time > p_configuration_data->warn_time))
						{
							printf("cancel temp warn \r\n");
							cancel_alarm();
							cancel_flag = 1;
							clearalarm();
						}
					
						//温度预警时间大于报警延迟
						ltimestamp = rtc_counter_get();
						if((temp_warn_time != 0) && (ltimestamp - temp_warn_time > p_configuration_data->alarm_delay))
						{	
							printf("temp warn -> alarm action \r\n");
							//进入高温报警状态
							temp_alarm_status = temp_warn_status;
							temp_alarm_time = rtc_counter_get();
							//退出预警状态
							temp_warn_status = 0;
							temp_warn_time = 0;
							//报警
						
							sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C",heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param / 10) % 1000, 
								heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param % 10);
							set_alarm_data();
							alarm_action(3);
							cancel_flag = 0;
						}
					}
				
					/* 高温报警 */
					if(temp_alarm_status != 0 && (key_status == key_status_back))
					{
						//高温报警时间大于报警延迟//无报警行为
						ltimestamp = rtc_counter_get();
						if((temp_alarm_time == 0) && (ltimestamp - temp_alarm_ltimestamp > p_configuration_data->alarm_delay))
						{
							printf("temp alarm action start \r\n");
							temp_alarm_time = rtc_counter_get();

							sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "C", heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param / 10) % 1000, 
								heartbeat_reportdata.heart_data.temperature.sensors[temp_alarm_status - 1].param % 10);
							set_alarm_data();
							alarm_action(3);
							cancel_flag = 0;
						}
						// 断电断气
						ltimestamp = rtc_counter_get();
						if((temp_alarm_time != 0) && (ltimestamp - temp_alarm_time > p_configuration_data->break_delay))
						{
							key_process_cmd(0, 0, 0); //断电断气
						}
						//报警时间大于报警时长
						ltimestamp = rtc_counter_get();
						if((temp_alarm_time != 0) && cancel_flag == 0&& (ltimestamp - temp_alarm_time > p_configuration_data->alarm_time))
						{
							printf("cancel temp alarm action\r\n");
							cancel_alarm();
							cancel_flag = 1;
							clearalarm();

							temp_alarm_status = 0;
							temp_alarm_time = 0;
							temp_warn_status = 0;
							temp_warn_time = 0;
						}
					}
				
					/* 燃气 报警 */
					if(gas_alarm_status != 0 && (key_status == key_status_back))
					{
						//报警时间大于报警延迟//没有报警行为
						ltimestamp = rtc_counter_get();
						if((gas_alarm_time == 0) && (ltimestamp - gas_alarm_ltimestamp > p_configuration_data->alarm_delay))
						{
							printf("gas alarm action\r\n");
							gas_alarm_time = rtc_counter_get();
						
							sprintf(alarmdata_report.alarm_type.value, "%s", "GAS_ALARM");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "R", heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d.%d", heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].param / 100, 
									heartbeat_reportdata.heart_data.gas_detector.sensors[gas_alarm_status - 1].param % 100);
							set_alarm_data();
							alarm_action(4);
							cancel_flag = 0;
						}	
						// 断电断气
						ltimestamp = rtc_counter_get();
						if((gas_alarm_time != 0) && (ltimestamp - gas_alarm_time > p_configuration_data->break_delay))
						{
							key_process_cmd(0, 0, 0); //断电断气
						}
						//报警时间大于报警时长
						ltimestamp = rtc_counter_get();
						if((gas_alarm_time != 0) && cancel_flag == 0 && (ltimestamp - gas_alarm_time > p_configuration_data->alarm_time))
						{
							printf("cancel gas alarm action\r\n");
							cancel_alarm();
							cancel_flag = 1;
							clearalarm();

							gas_alarm_status = 0;
							gas_alarm_time = 0;
						}
					}
				
					/* 烟雾报警 */
					if(smoke_alarm_status != 0 && (key_status == key_status_back))
					{
						//报警时间大于报警延迟//没有报警行为
						ltimestamp = rtc_counter_get();
						if((smoke_alarm_time == 0) && (ltimestamp - smoke_alarm_ltimestamp > p_configuration_data->alarm_delay))
						{
							printf("smoke alarm action\r\n");
							smoke_alarm_time = rtc_counter_get();
						
							sprintf(alarmdata_report.alarm_type.value, "%s", "SMOKE_ALARM");
							sprintf(alarmdata_report.sensor_id.value, "%s%s%x", alarmdata_report.serial_number.value, "Y",heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].device_id + 1);
							sprintf(alarmdata_report.alarm_param.value, "%d.%d", heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].param / 100, 
									heartbeat_reportdata.heart_data.smoke_detector.sensors[smoke_alarm_status - 1].param % 100);
							set_alarm_data();
							alarm_action(5);
							cancel_flag = 0;
						}
						// 断电断气
						ltimestamp = rtc_counter_get();
						if((smoke_alarm_time != 0) && (ltimestamp - smoke_alarm_time > p_configuration_data->break_delay))
						{
							key_process_cmd(0, 0, 0); //断电断气
						}
						//报警时间大于报警时长
						ltimestamp = rtc_counter_get();
						if((smoke_alarm_time != 0) && cancel_flag == 0 &&(ltimestamp - smoke_alarm_time > p_configuration_data->alarm_time))
						{
							printf("cancel smoke alarm action\r\n");
							cancel_alarm();
							cancel_flag = 1;
							clearalarm();
							smoke_alarm_status = 0;
							smoke_alarm_time = 0;
						}
					}
				} // end else //没有动火	
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
    }
}



void rs485net_task_init(void)
{

	uint8_t i = 0, j = 0;
//	show_data(p_configuration_data);
//	printf("------------------- rs485net_task_init start ----------------\r\n");

	deviceList[device_num].bus = EVAL_COM0;
    deviceList[device_num].address = 0x002F;
    deviceList[device_num].frequency = 0xffff;
	device_num++;

	for(i = 0; i < 4; i++)
	{
		/* 解析温度传感器端口地址 */
		if(p_configuration_data->temp_port[i] != 0)
		{
//			printf("---------- temp ----------\r\n");
			for(j = 0; j < 32; j ++){
				if(p_configuration_data->temp_port[i] & (0x01 << j)){
					deviceList[device_num].bus = EVAL_COM[i];
					deviceList[device_num].address = j + 0x00;
					deviceList[device_num].frequency = 1;
//					printf("device[%d] bus %x\ttemp port %d\taddr %X\r\n", device_num, deviceList[device_num].bus, i, deviceList[device_num].address);
					device_num ++;
				}
			}
		}
		/* 解析燃气传感器端口地址 */
		if(p_configuration_data->gas_port[i] != 0)
		{
//			printf("---------- gas ----------\r\n");
			for(j = 0; j < 32; j ++){
				if(p_configuration_data->gas_port[i] & (0x01 << j)){
					deviceList[device_num].bus = EVAL_COM[i];
					deviceList[device_num].address = j + 0x40;
					deviceList[device_num].frequency = 1;
//					printf("device[%d] bus %x\tgas port %d\taddr %X\r\n", device_num, deviceList[device_num].bus, i, deviceList[device_num].address);
					device_num++;
				}
			}
		}
		/* 解析烟雾传感器端口地址 */
		if(p_configuration_data->smoke_port[i] != 0)
		{
//			printf("---------- smoke ----------\r\n");
			for(j = 0; j < 32; j ++){
				if(p_configuration_data->smoke_port[i] & (0x01 << j)){
					
					deviceList[device_num].bus = EVAL_COM[i];
					deviceList[device_num].address = j + 0x80;
					deviceList[device_num].frequency = 1;
//					printf("device[%d] bus %x\tsmoke port %d\taddr %X\r\n", device_num, deviceList[device_num].bus, i, deviceList[device_num].address);
					device_num ++;
				}
			}
		}
		/* 解析pir端口地址 */
		if(p_configuration_data->pir_port[i] != 0)
		{
//			printf("---------- pir ----------\r\n");
			for(j = 0; j < 32; j ++){
				if(p_configuration_data->pir_port[i] & (0x01 << j)){
					deviceList[device_num].bus = EVAL_COM[i];
					deviceList[device_num].address = j + 0xC0;
					deviceList[device_num].frequency = 1;
//					printf("device[%d] bus %x\tpir port %d\taddr %X\r\n", device_num, deviceList[device_num].bus, i, deviceList[device_num].address);
					device_num++;
				}
			}
		}
	}

    xTaskCreate(rs485net_task, "RS485NET", DEFAULT_THREAD_STACKSIZE * 4, NULL, RS485NET_TASK_PRIO, &NetTaskHandle);
}

void rs485net_task_start(void)
{
	uint32_t ulValueToSend = 100UL;
    NettoStop = 0;
    xTaskNotify(NetTaskHandle, ulValueToSend, eSetValueWithOverwrite);
}

void rs485net_task_stop(void)
{
	if (NettoStop == 0)
    {
        NettoStop = 1;
    }
}

int uart0_get_tmperature_data()
{
	int i = 0;
    uint8_t Asdata[4] = {0};
    uint8_t Brdata[4] = {0};
	
	
	rs485net_task_stop();    


	/* 初始化发送数据 */
	for (i = 0; i < 4; i++)
	{
		Asdata[i] = 0x30 + i;
	}
	

	while(usart_recv_pre(EVAL_COM1, 4) < 0);
	while(usart_send(EVAL_COM0, (uint8_t *) Asdata, 4) < 0);	
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Asend_data[%d]: %d\n", i, Asdata[i]);
//	}
	
	while (usart_recv(EVAL_COM1, Brdata) < 0)
	{
		;
	}
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Brdata[%d]: %d\n", i, Brdata[i]);
//	}
	
	
	//if(memcmp(Asdata, Brdata, sizeof(Asdata)) == 0)
	if(Brdata[0] != 0 || Brdata[1] != 0 || Brdata[2] != 0 || Brdata[3] != 0)
	{
		//printf("data and rdata are equal\n");
		return 1;
	} 
	else{
		//printf("data and rdata are not equal\n");
		return 0;
	}
	
	//printf("test pass \r\n");
	//rs485net_task_start();
    		
}

int uart1_get_tmperature_data()
{
	int i = 0;
    uint8_t Asdata[4] = {0};
    uint8_t Brdata[4] = {0};
	
	//printf("test here \r\n");
	rs485net_task_stop();    


	/* 初始化发送数据 */
	for (i = 0; i < 4; i++)
	{
		Asdata[i] = 0x30 + i;
	}
	

	while(usart_recv_pre(EVAL_COM0, 4) < 0);
	while(usart_send(EVAL_COM1, (uint8_t *) Asdata, 4) < 0);	
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Asend_data[%d]: %d\n", i, Asdata[i]);
//	}
	
	while (usart_recv(EVAL_COM0, Brdata) < 0)
	{
		;
	}
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Brdata[%d]: %d\n", i, Brdata[i]);
//	}
	
	
	if(Brdata[0] != 0 || Brdata[1] != 0 || Brdata[2] != 0 || Brdata[3] != 0)
	{
		//printf("data and rdata are equal\n");
		return 1;
	} 
	else{
		//printf("data and rdata are not equal\n");
		return 0;
	}
	
}



int uart3_get_tmperature_data()
{
	int i = 0;
    uint8_t Asdata[4] = {0};
    uint8_t Brdata[4] = {0};
	
	
	//printf("test here \r\n");
	rs485net_task_stop();    

	/* 初始化发送数据 */
	for (i = 0; i < 4; i++)
	{
		Asdata[i] = 0x30 + i;
	}
	

	while(usart_recv_pre(EVAL_COM4, 4) < 0);
	while(usart_send(EVAL_COM3, (uint8_t *) Asdata, 4) < 0);	
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Asend_data[%d]: %d\n", i, Asdata[i]);
//	}
	
	while (usart_recv(EVAL_COM4, Brdata) < 0)
	{
		;
	}
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Brdata[%d]: %d\n", i, Brdata[i]);
//	}
	
	
	//if(memcmp(Asdata, Brdata, sizeof(Asdata)) == 0)
	if(Brdata[0] != 0 || Brdata[1] != 0 || Brdata[2] != 0 || Brdata[3] != 0)
	{
		//printf("data and rdata are equal\n");
		return 1;
	} 
	else{
		//printf("data and rdata are not equal\n");
		return 0;
	}
	
	
	//printf("test pass \r\n");
	//rs485net_task_start();
    		
}


int uart4_get_tmperature_data()
{
//	int i = 0;
    uint8_t Asdata[4] = {0x30, 0x31, 0x32, 0x33};
    uint8_t Brdata[4] = {0};
	
	
	//printf("test here \r\n");
	rs485net_task_stop();    

//	/* 初始化发送数据 */
//	for (i = 0; i < 4; i++)
//	{
//		Asdata[i] = 0x30 + i;
//	}
//	

	while(usart_recv_pre(EVAL_COM3, 4) < 0);
	while(usart_send(EVAL_COM4, (uint8_t *) Asdata, 4) < 0);	
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Asend_data[%d]: %d\n", i, Asdata[i]);
	
//	}
	
	//for(i = 0; i < 1000; i++)
	
	while (usart_recv(EVAL_COM3, Brdata) < 0)
	{
		;
	}
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("Brdata[%d]: %d\n", i, Brdata[i]);
//	}
	
	
	//if(memcmp(Asdata, Brdata, sizeof(Asdata)) == 0) 
	if(Brdata[0] != 0 || Brdata[1] != 0 || Brdata[2] != 0 || Brdata[3] != 0)
	{
		//printf("data and rdata are equal\n");
		return 1;
	} 
	else{
		//printf("data and rdata are not equal\n");
		return 0;
	}
    		
}

int get_tmperature_data()
{
	int i = 0;
	int equal = 1;
    uint8_t data[32] = {0};
    uint8_t rdata[32] = {0};
    
    //printf("test here \r\n");
	rs485net_task_stop();
    
	/* 初始化发送数据 */
	for (i = 0; i < 32; i++)
	{
		data[i] = 0x30 + i;
	}
	usart_recv_clear(EVAL_COM0);
	
	while(usart_recv_pre(EVAL_COM0, 4) < 0);
	Delay_ms(1);
	while(usart_send(EVAL_COM1, (uint8_t *) data, 4) < 0);
	Delay_ms(2);
	
	while (usart_recv(EVAL_COM0, rdata) < 0)
	{
	}

//	for(i = 0; i < 4; i++)
//	{
//		printf("send_data[%d]: %d\n", i, data[i]);
//	
//	}
	
//	for(i = 0; i < 4; i++)
//	{
//		printf("recv_data[%d]: %d\n", i, rdata[i]);
//	}
	
	for (i = 1; i < 4; i++) {
        if (data[i] != rdata[i]) {
            equal = 0;
            break;
        }
    }

    if (equal) {
        //printf("data and rdata are equal\n");
		return 1;
    } else {
       // printf("data and rdata are not equal\n");
		return 0;
    }
}

void set_camera_status(uint16_t channel, uint16_t status)
{	
	if(channel == 0 && p_configuration_data->channel0.dh_channel == 1){
		if(status == 0){
			camera_status[0] = 1; 
		}else{
			camera_status[0] = 0; 
		}
	}
	else if(channel == 1 && p_configuration_data->channel1.dh_channel == 1){
		if(status == 0){
			camera_status[1] = 1; 
		}else{
			camera_status[1] = 0; 
		}
	}
	else if(channel == 2 && p_configuration_data->channel2.dh_channel == 1){ 
		if(status == 0){
			camera_status[2] = 1; 
		}else{
			camera_status[2] = 0; 
		}
	}
	else if(channel == 3 && p_configuration_data->channel3.dh_channel == 1){
		if(status == 0){
			camera_status[3] = 1; 
		}else{
			camera_status[3] = 0; 
		}
	}
}

