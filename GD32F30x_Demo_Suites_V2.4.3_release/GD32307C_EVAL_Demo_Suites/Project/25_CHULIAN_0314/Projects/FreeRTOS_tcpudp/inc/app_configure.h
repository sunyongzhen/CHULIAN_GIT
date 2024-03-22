#include "gd32f30x.h"

/* define data index */
#define DATA_HEAD_H 			0  //data head 
#define DATA_HEAD_L 			1
#define DATA_CHECK_NUM1 			2  //data length
#define DATA_CHECK_NUM2 			3  //data length
/* ip */
#define LOCAL_IP 				4   //4 Bytes
#define GATEWAY_IP 				LOCAL_IP + 4   //to change length, need to change next line after "+"
#define DNS_IP 					GATEWAY_IP + 4  
#define SERVER_IP 				DNS_IP + 4  
/* port */
#define UDP_AIM_PORT 			SERVER_IP + 4 //2 Bytes
#define UDP_LOCAL_PORT 			UDP_AIM_PORT + 2
#define TCP_AIM_PORT 			UDP_LOCAL_PORT + 2
#define TCP_LOCAL_PORT 			TCP_AIM_PORT + 2
/* hotwork without person */
#define WARN_DELAY 				TCP_LOCAL_PORT + 2
#define ALARM_DELAY 			WARN_DELAY + 2
#define HOT_WORK_THRESHOLD 		ALARM_DELAY + 2
#define GAS_THRESHOLD 			HOT_WORK_THRESHOLD + 2
#define SMOKE_THRESHOLD 		GAS_THRESHOLD + 2
#define TEMP_CALIBRATE 			SMOKE_THRESHOLD + 2
/* hot work or no person signal mode, independent : 0, link : 1 */
#define SIGNAL_MODE 			TEMP_CALIBRATE + 2
/* hot work full motion mode, close : 0x00 open : 0x01 */
#define HOTWORK_FULL_MOTION 	SIGNAL_MODE + 1
/* no person full motion mode, close : 0x00 open : 0x01 */
#define NOPERSON_FULL_MOTION 	HOTWORK_FULL_MOTION + 1
/* mode enable , 8 channal in 1 Byte */
#define MODE_ENABLE 			NOPERSON_FULL_MOTION + 1
/* audio alert*/
#define WARN_TIME 				MODE_ENABLE + 1
#define ALARM_TIME 				WARN_TIME + 1
/* temperature detection */
#define WARN_TEMP 				ALARM_TIME + 1
#define ALARM_TEMP 				WARN_TEMP + 1
#define HEARTBEAT_TIME 			ALARM_TEMP + 1
#define ALARM_VOLUME 			HEARTBEAT_TIME + 1

#define HOST_ID 				ALARM_VOLUME + 1	//4 Bytes
#define HOST_SERIAL 			HOST_ID + 4 	//32 Bytes
#define THEME0 					HOST_SERIAL + 32 	//16 Bytes
#define THEME1 					THEME0 + 16 	//16 Bytes
#define SERVER_DOMIN 			THEME1 +16 	//64 Bytes
#define MQTT_ACCOUNT 			SERVER_DOMIN + 64 	//32 Bytes
#define MQTT_PASSWORD 			MQTT_ACCOUNT + 32 	//32 Bytes
#define CHECK_BYTE				MQTT_PASSWORD + 96

static const uint8_t crc8Table[256] = {   //CRC_8 check table
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};
struct ipaddr{
	uint8_t ipaddr0;
	uint8_t ipaddr1;
	uint8_t ipaddr2;
	uint8_t ipaddr3;
};

struct mode_enable{
	uint8_t channel0;
	uint8_t channel1;
	uint8_t channel2;
	uint8_t channel3;
};

/* analyzed data store in this struct */
struct eeprom_data{
	/* ip */
	struct ipaddr local_ip;
	struct ipaddr gateway;
	struct ipaddr dns;
	struct ipaddr server_ip;
	/* enable */
	struct mode_enable hotwork_enable;
	struct mode_enable noperson_enable;
	
	/* port */
	uint16_t udp_aim_port;
	uint16_t udp_local_port;
	uint16_t tcp_aim_port;
	uint16_t tcp_local_port;
	/* hotwork without person */
	uint16_t warn_delay;
	uint16_t alarm_delay;
	uint16_t hot_work_threshold;
	uint16_t gas_threshold;
	uint16_t smoke_threshold;
	uint16_t temp_calibrate;
	
	uint8_t hotwork_noperson_mode;
	uint8_t hotwork_fm;
	uint8_t noperson_fm;
	/* audio alert*/
	uint8_t warn_time;
	uint8_t alarm_time;
	uint8_t warn_temp;
	uint8_t alarm_temp;
	uint8_t heartbeat_time;
	uint8_t alarm_volume;
	
	uint8_t host_ID[8];
	uint8_t host_serial[32];
	uint8_t theme0[16];
	uint8_t theme1[16];
	uint8_t server_domin[64];
	uint8_t MQTT_account[32];
	uint8_t MQTT_password[32];
};

extern struct eeprom_data configuration_data;
extern struct eeprom_data *p_configuration_data;

/* read data , send to server */
int read_data_and_set_ack(uint8_t *buf, uint16_t buf_size);
/* read data from eeprom, check and analyze */
int read_data_and_analyze(uint16_t buf_size);
/* check data , write into eeprom */
int write_data_to_flash(uint8_t *buf, uint16_t buf_size);
/* check data head, data length, CRC_8 check */
int data_check(uint8_t *buf, uint16_t buf_size);
/* analyze data, return struct eeprom_data* */
struct eeprom_data *data_analyze(uint8_t *buf, uint16_t buf_size);
/* print data */
void show_data(struct eeprom_data *data);
/* separate 1 Byte to 8 bool */
void one_byte_to_eight_bool(uint8_t c, bool b[8]);
/* crc8 check£¬x8 + x2 + x1 + 1 0000 0111 0x07 */
uint8_t CRC8_check(uint8_t *buf, uint16_t buf_size);

int handle_configuration_read(uint8_t *data);
int handle_configuration_write(uint8_t *data);
int handle_configuration_downloaded_wav(uint8_t *data);
