#include "protocol_rs485.h"
#include "protocol_usart.h"
#include "protocol_mqtt.h"
#include "app_format.h"
#include <stdio.h>
#include <string.h>

HeartbeatDataReport heartbeat_reportdata;
AlarmDataReport alarmdata_report;
AlarmDataReport warndata_report;

static char buf[MAX_JSON_STRING_LEN];

char * build_json_string()
{
    int pos = 0;
    
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
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.body_detector.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.body_detector.status);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"leaveTime\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.body_detector.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.body_detector.leave_time_stamp);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dhStatus\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.fire_detector.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.fire_detector.status);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"tempList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.temperature.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"},", heartbeat_reportdata.heart_data.temperature.sensors[0].device_id , heartbeat_reportdata.heart_data.temperature.sensors[0].param);
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"}", heartbeat_reportdata.heart_data.temperature.sensors[1].device_id, heartbeat_reportdata.heart_data.temperature.sensors[1].param);
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"gasList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.gas_detector.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"},", heartbeat_reportdata.heart_data.gas_detector.sensors[0].device_id, heartbeat_reportdata.heart_data.gas_detector.sensors[0].param);
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"}", heartbeat_reportdata.heart_data.gas_detector.sensors[1].device_id, heartbeat_reportdata.heart_data.gas_detector.sensors[1].param);
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"smokeList\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.smoke_detector.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"[");
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"},", heartbeat_reportdata.heart_data.smoke_detector.sensors[0].device_id, heartbeat_reportdata.heart_data.smoke_detector.sensors[0].param);
    pos += sprintf(&buf[pos], "{\\\"deviceId\\\":\\\"%s\\\",\\\"param\\\":\\\"%d\\\"}", heartbeat_reportdata.heart_data.smoke_detector.sensors[1].device_id, heartbeat_reportdata.heart_data.smoke_detector.sensors[1].param);
    pos += sprintf(&buf[pos], "]\"");
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dcfStatus\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.power_valves.time_stamp);
    pos += sprintf(&buf[pos], "\"value\":\"%d\"", heartbeat_reportdata.heart_data.power_valves.status);
    pos += sprintf(&buf[pos], "},");

    pos += sprintf(&buf[pos], "\"dhElectricity\":{");
    pos += sprintf(&buf[pos], "\"time\":%d,", heartbeat_reportdata.heart_data.fire_sampler.time_stamp);
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

unsigned char iswarnhappened(void)
{
	return warndata_report.data_type;
}

unsigned char clearwarn(void)
{
    return warndata_report.data_type = 0;
}

unsigned char isalarmhappened(void)
{
    return alarmdata_report.data_type;
}

unsigned char clearalarm(void)
{
    return alarmdata_report.data_type = 0;
}

int json_parse()
{
    char jsonStr[] = "{\"deviceID\":\"72-0066DF313436-3331\",\"payload\":\"{\\\"dhlr_warn_delay\\\":\\\"60\\\",\\\"alarm_delay\\\":\\\"30\\\",\\\"alarm_time\\\":\\\"15\\\",\\\"warn_time\\\":\\\"5\\\",\\\"warn_temp\\\":\\\"300\\\",\\\"alarm_temp\\\":\\\"320\\\",\\\"dh_electricity\\\":\\\"500\\\",\\\"release_alarm\\\":\\\"0\\\",\\\"break_delay\\\":\\\"15\\\",\\\"sound_size\\\":\\\"5\\\",\\\"alarm_gas\\\":\\\"25\\\",\\\"alarm_smoke\\\":\\\"10\\\",\\\"heart_frequency\\\":\\\"10\\\"}\",\"topic\":\"SETINGS\"}";
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
    char *topicStart;
    char *topicValueStart;
    char *topicValueEnd;
    char topic[32] = {0};

    deviceIDStart = strstr(jsonStr, "\"deviceID\":");
    deviceIDValueStart = strchr(deviceIDStart + strlen("\"deviceID\":"), '\"');
    deviceIDValueEnd = strchr(deviceIDValueStart + 1, '\"');

    strncpy(deviceID, deviceIDValueStart + 1, deviceIDValueEnd - deviceIDValueStart - 1);

    payloadStart = strstr(jsonStr, "\"payload\":");
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
    alarm_timeStart = strstr(payloadValueStart, "\"alarm_time\\\":");;
    alarm_timeValueStart = strchr(alarm_timeStart + strlen("\"alarm_time\\\":"), '\"');
    alarm_timeValueEnd = strchr(alarm_timeValueStart + 1, '\"');
    
    sscanf(alarm_timeValueStart + 1, "%d", &alarm_time);
    
    warn_timeStart = strstr(payloadValueStart, "\"warn_time\\\":");;
    warn_timeValueStart = strchr(warn_timeStart + strlen("\"warn_time\\\":"), '\"');
    warn_timeValueEnd = strchr(warn_timeValueStart + 1, '\"');
    
    sscanf(warn_timeValueStart + 1, "%d", &warn_time);

    topicStart = strstr(jsonStr, "\"topic\":");
    topicValueStart = strchr(topicStart + strlen("\"topic\":"), '\"');
    topicValueEnd = strchr(topicValueStart + 1, '\"');
    strncpy(topic, topicValueStart + 1, topicValueEnd - topicValueStart - 1);

//    printf("deviceID = %s\n", deviceID);
//    printf("dhlr_warn_delay = %d\n", dhlr_warn_delay);
//    printf("alarm_delay = %d\n", alarm_delay);
//    printf("alarm_time = %d\n", alarm_time);
//    printf("warn_time = %d\n", warn_time);

    // 依次输出其他变量和对应的值

    return 0;
}

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
