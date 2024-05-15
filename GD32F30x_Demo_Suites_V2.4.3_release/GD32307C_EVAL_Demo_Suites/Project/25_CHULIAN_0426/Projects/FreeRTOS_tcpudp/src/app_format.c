#include "protocol_rs485.h"
#include "protocol_usart.h"
#include "protocol_mqtt.h"
#include "app_format.h"
#include "app_configure.h"
#include "module_gd25qxx.h"
#include "module_audio.h"
#include "module_key.h"
#include "app_rs485net.h"
#include <stdio.h>
#include <string.h>

HeartbeatDataReport heartbeat_reportdata;
AlarmDataReport alarmdata_report;

static char buf[MAX_JSON_STRING_LEN];

extern uint8_t temp_count;
extern uint8_t smoke_count;
extern uint8_t gas_count;

extern struct config_data *p_configuration_data;

char * build_json_string()
{
    int pos = 0, i = 0;
    
    /* 构造JSON格式的字符串 */
    pos += sprintf(&buf[pos], "{");
    pos += sprintf(&buf[pos], "\"data_type\":\"event\",");
    pos += sprintf(&buf[pos], "\"stream_id\":\"heartData\",");
    pos += sprintf(&buf[pos], "\"data\":{");

    pos += sprintf(&buf[pos], "\"serialNumber\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%s\"", heartbeat_reportdata.heart_data.serial_number.value);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"leave\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
	pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.body_detector.status);
	pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"leaveTime\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
	pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.body_detector.leave_time);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dhStatus\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.fire_detector.status);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"tempList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
	for(i = 0; i < temp_count; i ++){
		pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d.%d\\\"}", heartbeat_reportdata.heart_data.temperature.sensors[i].device_id ,
		heartbeat_reportdata.heart_data.temperature.sensors[i].param / 10, heartbeat_reportdata.heart_data.temperature.sensors[i].param % 10);
		if(i != temp_count - 1){
			pos += sprintf(&buf[pos], ",");
		}
	}
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"gasList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
	for(i = 0; i < gas_count; i ++){
		pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d.%d\\\"}", heartbeat_reportdata.heart_data.gas_detector.sensors[i].device_id, 
		heartbeat_reportdata.heart_data.gas_detector.sensors[i].param / 100 , heartbeat_reportdata.heart_data.gas_detector.sensors[i].param % 100);
		if(i != gas_count - 1){
			pos += sprintf(&buf[pos], ",");
		}
	}
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"smokeList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
	for(i = 0; i < smoke_count; i ++){
		pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d.%d\\\"}", heartbeat_reportdata.heart_data.smoke_detector.sensors[i].device_id, 
		heartbeat_reportdata.heart_data.smoke_detector.sensors[i].param / 100, heartbeat_reportdata.heart_data.smoke_detector.sensors[i].param % 100);
		if(i != smoke_count - 1){
			pos += sprintf(&buf[pos], ",");
		}
	}
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dcfStatus\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.power_valves.status);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dhElectricity\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.fire_sampler.code_data);
    pos += sprintf(&buf[pos], "}");
    pos += sprintf(&buf[pos], "}");
    pos += sprintf(&buf[pos], "}");
    
    return buf;
}

char * build_alarm_json_string()
{
    int pos = 0;
    
    /* 构造JSON格式的字符串 */
    pos += sprintf(&buf[pos], "{");
    pos += sprintf(&buf[pos], "\"data_type\":\"event\",");
    pos += sprintf(&buf[pos], "\"stream_id\":\"reportdata\",");
    pos += sprintf(&buf[pos], "\"data\":{");

    pos += sprintf(&buf[pos], "\"serialNumber\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", alarmdata_report.serial_number.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%s\"", alarmdata_report.serial_number.value);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"sensorId\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", alarmdata_report.sensor_id.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%s\"", alarmdata_report.sensor_id.value);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"alarmType\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", alarmdata_report.alarm_type.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%s\"", alarmdata_report.alarm_type.value);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"alaParam\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", alarmdata_report.alarm_param.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%s\"", alarmdata_report.alarm_param.value);
    pos += sprintf(&buf[pos], "}");

    pos += sprintf(&buf[pos], "}");
    pos += sprintf(&buf[pos], "}");
    
    return buf;
}

