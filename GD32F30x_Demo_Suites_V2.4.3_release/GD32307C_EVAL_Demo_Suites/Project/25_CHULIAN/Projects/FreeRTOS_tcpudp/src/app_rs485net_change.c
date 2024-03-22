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

#define RS485NET_TASK_PRIO  ( tskIDLE_PRIORITY + 5 )

extern struct eeprom_data *p_configuration_data;

struct rs485net_timing_t
{
    uint32_t bus;
    uint16_t frequency;
    uint16_t address;
} deviceList[4];

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

int led_status = 0;
int audio_num = 0;

void cancel_alarm()
{	
	led_status &= 0xdf;
	led_process_cmd(0, &led_status, 0);
	audio_process_cmd(0, 0, 0);
	audio_process_cmd(5, 0, 0);
	
}

void alarm_action(uint8_t n)
{
	//蜂鸣器
	audio_process_cmd(4, 0, 0);
	//报警灯
	led_status |= 0x30;
	printf("led status %d\r\n", led_status);
	led_process_cmd(0, &led_status,0);
	//语音播放
	set_num(n);  //动火离人预警
	audio_process_cmd(1, 0, 0);
}

void set_alarm_data()
{
	alarmdata_report.data_type = 1;
	alarmdata_report.alarm_type.time_stamp = rtc_counter_get();
	alarmdata_report.alarm_param.time_stamp = alarmdata_report.alarm_type.time_stamp;
}

void set_warn_data()
{
	warndata_report.data_type = 1;
	warndata_report.alarm_type.time_stamp = rtc_counter_get();
	warndata_report.alarm_param.time_stamp = warndata_report.alarm_type.time_stamp;
}

