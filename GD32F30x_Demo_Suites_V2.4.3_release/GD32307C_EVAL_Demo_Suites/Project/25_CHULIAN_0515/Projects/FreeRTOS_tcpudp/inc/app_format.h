
#define MAX_JSON_STRING_LEN    1024
#define MAX_SERIAL_NUMBER_SIZE 32
#define MAX_SENSOR_NAME_SIZE   10
#define MAX_SENSOR_NUM         8

#define DATA_TYPE_EVENT        0
#define STREAM_ID_HEART_DATA   0
#define STREAM_ID_REPORT_DATA  1

/* 传感器序列号 */
typedef struct {
    int time_stamp;
    char size;
    char value[MAX_SERIAL_NUMBER_SIZE];
} SerialNumber;

typedef struct {
    int leave_time_stamp;
    char device_id[MAX_SENSOR_NAME_SIZE];
    unsigned int status;
} BodyDetectorData;

/* 人体感应传感器数据 */
typedef struct {
    int time_stamp;
    int leave_time;
	unsigned int status;
    BodyDetectorData sensors[4];
} BodyDetectorDataGroup;

/* 动火传感器数据 */
typedef struct {
    int time_stamp;
    unsigned char status;
} FireDetectorData;

/* 其他传感器数据（温度/燃气/烟感）*/
typedef struct {
    unsigned char size;
    unsigned int device_id;
    unsigned int param;
} OtherSensorData;

/* 其他传感器组数据组（温度/燃气/烟感）*/
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
//    BodyDetectorData body_detector;  //pir
	BodyDetectorDataGroup body_detector;
    FireDetectorData fire_detector; // 动火
    OtherSensorsDataGroup temperature; //温度
    OtherSensorsDataGroup gas_detector; //燃气
    OtherSensorsDataGroup smoke_detector; //烟雾
    PowerValveData power_valves;  
    FireSamplerData fire_sampler; //adc 电流
} HeartData;

/* 上报主题数据 */
typedef struct {
    unsigned char data_type;
    unsigned char stream_id;
    HeartData heart_data;
} HeartbeatDataReport;

/* 探测器编号 */
typedef struct {
    int time_stamp;
    char size;
    char value[MAX_SERIAL_NUMBER_SIZE];
} sensorId;

/* 报警类型 */
typedef struct {
    int time_stamp;
    char size;
    char value[MAX_SERIAL_NUMBER_SIZE];
} alarmType;

/* 报警参数 */
typedef struct {
    int time_stamp;
    char size;
    char value[MAX_SERIAL_NUMBER_SIZE];
} alaParam;

/* 上报报警主题 */
typedef struct {
    unsigned char data_type;
    unsigned char stream_id;
    SerialNumber serial_number;
    sensorId sensor_id;
    alarmType alarm_type;
    alaParam alarm_param;
} AlarmDataReport;

extern HeartbeatDataReport heartbeat_reportdata;
extern AlarmDataReport alarmdata_report;
char * build_json_string(void);
char * build_alarm_json_string(void);
unsigned char isalarmhappened(void);
unsigned char clearalarm(void);
int json_parse(char * jsonStr);
//int json_parse();
