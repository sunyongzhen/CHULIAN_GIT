#include "protocol_rs485.h"
#include "protocol_usart.h"
#include "protocol_mqtt.h"

#define MAX_JSON_STRING_LEN    1024
#define MAX_SERIAL_NUMBER_SIZE 32
#define MAX_SENSOR_NAME_SIZE   10
#define MAX_SENSOR_NUM         3

#define DATA_TYPE_EVENT        0
#define STREAM_ID_HEART_DATA   0
#define STREAM_ID_REPORT_DATA  1

/* 传感器序列号 */
typedef struct {
    int time_stamp;
    char size;
    char value[MAX_SERIAL_NUMBER_SIZE];
} SerialNumber;

/* 人体感应传感器数据 */
typedef struct {
    int time_stamp;
    unsigned char status;
    int leave_time_stamp;
} BodyDetectorData;

/* 其他传感器数据（温度/燃气/烟感）*/
typedef struct {
    unsigned char size;
    char device_id[MAX_SENSOR_NAME_SIZE];
    unsigned char param;
} OtherSensorData;

/* 其他传感器组数据（温度/燃气/烟感）*/
typedef struct {
    int time_stamp;
    unsigned char sensor_count;
    OtherSensorData sensors[MAX_SENSOR_NUM];
} OtherSensorsDataGroup;

/* 电池阀状态数据 */
typedef struct {
    int time_stamp;
    unsigned char status;
} PowerValveData;

/* 动火采集器码值数据 */
typedef struct {
    int time_stamp;
    int code_data;
} FireSamplerData;

/* 心率数据 */ 
typedef struct {
    SerialNumber serial_number;
    BodyDetectorData body_detector;
    OtherSensorData fire_detector;
    OtherSensorsDataGroup temperature;
    OtherSensorsDataGroup gas_detector;
    OtherSensorsDataGroup smoke_detector;
    PowerValveData power_valves;
    FireSamplerData fire_sampler;
} HeartData;

/* 上报主题数据 */
typedef struct {
    unsigned char data_type;
    unsigned char stream_id;
    HeartData heart_data;
} HeartbeatDataReport;

HeartbeatDataReport heartbeat_reportdata;

static char buf[MAX_JSON_STRING_LEN];

void build_json_string()
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