static void rs485net_task(void *arg)
{
    uint32_t count = 0;
    uint32_t ltimestamp = 0, temp_warn_time = 0, temp_alarm_time = 0, left_ltimestamp = 0;
	uint32_t dhlr_warn_time = 0, dhlr_alarm_time = 0, gas_alarm_time = 0, smoke_alarm_time = 0;
    uint8_t index = 0;
    uint16_t retry = 0;
    unsigned char rdata[8] = {0};
    char display_buf[7];
    int key_status = 0;
	int adc_value[4] = {0};  //add by heyt 20240304
	
	uint8_t smoke_alarm_status = 0, gas_alarm_status = 0;
	uint8_t temp_warn_status = 0, temp_alarm_status = 0;
	uint8_t dhlr_warn_status = 0, dhlr_alarm_status = 0;
	uint8_t lr_status = 0, adc_status = 0;
	uint32_t smoke_alarm_ltimestamp = 0, gas_alarm_ltimestamp = 0;
	uint32_t temp_warn_ltimestamp = 0, temp_alarm_ltimestamp = 0;
	uint32_t dhlr_warn_ltimestamp = 0, dhlr_alarm_ltimestamp = 0;
	
	
    memcpy(heartbeat_reportdata.heart_data.serial_number.value, "72-0066DF313436-3331", 21);
    ltimestamp = rtc_counter_get();
    while (1)
    {
        // get key status
        key_status = get_key_value();
        printf("keybitmap:%02x\r\n", key_status);
        if ((key_status & 0x01) == 0)
        {	
			cancel_alarm();
			//add by heyt 20240306 start ----------------------
			smoke_alarm_status = 0;
			gas_alarm_status = 0;
			temp_warn_status = 0;
			temp_alarm_status = 0;
			dhlr_warn_status = 0;
			dhlr_alarm_status = 0;
			lr_status = 0;
			adc_status = 0;
			//add by heyt 20240306 end ------------------------
        }
        
        memset(rdata, 0x00, 8);

        heartbeat_reportdata.heart_data.serial_number.time_stamp = rtc_counter_get();
//        printf("timestamp: %d\r\n", heartbeat_reportdata.heart_data.serial_number.time_stamp);
        for (index = 0; index < 4; index++)
        {
            if ((count % deviceList[index].frequency) == 0)
            {
                while (usart_send(deviceList[index].bus, (uint8_t *)packed_modbus_rtu_frame(deviceList[index].address, 0x03, 0x0000, 0x0001), 8) < 0)
                {
//                    retry++;
//                    vTaskDelay(pdMS_TO_TICKS(10));
//                    if (retry > 100)
//                    {
//                        retry = 0;
//                        printf("usart_send retry over 100 times \r\n");
//                        break;
//                    }
                }

                // delay 1ms to make sure send finish
                vTaskDelay(pdMS_TO_TICKS(1));

                usart_recv_clear(deviceList[index].bus);
                while (usart_recv_pre(deviceList[index].bus, 7) < 0)
                {
                    retry++;
                    vTaskDelay(pdMS_TO_TICKS(10));
                    if (retry > 100)
                    {
                        retry = 0;
                        printf("usart_recv_pre retry over 100 times \r\n");
                        break;
                    }
                }

                while (usart_recv(deviceList[index].bus, rdata) < 0)
                {
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
                        led_process_cmd(0, &led_status,0);
//                        printf("slave_address:%02x, function_code:%02x, byte_count:%02x, ", ((ModbusRtuFrameAck *)rdata)->slave_address, ((ModbusRtuFrameAck *)rdata)->function_code, ((ModbusRtuFrameAck *)rdata)->byte_count);
//                        printf("data[0]:%02x, data[1]:%02x\r\n", ((ModbusRtuFrameAck *)rdata)->data[0], ((ModbusRtuFrameAck *)rdata)->data[1]);
						
                        switch (((ModbusRtuFrameAck *)rdata)->slave_address >> 6)
                        {
                        case 0x00: // smoker
                            sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
                            // TODO:display smoker value
                            lcd_display(4, 5, display_buf);

                            // TODO: upload smoker sensor's value
                            heartbeat_reportdata.heart_data.smoke_detector.time_stamp = rtc_counter_get();
                            sprintf(heartbeat_reportdata.heart_data.smoke_detector.sensors[0].device_id, "%d", ((ModbusRtuFrameAck *)rdata)->slave_address);
                            heartbeat_reportdata.heart_data.smoke_detector.sensors[0].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];

                            // TODO: compare with alram th
                            printf("smoker value: %d\r\n", heartbeat_reportdata.heart_data.smoke_detector.sensors[0].param);
							
                            if ((key_status&0x01) && (heartbeat_reportdata.heart_data.smoke_detector.sensors[0].param > 2000))//p_configuration_data->smoke_threshold))
                            {
								printf("smoke alarm status = 1\r\n");
								if(smoke_alarm_status == 0){
									smoke_alarm_status = 1;
									smoke_alarm_ltimestamp = rtc_counter_get();
								}
                            }else{
								smoke_alarm_status = 0;
								smoke_alarm_ltimestamp = 0;
							}
                            break;
							
                        case 0x01: // gas
                            sprintf(display_buf, "%02d.%02d", ((ModbusRtuFrameAck *)rdata)->data[0] % 100, ((ModbusRtuFrameAck *)rdata)->data[1] % 100);
                            // TODO:display gas value
                            lcd_display(3, 5, display_buf);
                            // TODO: compare with alram th
                            // TODO: upload smoker sensor's value
                            heartbeat_reportdata.heart_data.gas_detector.time_stamp = rtc_counter_get();
                            sprintf(heartbeat_reportdata.heart_data.gas_detector.sensors[0].device_id, "%d", ((ModbusRtuFrameAck *)rdata)->slave_address);
                            heartbeat_reportdata.heart_data.gas_detector.sensors[0].param = ((ModbusRtuFrameAck *)rdata)->data[0] * 100 + ((ModbusRtuFrameAck *)rdata)->data[1];
							printf("gas value %d\r\n",  heartbeat_reportdata.heart_data.gas_detector.sensors[0].param);
						
							if ((key_status&0x01) && (heartbeat_reportdata.heart_data.gas_detector.sensors[0].param > 2000))//p_configuration_data->gas_threshold))
                            {
								printf("gas alarm status = 1\r\n");
								if(gas_alarm_status == 0){
									gas_alarm_status = 1;
									gas_alarm_ltimestamp = rtc_counter_get();
								}
                            }
							else{
								gas_alarm_status = 0;
								gas_alarm_ltimestamp = 0;
							}
                            break;
							
                        case 0x02: // tempeture
                            heartbeat_reportdata.heart_data.temperature.sensors[0].param = ((ModbusRtuFrameAck *)rdata)->data[0] << 8 | ((ModbusRtuFrameAck *)rdata)->data[1];
                            
                            sprintf(display_buf, " %03d.%d", (heartbeat_reportdata.heart_data.temperature.sensors[0].param / 10) % 1000, heartbeat_reportdata.heart_data.temperature.sensors[0].param % 10);
                            // TODO:display tempeture value
                            lcd_display(2, 4, display_buf);

                            // TODO: upload tempeture sensor's value
                            heartbeat_reportdata.heart_data.temperature.time_stamp = rtc_counter_get();
                            sprintf(heartbeat_reportdata.heart_data.temperature.sensors[0].device_id, "%d", ((ModbusRtuFrameAck *)rdata)->slave_address);
                            // TODO: compare with alram th
                        	printf("temperature  %d \r\n", heartbeat_reportdata.heart_data.temperature.sensors[0].param);
							// edit by heyt 20240305 start ------------
							//温度大于报警温度 不管有无人 报警
							if ((key_status&0x01) && ((heartbeat_reportdata.heart_data.temperature.sensors[0].param / 10) > 200))//(p_configuration_data->alarm_temp - p_configuration_data->temp_calibrate)))
                            {
								printf("tempeture alarm status = 1\r\n");
								if(temp_alarm_status == 0){
									temp_alarm_status = 1;
									if(temp_warn_status == 1){
										temp_warn_status = 0;
									}
									temp_alarm_ltimestamp = rtc_counter_get();
								}
                            }
							else if((key_status&0x01) && ((heartbeat_reportdata.heart_data.temperature.sensors[0].param / 10) > 100))//(p_configuration_data->warn_temp - p_configuration_data->temp_calibrate)))
                            {
								printf("tempeture warn status = 1\r\n");
								if(temp_warn_status == 0 && temp_alarm_status == 0){
									temp_warn_status = 1;
									temp_warn_ltimestamp = rtc_counter_get();
								}
                            }
							else{
								temp_warn_status = 0;
								temp_warn_ltimestamp = 0;
								temp_warn_time = 0;
								temp_alarm_status = 0;
								temp_alarm_ltimestamp = 0;
							}
							break;
							// edit by heyt 20240305 end ------------
                        case 0x03: // pir
                            // TODO:display pir value
                            if (((ModbusRtuFrameAck *)rdata)->data[1] == 1)
                            { // 有人，清空离人时间
                                heartbeat_reportdata.heart_data.body_detector.leave_time_stamp = 0;
                                heartbeat_reportdata.heart_data.body_detector.status = ((ModbusRtuFrameAck *)rdata)->data[1];
                                sprintf(display_buf, " 000");
                                lcd_display(1, 5, display_buf);
								lr_status = 0;
                            }
                            else
                            { // 没人，且上次也没人
                                if (heartbeat_reportdata.heart_data.body_detector.status == ((ModbusRtuFrameAck *)rdata)->data[1])
                                {
                                    heartbeat_reportdata.heart_data.body_detector.leave_time_stamp = heartbeat_reportdata.heart_data.body_detector.time_stamp - left_ltimestamp;
                                }
                                else // 没人，上次有人
                                {	
                                    // person left detected, save timestamp
                                    left_ltimestamp = rtc_counter_get();
                                }
								lr_status = 1;
                                heartbeat_reportdata.heart_data.body_detector.status = ((ModbusRtuFrameAck *)rdata)->data[1];
                                heartbeat_reportdata.heart_data.body_detector.time_stamp = rtc_counter_get();
                            }
							// 显示时间
                            if ((heartbeat_reportdata.heart_data.body_detector.leave_time_stamp > 0) && (heartbeat_reportdata.heart_data.body_detector.leave_time_stamp < 1000))
                            {
                                sprintf(display_buf, " %03d", heartbeat_reportdata.heart_data.body_detector.leave_time_stamp);
                                lcd_display(1, 5, display_buf);
								printf("left time %d\r\n", heartbeat_reportdata.heart_data.body_detector.leave_time_stamp);
//                                if ((key_status & 0x01) && (heartbeat_reportdata.heart_data.body_detector.leave_time_stamp > (p_configuration_data->warn_time + p_configuration_data->alarm_time)))
//                                {
//                                    audio_process_cmd(4, 0, 0);
//                                    led_status |= 0x20;
//                                    led_process_cmd(0, &led_status,0);
//                                }
                            }
                            // TODO: compare with alram th
                            // TODO: upload smoker sensor's value
                            break;
                        default:
                            break;
                        }

                    }
                }
//                printf("%02x %02x %02x %02x %02x %02x %02x %02x\r\n", rdata[0], rdata[1], rdata[2], rdata[3], rdata[4], rdata[5], rdata[6], rdata[7]);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        count++;
		// TODO:ADC read
		// 取adc 电流最大值
		adc_process_cmd(0, adc_value, 4);
		heartbeat_reportdata.heart_data.fire_sampler.code_data = 0;
		for(index = 0; index < 4; index++){
			if(adc_value[index] > heartbeat_reportdata.heart_data.fire_sampler.code_data){
				heartbeat_reportdata.heart_data.fire_sampler.code_data = adc_value[index];
			}
		}
		heartbeat_reportdata.heart_data.fire_sampler.code_data = 100;   //---------------
		if(heartbeat_reportdata.heart_data.fire_sampler.code_data > 2000){//p_configuration_data->hot_work_threshold){
			adc_status = 1;
		}
		
		
		/*    判断动火状态    */
		//动火报警
		if((key_status & 0x01) && (lr_status == 1) && (temp_alarm_status == 1 || adc_status == 1))
		{
			printf("dhlr alarm status = 1\r\n");
			if(dhlr_alarm_status == 0){
				dhlr_alarm_status = 1;
				if(dhlr_warn_status == 1){
					dhlr_warn_status = 0;
				}
				dhlr_alarm_ltimestamp = rtc_counter_get();
			}
		}
		//动火预警
		else if((key_status & 0x01) && (lr_status == 1) && (temp_warn_status == 1 || adc_status == 1))
		{
			printf("dhlr warn status = 1\r\n");
			if(dhlr_warn_status == 0 && dhlr_alarm_status == 0){
				dhlr_warn_status = 1;
				dhlr_warn_ltimestamp = rtc_counter_get();
			}
		}
		//没有动火
		else if(!(key_status & 0x01) || (lr_status == 0) || 
			(temp_alarm_status == 0 && temp_warn_status == 0 && adc_status == 0))
		{
			printf("dhlr status = 0\r\n");
			dhlr_warn_status = 0;
			dhlr_warn_ltimestamp = 0;
			dhlr_alarm_status = 0;
			dhlr_alarm_ltimestamp = 0;
		}
		
		//动火预警
		if(dhlr_warn_status == 1)
		{
			ltimestamp = rtc_counter_get();
			if(dhlr_warn_time == 0)//无动火预警行为
			{		//时间大于预警延迟
				if((ltimestamp - dhlr_warn_ltimestamp) > 5)//p_configuration_data->warn_delay)
				{	
					printf("dhlr warn > delay action\r\n");
					//预警
					set_warn_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_WARN");
					if(temp_warn_status == 1){
						sprintf(warndata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
					}else if(adc_status == 1){
						sprintf(warndata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.fire_sampler.code_data);
					}
					alarm_action(0);
					//记录预警时间
					dhlr_warn_time = rtc_counter_get();
				}
			}
			else  //有动火预警行为
			{
				if(dhlr_alarm_time == 0) // 没有动火报警行为
				{
					ltimestamp = rtc_counter_get(); 
					// 动火预警时间 大于 预警时长 取消预警
					if(ltimestamp - dhlr_warn_time > 5)//p_configuration_data->warn_time)
					{  
						printf("dhlr warn cancel action\r\n");
						cancel_alarm();
					}
					// 动火时间 大于 报警延迟 报警
					else if(ltimestamp - dhlr_warn_time > 10)//p_configuration_data->alarm_delay)
					{
						printf("dhlr warn -> alarm action\r\n");
						// 报警
						set_alarm_data();
						sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_ALARM");
						if(temp_alarm_status == 1){
							sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
						}else if(adc_status == 1){
							sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.fire_sampler.code_data);
						}
						alarm_action(1);
						//退出预警状态
						dhlr_warn_status = 0;
						dhlr_warn_time = 0;
						//进入报警状态
						dhlr_alarm_status = 1;
						dhlr_alarm_time = rtc_counter_get();
					}
				}
			}
		}
		
		if(dhlr_alarm_status == 1) // 动火报警
		{
			ltimestamp = rtc_counter_get();
			if(dhlr_alarm_time == 0) //没有报警行为
			{  //动火时间 大于 报警延迟
				if(ltimestamp - dhlr_alarm_ltimestamp > 10)//p_configuration_data->alarm_delay)
				{	
					printf("dhlr alarm action start \r\n");
					//报警
					set_alarm_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_ALARM");
					if(temp_alarm_status == 1){
						sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
					}else if(adc_status == 1){
						sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.fire_sampler.code_data);
					}
					alarm_action(1);
					//记录报警时间
					dhlr_alarm_time = rtc_counter_get();
				}
			}
			else
			{	// 动火报警时间大于报警时长
				if(ltimestamp - dhlr_alarm_time > 10)//p_configuration_data->alarm_time)
				{
					printf("dhlr alarm cancel \r\n");
					cancel_alarm(); //取消报警
					key_process_cmd(0, 0, 0); //断电断气
					dhlr_alarm_status = 0;
					dhlr_alarm_time = 0;
				}
			}
		}
		
		if(temp_warn_status == 1)
		{
			ltimestamp = rtc_counter_get();
			if(temp_warn_time == 0) // 没有温度预警行为
			{	//温度预警时间大于预警延迟 
				if(ltimestamp - temp_warn_ltimestamp > 5)//p_configuration_data->warn_delay)
				{  
					printf("temp warn > delay \r\n");
					//预警
					set_warn_data();
					sprintf(warndata_report.alarm_type.value, "%s", "OIL_TEMP_WARN");
					sprintf(warndata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
					alarm_action(2);
					temp_warn_time = rtc_counter_get();
				}
			}
			else // 有温度预警行为
			{	//温度预警时间大于预警时长
				if(ltimestamp - temp_warn_time > 5)//p_configuration_data->warn_time)
				{
					printf("cancel temp warn \r\n");
					cancel_alarm();
				}
				//温度预警时间大于报警延迟
				else if(ltimestamp - temp_warn_time > 10)//p_configuration_data->alarm_delay)
				{	
					printf("temp warn -> alarm action \r\n");
					//报警
					set_alarm_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
					sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
					alarm_action(3);
					//退出预警状态
					temp_warn_status = 0;
					temp_warn_time = 0;
					//进入报警状态
					temp_alarm_status = 1;
					temp_alarm_time = rtc_counter_get();
				}
			}
		}	
		
		if(temp_alarm_status == 1)
		{
			ltimestamp = rtc_counter_get();
			if(temp_alarm_time == 0) //无报警行为
			{	//高温报警时间大于报警延迟
				if(ltimestamp - temp_alarm_ltimestamp > 10)//p_configuration_data->alarm_delay)
				{
					printf("temp alarm action start \r\n");
					set_alarm_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "DHLR_OIL_TEMP_HIGH");
					sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.temperature.sensors[0].param / 100);
					alarm_action(3);
					temp_alarm_time = rtc_counter_get();
				}
			}
			else
			{
				if(ltimestamp - temp_alarm_time > 10)//p_configuration_data->alarm_time)
				{
					printf("cancel temp alarm action\r\n");
					cancel_alarm();
					key_process_cmd(0, 0, 0);
					temp_alarm_status = 0;
					temp_alarm_time = 0;
				}
			}
		}
		
		if(gas_alarm_status == 1)
		{
			ltimestamp = rtc_counter_get();
			if(gas_alarm_time == 0)
			{
				if(ltimestamp - gas_alarm_ltimestamp > 10)//p_configuration_data->alarm_delay)
				{
					printf("gas alarm action\r\n");
					set_alarm_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "GAS_ALARM");
					sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.gas_detector.sensors[0].param / 100);
					alarm_action(4);
					gas_alarm_time = rtc_counter_get();
				}
			}
			else
			{
				if(ltimestamp - gas_alarm_time > 10)//p_configuration_data->alarm_time)
				{
					printf("cancel gas alarm action\r\n");
					cancel_alarm();
					key_process_cmd(0, 0, 0);
					gas_alarm_status = 0;
					gas_alarm_time = 0;
				}
			}
		}
		
		if(smoke_alarm_status == 1)
		{
			ltimestamp = rtc_counter_get();
			if(smoke_alarm_time == 0){
				if(ltimestamp - smoke_alarm_ltimestamp > 10)//p_configuration_data->alarm_delay)
				{
					printf("smoke alarm action\r\n");
					set_alarm_data();
					sprintf(alarmdata_report.alarm_type.value, "%s", "SMOKE_ALARM");
					sprintf(alarmdata_report.alarm_param.value, "0.%d", heartbeat_reportdata.heart_data.smoke_detector.sensors[0].param / 100);
					alarm_action(5);
					smoke_alarm_time = rtc_counter_get();
				}
			}
			else
			{
				if(ltimestamp - smoke_alarm_time > 10)//p_configuration_data->alarm_time)
				{
					printf("cancel smoke alarm action\r\n");
					cancel_alarm();
					key_process_cmd(0, 0, 0);
					smoke_alarm_status = 0;
					smoke_alarm_time = 0;
				}
			}
		}
		
		vTaskDelay(pdMS_TO_TICKS(100));
		//add by heyt 20240306 end ----------------------
		
//        // TODO: 电池阀状态 相关应用
//        heartbeat_reportdata.heart_data.power_valves.time_stamp = rtc_counter_get();
//        heartbeat_reportdata.heart_data.power_valves.status = 0;

//        // TODO: 动火传感器码值处理
//        heartbeat_reportdata.heart_data.fire_sampler.time_stamp = rtc_counter_get();
//        heartbeat_reportdata.heart_data.fire_sampler.code_data = 800;

    }

}

void rs485net_task_init(void)
{
    deviceList[0].bus = EVAL_COM4;
    deviceList[0].address = 0x0041;
    deviceList[0].frequency = 0x01;
    deviceList[1].bus = EVAL_COM4;
    deviceList[1].address = 0x00c3;
    deviceList[1].frequency = 0x01;
    deviceList[2].bus = EVAL_COM4;
    deviceList[2].address = 0x0000;
    deviceList[2].frequency = 0x01;
    deviceList[3].bus = EVAL_COM4;
    deviceList[3].address = 0x0082;
    deviceList[3].frequency = 0x01;
    xTaskCreate(rs485net_task, "RS485NET", DEFAULT_THREAD_STACKSIZE * 3, NULL, RS485NET_TASK_PRIO, NULL);
}