unsigned char isalarmhappened(void)
{
    return alarmdata_report.data_type;
}

unsigned char clearalarm(void)
{
    return alarmdata_report.data_type = 0;
}

#if 1
int json_parse(char * jsonStr)  //时智厨
{
	uint8_t config_buf[RECV_DATA_NUM] = {0};
//	{"data_type": "request", "task_id": "42e13e112e18d83438868b073faad87e", 
//"data": {"warn_temp": "260", "alarm_smoke": "1000", "alarm_time": "15", "sound_size": "15", 
//"warn_time": "5", "release_alarm": "0", "dh_type": "0", "dh_electricity": "500", 
//"alarm_gas": "2500", "heart_frequency": "null", "dhlr_warn_delay": "40", "alarm_delay": "30", 
//"alarm_temp": "300"}}

	char *data_typeStart;
    char *data_typeValueStart;
    char *data_typeValueEnd;
    char data_type[32] = {0};
	char *task_idStart;
    char *task_idValueStart;
    char *task_idValueEnd;
    char task_id[32] = {0};
    char *dataStart;
    char *dataValueStart;
    char *dataValueEnd;
    char *dhlr_warn_delayStart;
    char *dhlr_warn_delayValueStart;
    char *dhlr_warn_delayValueEnd;
    int dhlr_warn_delay = 0;
    char *alarm_delayStart;
    char *alarm_delayValueStart;
    char *alarm_delayValueEnd;
    int alarm_delay = 0;
    char *alarm_timeStart;
    char *alarm_timeValueStart;
    char *alarm_timeValueEnd;
    int alarm_time = 0;
    char *warn_timeStart;
    char *warn_timeValueStart;
    char *warn_timeValueEnd;
    int warn_time = 0;
	char *warn_tempStart;
    char *warn_tempValueStart;
    char *warn_tempValueEnd;
	int warn_temp = 0;
	char *alarm_tempStart;
    char *alarm_tempValueStart;
    char *alarm_tempValueEnd;
	int alarm_temp = 0;
	char *dh_electricityStart;
    char *dh_electricityValueStart;
    char *dh_electricityValueEnd;
	int dh_electricity = 0;
	char *release_alarmStart;
    char *release_alarmValueStart;
    char *release_alarmValueEnd;
	int release_alarm = 0;
	char *break_delayStart;
    char *break_delayValueStart;
    char *break_delayValueEnd;
	int break_delay = 0;
	char *sound_sizeStart;
    char *sound_sizeValueStart;
    char *sound_sizeValueEnd;
	int sound_size = 0;
	char *alarm_gasStart;
    char *alarm_gasValueStart;
    char *alarm_gasValueEnd;
	int alarm_gas = 0;
	char *alarm_smokeStart;
    char *alarm_smokeValueStart;
    char *alarm_smokeValueEnd;
	int alarm_smoke = 0;
	char *heart_frequencyStart;
    char *heart_frequencyValueStart;
    char *heart_frequencyValueEnd;
	int heart_frequency = 0;

	data_typeStart = strstr(jsonStr, "\"data_type\":");
    data_typeValueStart = strchr(data_typeStart + strlen("\"data_type\":"), '\"');
    data_typeValueEnd = strchr(data_typeValueStart + 1, '\"');

    strncpy(data_type, data_typeValueStart + 1, data_typeValueEnd - data_typeValueStart - 1);

    task_idStart = strstr(jsonStr, "\"task_id\":");
    task_idValueStart = strchr(task_idStart + strlen("\"task_id\":"), '\"');
    task_idValueEnd = strchr(task_idValueStart + 1, '\"');

    strncpy(task_id, task_idValueStart + 1, task_idValueEnd - task_idValueStart - 1);

    dataStart = strstr(jsonStr, "\"data\":");
    dataValueStart = strchr(dataStart + strlen("\"data\":"), '{');
    dataValueEnd = strrchr(dataValueStart, '}');
    *dataValueEnd = '\0';    // 将 } 改成 \0，使得 dataValueStart 成为一个以 { 开始、以 \0 结束的字符串

    dhlr_warn_delayStart = strstr(dataValueStart, "\"dhlr_warn_delay\":");
    dhlr_warn_delayValueStart = strchr(dhlr_warn_delayStart + strlen("\"dhlr_warn_delay\":"), '\"');
    dhlr_warn_delayValueEnd = strchr(dhlr_warn_delayValueStart + 1, '\"');

    sscanf(dhlr_warn_delayValueStart + 1, "%d", &dhlr_warn_delay);
	
    alarm_delayStart = strstr(dataValueStart, "\"alarm_delay\":");
    alarm_delayValueStart = strchr(alarm_delayStart + strlen("\"alarm_delay\":"), '\"');
    alarm_delayValueEnd = strchr(alarm_delayValueStart + 1, '\"');

    sscanf(alarm_delayValueStart + 1, "%d", &alarm_delay);

    // 依次解析其他变量
    alarm_timeStart = strstr(dataValueStart, "\"alarm_time\":");
    alarm_timeValueStart = strchr(alarm_timeStart + strlen("\"alarm_time\":"), '\"');
    alarm_timeValueEnd = strchr(alarm_timeValueStart + 1, '\"');
    
    sscanf(alarm_timeValueStart + 1, "%d", &alarm_time);
	
    warn_timeStart = strstr(dataValueStart, "\"warn_time\":");
    warn_timeValueStart = strchr(warn_timeStart + strlen("\"warn_time\":"), '\"');
    warn_timeValueEnd = strchr(warn_timeValueStart + 1, '\"');
    
    sscanf(warn_timeValueStart + 1, "%d", &warn_time);

	warn_tempStart = strstr(dataValueStart, "\"warn_temp\":");
	warn_tempValueStart = strchr(warn_tempStart + strlen( "\"warn_temp\":"), '\"');
	warn_tempValueEnd = strchr(warn_tempValueStart + 1, '\"');
	
	sscanf(warn_tempValueStart + 1, "%d", &warn_temp);
	
	alarm_tempStart = strstr(dataValueStart, "\"alarm_temp\":");
	alarm_tempValueStart = strchr(alarm_tempStart + strlen( "\"alarm_temp\":"), '\"');
	alarm_tempValueEnd = strchr(alarm_tempValueStart + 1, '\"');
	
	sscanf(alarm_tempValueStart + 1, "%d", &alarm_temp);

	dh_electricityStart = strstr(dataValueStart, "\"dh_electricity\":");
	dh_electricityValueStart = strchr(dh_electricityStart + strlen( "\"dh_electricity\":"), '\"');
	dh_electricityValueEnd = strchr(dh_electricityValueStart + 1, '\"');
	
	sscanf(dh_electricityValueStart + 1, "%d", &dh_electricity);

	release_alarmStart = strstr(dataValueStart, "\"release_alarm\":");
	release_alarmValueStart = strchr(release_alarmStart + strlen( "\"release_alarm\":"), '\"');
	release_alarmValueEnd = strchr(release_alarmValueStart + 1, '\"');
	
	sscanf(release_alarmValueStart + 1, "%d", &release_alarm);
	
	break_delayStart = strstr(dataValueStart, "\"break_delay\":");
	break_delayValueStart = strchr(break_delayStart + strlen( "\"break_delay\":"), '\"');
	break_delayValueEnd = strchr(break_delayValueStart + 1, '\"');
	
	sscanf(break_delayValueStart + 1, "%d", &break_delay);
	
	sound_sizeStart = strstr(dataValueStart, "\"sound_size\":");
	sound_sizeValueStart = strchr(sound_sizeStart + strlen( "\"sound_size\":"), '\"');
	sound_sizeValueEnd = strchr(sound_sizeValueStart + 1, '\"');
	
	sscanf(sound_sizeValueStart + 1, "%d", &sound_size);
	sound_size = sound_size * 256 / 100;
	
	alarm_gasStart = strstr(dataValueStart, "\"alarm_gas\":");
	alarm_gasValueStart = strchr(alarm_gasStart + strlen( "\"alarm_gas\":"), '\"');
	alarm_gasValueEnd = strchr(alarm_gasValueStart + 1, '\"');
	
	sscanf(alarm_gasValueStart + 1, "%d", &alarm_gas);

	alarm_smokeStart = strstr(dataValueStart, "\"alarm_smoke\":");
	alarm_smokeValueStart = strchr(alarm_smokeStart + strlen( "\"alarm_smoke\":"), '\"');
	alarm_smokeValueEnd = strchr(alarm_smokeValueStart + 1, '\"');
	
	sscanf(alarm_smokeValueStart + 1, "%d", &alarm_smoke);
	
	heart_frequencyStart = strstr(dataValueStart, "\"heart_frequency\":");
	heart_frequencyValueStart = strchr(heart_frequencyStart + strlen( "\"heart_frequency\":"), '\"');
	heart_frequencyValueEnd = strchr(heart_frequencyValueStart + 1, '\"');
	
	sscanf(heart_frequencyValueStart + 1, "%d", &heart_frequency);

	// 将下发的配置 写进flash
	spi_flash_buffer_read(config_buf, CONFIGURATION_ADDRESS, RECV_DATA_NUM);
	
	config_buf[WARN_DELAY] = dhlr_warn_delay / 256;
	config_buf[WARN_DELAY + 1] = dhlr_warn_delay % 256;
	
	config_buf[ALARM_DELAY] = alarm_delay / 256;
	config_buf[ALARM_DELAY + 1] = alarm_delay % 256;
	
	config_buf[ALARM_TIME] = alarm_time;
	
	config_buf[WARN_TIME] = warn_time;
	
	config_buf[WARN_TEMP] = warn_temp / 256;
	config_buf[WARN_TEMP + 1] = warn_temp % 256;
	
	config_buf[ALARM_TEMP] = alarm_temp / 256;
	config_buf[ALARM_TEMP + 1] = alarm_temp % 256;
	
	config_buf[RELEASE_ALARM] = release_alarm;
	config_buf[BREAK_DELAY] = break_delay;
	
	config_buf[HOT_WORK_THRESHOLD] = dh_electricity / 256;
	config_buf[HOT_WORK_THRESHOLD + 1] = dh_electricity % 256;
	
	config_buf[ALARM_VOLUME] = sound_size;
	
	if(heart_frequency != 0){
		config_buf[HEARTBEAT_TIME] = heart_frequency;
	}
	
	config_buf[SMOKE_TH] = alarm_smoke / 256;
	config_buf[SMOKE_TH + 1] = alarm_smoke % 256;
	
	config_buf[GAS_TH] = alarm_gas / 256;
	config_buf[GAS_TH + 1] = alarm_gas % 256;
	
	config_buf[CHECK_BYTE] = CRC8_check(config_buf, RECV_DATA_NUM - 1);
	
	spi_flash_sector_erase(CONFIGURATION_ADDRESS);
	spi_flash_sector_erase(CONFIGURATION_ADDRESS + 0x400);
	
	spi_flash_buffer_write(config_buf, CONFIGURATION_ADDRESS, RECV_DATA_NUM);
	
	//配置生效
	p_configuration_data->warn_delay = dhlr_warn_delay;
	p_configuration_data->alarm_delay = alarm_delay;
	p_configuration_data->alarm_time = alarm_time;
	p_configuration_data->warn_time = warn_time;
	p_configuration_data->warn_temp = warn_temp;
	p_configuration_data->alarm_temp = alarm_temp;
	p_configuration_data->hot_work_threshold = dh_electricity;
	p_configuration_data->release_alarm = release_alarm;
	p_configuration_data->break_delay = break_delay;
	p_configuration_data->gas_threshold = alarm_gas;
	p_configuration_data->alarm_volume = sound_size;
	p_configuration_data->smoke_threshold = alarm_smoke;
	
	if(heart_frequency != 0){
		p_configuration_data->heartbeat_time = heart_frequency;
	}
	
	// 依次输出其他变量和对应的值
    printf("datatype: %s\ttask_id = %s\r\n", data_type, task_id);
    printf("dhlr_warn_delay = %d\talarm_delay = %d\r\n", p_configuration_data->warn_delay, p_configuration_data->alarm_delay);
    printf("warn_time = %d\talarm_time = %d\r\n", p_configuration_data->warn_time, p_configuration_data->alarm_time);
	printf("warn_temp = %d\talarm_temp = %d\r\n", p_configuration_data->warn_temp, p_configuration_data->alarm_temp);
    printf("dh_electricity = %d\r\n", p_configuration_data->hot_work_threshold);
    printf("release_alarm = %d\tbreak_delay = %d\r\n", p_configuration_data->release_alarm, p_configuration_data->break_delay);
	printf("alarm_gas = %d\talarm_smoke = %d\r\n", p_configuration_data->gas_th, p_configuration_data->smoke_th);
	printf("heart_frequency = %d\tsound_size = %d\r\n", p_configuration_data->heartbeat_time, p_configuration_data->alarm_volume);
	
	audio_process_cmd(3, &sound_size, 1);
	
	if(release_alarm == 1){
		cancel_alarm();
	}
	else if(release_alarm == 2){
		pulse_gpio_set(SET);
	}
	else if(release_alarm == 3){
		pulse_gpio_set(RESET);
	}
	else if(release_alarm == 99){
		NVIC_SystemReset();
	}	
    return 0;
}
#endif
#if 0
int json_parse(char * jsonStr)  //开发协议
{
	uint8_t config_buf[RECV_DATA_NUM] = {0};
//	{"data_type": "request", "task_id": "42e13e112e18d83438868b073faad87e", "data": {"warn_temp": "260", "alarm_smoke": "1000", "alarm_time": "15", "sound_size": "15", "warn_time": "5", "release_alarm": "0", "dh_type": "0", "dh_electricity": "500", "alarm_gas": "2500", "heart_frequency": "null", "dhlr_warn_delay": "40", "alarm_delay": "30", "alarm_temp": "300"}}
    char jsonstr[] = "{\"deviceID\":\"72-0066DF313436-3331\",\"payload\":\"{\\\"dhlr_warn_delay\\\":\\\"60\\\",\\\"alarm_delay\\\":\\\"30\\\",\\\"alarm_time\\\":\\\"15\\\",\\\"warn_time\\\":\\\"5\\\",\\\"warn_temp\\\":\\\"300\\\",\\\"alarm_temp\\\":\\\"320\\\",\\\"dh_electricity\\\":\\\"500\\\",\\\"release_alarm\\\":\\\"0\\\",\\\"break_delay\\\":\\\"15\\\",\\\"sound_size\\\":\\\"5\\\",\\\"alarm_gas\\\":\\\"25\\\",\\\"alarm_smoke\\\":\\\"10\\\",\\\"heart_frequency\\\":\\\"10\\\"}\",\"topic\":\"SETINGS\"}";
    char *deviceIDStart;
    char *deviceIDValueStart;
    char *deviceIDValueEnd;
    char deviceID[32] = {0};
    char *payloadStart;
    char *payloadValueStart;
    char *payloadValueEnd;
    char *dhlr_warn_delayStart;
    char *dhlr_warn_delayValueStart;
    char *dhlr_warn_delayValueEnd;
    int dhlr_warn_delay = 0;
    char *alarm_delayStart;
    char *alarm_delayValueStart;
    char *alarm_delayValueEnd;
    int alarm_delay = 0;
    char *alarm_timeStart;
    char *alarm_timeValueStart;
    char *alarm_timeValueEnd;
    int alarm_time = 0;
    char *warn_timeStart;
    char *warn_timeValueStart;
    char *warn_timeValueEnd;
    int warn_time = 0;
	char *warn_tempStart;
    char *warn_tempValueStart;
    char *warn_tempValueEnd;
	int warn_temp = 0;
	char *alarm_tempStart;
    char *alarm_tempValueStart;
    char *alarm_tempValueEnd;
	int alarm_temp = 0;
	char *dh_electricityStart;
    char *dh_electricityValueStart;
    char *dh_electricityValueEnd;
	int dh_electricity = 0;
	char *release_alarmStart;
    char *release_alarmValueStart;
    char *release_alarmValueEnd;
	int release_alarm = 0;
	char *break_delayStart;
    char *break_delayValueStart;
    char *break_delayValueEnd;
	int break_delay = 0;
	char *sound_sizeStart;
    char *sound_sizeValueStart;
    char *sound_sizeValueEnd;
	int sound_size = 0;
	char *alarm_gasStart;
    char *alarm_gasValueStart;
    char *alarm_gasValueEnd;
	int alarm_gas = 0;
	char *alarm_smokeStart;
    char *alarm_smokeValueStart;
    char *alarm_smokeValueEnd;
	int alarm_smoke = 0;
	char *heart_frequencyStart;
    char *heart_frequencyValueStart;
    char *heart_frequencyValueEnd;
	int heart_frequency = 0;
	
	
    char *topicStart;
    char *topicValueStart;
    char *topicValueEnd;
    char topic[32] = {0};

    deviceIDStart = strstr(jsonstr, "\"deviceID\":");
    deviceIDValueStart = strchr(deviceIDStart + strlen("\"deviceID\":"), '\"');
    deviceIDValueEnd = strchr(deviceIDValueStart + 1, '\"');

    strncpy(deviceID, deviceIDValueStart + 1, deviceIDValueEnd - deviceIDValueStart - 1);

    payloadStart = strstr(jsonstr, "\"payload\":");
    payloadValueStart = strchr(payloadStart + strlen("\"payload\":"), '{');
    payloadValueEnd = strrchr(payloadValueStart, '}');
    *payloadValueEnd = '\0';    // 将 } 改成 \0，使得 payloadValueStart 成为一个以 { 开始、以 \0 结束的字符串

    dhlr_warn_delayStart = strstr(payloadValueStart, "\"dhlr_warn_delay\\\":");
    dhlr_warn_delayValueStart = strchr(dhlr_warn_delayStart + strlen("\"dhlr_warn_delay\\\":"), '\"');
    dhlr_warn_delayValueEnd = strchr(dhlr_warn_delayValueStart + 1, '\"');

    sscanf(dhlr_warn_delayValueStart + 1, "%d", &dhlr_warn_delay);
	
    alarm_delayStart = strstr(payloadValueStart, "\"alarm_delay\\\":");
    alarm_delayValueStart = strchr(alarm_delayStart + strlen("\"alarm_delay\\\":"), '\"');
    alarm_delayValueEnd = strchr(alarm_delayValueStart + 1, '\"');

    sscanf(alarm_delayValueStart + 1, "%d", &alarm_delay);

    // 依次解析其他变量
    alarm_timeStart = strstr(payloadValueStart, "\"alarm_time\\\":");
    alarm_timeValueStart = strchr(alarm_timeStart + strlen("\"alarm_time\\\":"), '\"');
    alarm_timeValueEnd = strchr(alarm_timeValueStart + 1, '\"');
    
    sscanf(alarm_timeValueStart + 1, "%d", &alarm_time);
	
    warn_timeStart = strstr(payloadValueStart, "\"warn_time\\\":");
    warn_timeValueStart = strchr(warn_timeStart + strlen("\"warn_time\\\":"), '\"');
    warn_timeValueEnd = strchr(warn_timeValueStart + 1, '\"');
    
    sscanf(warn_timeValueStart + 1, "%d", &warn_time);

	warn_tempStart = strstr(payloadValueStart, "\"warn_temp\\\":");
	warn_tempValueStart = strchr(warn_tempStart + strlen( "\"warn_temp\\\":"), '\"');
	warn_tempValueEnd = strchr(warn_tempValueStart + 1, '\"');
	
	sscanf(warn_tempValueStart + 1, "%d", &warn_temp);
	
	alarm_tempStart = strstr(payloadValueStart, "\"alarm_temp\\\":");
	alarm_tempValueStart = strchr(alarm_tempStart + strlen( "\"alarm_temp\\\":"), '\"');
	alarm_tempValueEnd = strchr(alarm_tempValueStart + 1, '\"');
	
	sscanf(alarm_tempValueStart + 1, "%d", &alarm_temp);

	dh_electricityStart = strstr(payloadValueStart, "\"dh_electricity\\\":");
	dh_electricityValueStart = strchr(dh_electricityStart + strlen( "\"dh_electricity\\\":"), '\"');
	dh_electricityValueEnd = strchr(dh_electricityValueStart + 1, '\"');
	
	sscanf(dh_electricityValueStart + 1, "%d", &dh_electricity);

	release_alarmStart = strstr(payloadValueStart, "\"release_alarm\\\":");
	release_alarmValueStart = strchr(release_alarmStart + strlen( "\"release_alarm\\\":"), '\"');
	release_alarmValueEnd = strchr(release_alarmValueStart + 1, '\"');
	
	sscanf(release_alarmValueStart + 1, "%d", &release_alarm);
	
	break_delayStart = strstr(payloadValueStart, "\"break_delay\\\":");
	break_delayValueStart = strchr(break_delayStart + strlen( "\"break_delay\\\":"), '\"');
	break_delayValueEnd = strchr(break_delayValueStart + 1, '\"');
	
	sscanf(break_delayValueStart + 1, "%d", &break_delay);
	
	sound_sizeStart = strstr(payloadValueStart, "\"sound_size\\\":");
	sound_sizeValueStart = strchr(sound_sizeStart + strlen( "\"sound_size\\\":"), '\"');
	sound_sizeValueEnd = strchr(sound_sizeValueStart + 1, '\"');
	
	sscanf(sound_sizeValueStart + 1, "%d", &sound_size);
	sound_size *= 25;
	
	alarm_gasStart = strstr(payloadValueStart, "\"alarm_gas\\\":");
	alarm_gasValueStart = strchr(alarm_gasStart + strlen( "\"alarm_gas\\\":"), '\"');
	alarm_gasValueEnd = strchr(alarm_gasValueStart + 1, '\"');
	
	sscanf(alarm_gasValueStart + 1, "%d", &alarm_gas);

	alarm_smokeStart = strstr(payloadValueStart, "\"alarm_smoke\\\":");
	alarm_smokeValueStart = strchr(alarm_smokeStart + strlen( "\"alarm_smoke\\\":"), '\"');
	alarm_smokeValueEnd = strchr(alarm_smokeValueStart + 1, '\"');
	
	sscanf(alarm_smokeValueStart + 1, "%d", &alarm_smoke);
	
	heart_frequencyStart = strstr(payloadValueStart, "\"heart_frequency\\\":");
	heart_frequencyValueStart = strchr(heart_frequencyStart + strlen( "\"heart_frequency\\\":"), '\"');
	heart_frequencyValueEnd = strchr(heart_frequencyValueStart + 1, '\"');
	
	sscanf(heart_frequencyValueStart + 1, "%d", &heart_frequency);
	
    topicStart = strstr(jsonstr, "\"topic\":");
    topicValueStart = strchr(topicStart + strlen("\"topic\":"), '\"');
    topicValueEnd = strchr(topicValueStart + 1, '\"');
    strncpy(topic, topicValueStart + 1, topicValueEnd - topicValueStart - 1);

	// 将下发的配置 写进flash
	spi_flash_buffer_read(config_buf, CONFIGURATION_ADDRESS, RECV_DATA_NUM);
	
	config_buf[WARN_DELAY] = dhlr_warn_delay / 256;
	config_buf[WARN_DELAY + 1] = dhlr_warn_delay % 256;
	
	config_buf[ALARM_DELAY] = alarm_delay / 256;
	config_buf[ALARM_DELAY + 1] = alarm_delay % 256;
	
	config_buf[ALARM_TIME] = alarm_time;
	
	config_buf[WARN_TIME] = warn_time;
	
	config_buf[WARN_TEMP] = warn_temp / 256;
	config_buf[WARN_TEMP + 1] = warn_temp % 256;
	
	config_buf[ALARM_TEMP] = alarm_temp / 256;
	config_buf[ALARM_TEMP + 1] = alarm_temp % 256;
	
	config_buf[RELEASE_ALARM] = release_alarm;
	config_buf[BREAK_DELAY] = break_delay;
	
	config_buf[HOT_WORK_THRESHOLD] = dh_electricity / 256;
	config_buf[HOT_WORK_THRESHOLD + 1] = dh_electricity % 256;
	
	config_buf[ALARM_VOLUME] = sound_size;
	
	config_buf[HEARTBEAT_TIME] = heart_frequency;
	
	config_buf[SMOKE_TH] = alarm_smoke / 256;
	config_buf[SMOKE_TH + 1] = alarm_smoke % 256;
	
	config_buf[GAS_TH] = alarm_gas / 256;
	config_buf[GAS_TH + 1] = alarm_gas % 256;
	
	config_buf[CHECK_BYTE] = CRC8_check(config_buf, RECV_DATA_NUM - 1);
	
	spi_flash_sector_erase(CONFIGURATION_ADDRESS);
	spi_flash_sector_erase(CONFIGURATION_ADDRESS + 0x400);
	
	spi_flash_buffer_write(config_buf, CONFIGURATION_ADDRESS, RECV_DATA_NUM);
	
	//配置生效
	p_configuration_data->warn_delay = dhlr_warn_delay;
	p_configuration_data->alarm_delay = alarm_delay;
	p_configuration_data->alarm_time = alarm_time;
	p_configuration_data->warn_time = warn_time;
	p_configuration_data->warn_temp = warn_temp;
	p_configuration_data->alarm_temp = alarm_temp;
	p_configuration_data->hot_work_threshold = dh_electricity;
	p_configuration_data->release_alarm = release_alarm;
	p_configuration_data->break_delay = break_delay;
	p_configuration_data->gas_threshold = alarm_gas;
	p_configuration_data->alarm_volume = sound_size;
	p_configuration_data->smoke_threshold = alarm_smoke;
	p_configuration_data->heartbeat_time = heart_frequency;
	
	// 依次输出其他变量和对应的值
    printf("deviceID = %s\r\n", deviceID);
    printf("dhlr_warn_delay = %d\talarm_delay = %d\r\n", p_configuration_data->warn_delay, p_configuration_data->alarm_delay);
    printf("warn_time = %d\talarm_time = %d\r\n", p_configuration_data->warn_time, p_configuration_data->alarm_time);
	printf("warn_temp = %d\talarm_temp = %d\r\n", p_configuration_data->warn_temp, p_configuration_data->alarm_temp);
    printf("dh_electricity = %d\r\n", p_configuration_data->hot_work_threshold);
    printf("release_alarm = %d\tbreak_delay = %d\r\n", p_configuration_data->release_alarm, p_configuration_data->break_delay);
	printf("alarm_gas = %d\talarm_smoke = %d\r\n", p_configuration_data->gas_th, p_configuration_data->smoke_th);
	printf("heart_frequency = %d\tsound_size = %d\r\n", p_configuration_data->heartbeat_time, p_configuration_data->alarm_volume);
	
	audio_process_cmd(3, &sound_size, 1);
	
	if(release_alarm == 1){
		cancel_alarm();
	}
	else if(release_alarm == 2){
		pulse_gpio_set(SET);
	}
	else if(release_alarm == 3){
		pulse_gpio_set(RESET);
	}
	else if(release_alarm == 99){
		NVIC_SystemReset();
	}	
    return 0;
}
#endif

/**
 * @function [deal sensor related commands]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:03:08+0800
 * @version  [1.0.0]
 * @param    cmd_id                   [command id]
 * @param    param                    [param]
 * @param    paramLen                 [param length]
 * @return                            [deal result]
 */
int sensor_process_cmd(int cmd_id, char * param, int paramLen)
{
    /* TODO: handle sensor commands using net or usart */
    return 0;
}

/**
 * @function [init sensor device]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-15T11:09:12+0800
 * @version  [1.0.0]
 * @return                            [deal result]
 */
int sensor_init(void)
{
    /* TODO: Communicate with the sensor board using RS485 protocols */
    return 0;
}
