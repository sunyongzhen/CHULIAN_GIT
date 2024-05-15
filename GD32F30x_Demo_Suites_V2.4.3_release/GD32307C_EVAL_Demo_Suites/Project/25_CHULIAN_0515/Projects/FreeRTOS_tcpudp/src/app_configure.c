#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "module_gd25qxx.h"
#include "app_configure.h"
#include "gd32f30x_rtc.h"
#include <math.h>

uint32_t TMPE_ADDRESS = 0;

struct config_data configuration_data;
struct config_data *p_configuration_data;

uint32_t audio_add = 0;


int read_data_and_set_ack(uint8_t *buf, uint16_t buf_size)
{
    memset(buf, 0, buf_size);
    spi_flash_buffer_read(buf, CONFIGURATION_ADDRESS, buf_size);
    /* change data head */
    buf[DATA_HEAD_H] = 0xFF;
    buf[DATA_HEAD_L] = 0xCC;
    return 0;
}

int read_data_and_analyze(uint16_t buf_size) //return 0 success  else fail
{
    int ret = -1;
//	int	i;
    uint8_t buf[RECV_DATA_NUM];
    memset(buf, 0, buf_size);
    /* read data from spi flash */
    spi_flash_buffer_read(buf, CONFIGURATION_ADDRESS, buf_size);

    /* check data */
    ret = data_check(buf, buf_size);
    if (ret < 0) {
//        printf("read_data_and_analyze data_check error\r\n");
        return ret;
    }
//    printf("read data num :%d\r\n", buf_size);
//    for (i = 0; i < buf_size; i++) {
//        printf("0x%02X ", buf[i]);
//        if (15 == i % 16) {
//            printf("\r\n");
//        }
//    }
//    printf("\r\n");
    /* data analyze */
    data_analyze(buf, buf_size);
    return ret;
}

int write_data_to_flash(uint8_t *buf, uint16_t buf_size) //return 0 success  else fail
{
    int ret = 0;
    /* check data */
    ret = data_check(buf, buf_size);
    if (ret < 0) {
//        printf("write_data_to_flash data_check error\r\n");
        memset(buf, 0, buf_size);
        buf[0] = 0xAA;
        buf[1] = 0xFF;
        buf[2] = 0x01;
        return ret;
    }
    /* erases the specified flash sector */
    spi_flash_sector_erase(CONFIGURATION_ADDRESS);
	spi_flash_sector_erase(CONFIGURATION_ADDRESS + 0x400);
    /* write data to the flash */
    spi_flash_buffer_write(buf, CONFIGURATION_ADDRESS, CONFIGURATION_DATASIZE);
    /* set ack data */
    memset(buf, 0, buf_size);
    buf[0] = 0xAA;
    buf[1] = 0xFF;
    buf[2] = 0x00;
    return ret;
}


int write_sn_to_flash(uint8_t *buf, uint16_t buf_size)
{
	uint8_t *data;
	int i;
	 /* erases the specified flash sector */
    spi_flash_sector_erase(AUDIO7_ADDRESS);
	
	for(i = 0; i < buf_size; i++)
	{
		//printf("write:%x\n", buf[i]);
	}
    /* write data to the flash */
    spi_flash_buffer_write(buf, AUDIO7_ADDRESS, buf_size);
	
	spi_flash_buffer_read(data, AUDIO7_ADDRESS, buf_size);
	
	for(i = 0; i < buf_size; i++)
	{
		//printf("read:%x\n", data[i]);
	}
	
	if(memcmp(buf, data, buf_size) == 0)
	{
		//printf("write sn success!");
		return 1;
	}
	else{
		//printf("write sn fail!");
		return 0;
	}
	
	
//	for(i = 0; i < 6; i++)
//	{
//		printf("%x\n", data[i]);
//	}
//	return 1;
}


void read_sn_from_flash(uint8_t* buf, uint8_t buf_size)
{
	spi_flash_buffer_read(buf, AUDIO7_ADDRESS, buf_size);
}

int data_check(uint8_t *buf, uint16_t buf_size) //return 0 success  else fail
{
    uint8_t check_byte = 0xFF;
    if ((buf[DATA_HEAD_H] == 0xFF) && (buf[DATA_HEAD_L] == 0xAA)) //data head correct
    {
        if (((buf[DATA_CHECK_NUM1] * 256) + buf[DATA_CHECK_NUM2]) == buf_size) { //length correct
            /* check data */
            check_byte = CRC8_check(buf, buf_size - 1);
//			printf("buf[CHECK_BYTE]: %d, check_byte: %d\n", buf[CHECK_BYTE], check_byte);
            if (buf[CHECK_BYTE] == check_byte) {
//                printf("data_check success\r\n");
                return 0;
            } 
//			else {
//                printf("data_check fail\r\n");
//            }
        } 
//		else {
//            printf("data_check bufsize error\r\n");
//        }
    } 
//	else {
//        printf("data_check datahead error\r\n");
//    }
    return -1;
}

struct config_data *data_analyze(uint8_t *buf, uint16_t buf_size) //return 0 success  else fail
{

    bool temp[8];
    int i;
    memset(&configuration_data, 0, sizeof(struct config_data));
    /* local ip */
    configuration_data.local_ip.ipaddr0 = buf[LOCAL_IP];
    configuration_data.local_ip.ipaddr1 = buf[LOCAL_IP + 1];
    configuration_data.local_ip.ipaddr2 = buf[LOCAL_IP + 2];
    configuration_data.local_ip.ipaddr3 = buf[LOCAL_IP + 3];
    /* gateway */
    configuration_data.gateway.ipaddr0  = buf[GATEWAY_IP];
    configuration_data.gateway.ipaddr1  = buf[GATEWAY_IP + 1];
    configuration_data.gateway.ipaddr2  = buf[GATEWAY_IP + 2];
    configuration_data.gateway.ipaddr3  = buf[GATEWAY_IP + 3];
    /* DNS */
    configuration_data.dns.ipaddr0 = buf[DNS_IP];
    configuration_data.dns.ipaddr1 = buf[DNS_IP + 1];
    configuration_data.dns.ipaddr2 = buf[DNS_IP + 2];
    configuration_data.dns.ipaddr3 = buf[DNS_IP + 3];
	/* mask */
    configuration_data.mask.ipaddr0 = buf[SUBNET_MASK];
    configuration_data.mask.ipaddr1 = buf[SUBNET_MASK + 1];
    configuration_data.mask.ipaddr2 = buf[SUBNET_MASK + 2];
    configuration_data.mask.ipaddr3 = buf[SUBNET_MASK + 3];
    /* server ip */
    configuration_data.server_ip.ipaddr0  = buf[SERVER_IP];
    configuration_data.server_ip.ipaddr1  = buf[SERVER_IP + 1];
    configuration_data.server_ip.ipaddr2  = buf[SERVER_IP + 2];
    configuration_data.server_ip.ipaddr3  = buf[SERVER_IP + 3];
    /* port */
    configuration_data.udp_aim_port       = buf[UDP_AIM_PORT]     * 256 + buf[UDP_AIM_PORT + 1];
    configuration_data.udp_local_port     = buf[UDP_LOCAL_PORT]   * 256 + buf[UDP_LOCAL_PORT + 1];
    configuration_data.tcp_aim_port       = buf[TCP_AIM_PORT]     * 256 + buf[TCP_AIM_PORT + 1];
    configuration_data.tcp_local_port     = buf[TCP_LOCAL_PORT]   * 256 + buf[TCP_LOCAL_PORT + 1];
    /* hot work no person */
    configuration_data.warn_delay         = buf[WARN_DELAY]           * 256 + buf[WARN_DELAY + 1];
    configuration_data.alarm_delay        = buf[ALARM_DELAY]          * 256 + buf[ALARM_DELAY + 1];
    configuration_data.hot_work_threshold = buf[HOT_WORK_THRESHOLD]   * 256 + buf[HOT_WORK_THRESHOLD + 1];
    configuration_data.gas_threshold      = buf[GAS_THRESHOLD] 		  * 256 + buf[GAS_THRESHOLD + 1];
    configuration_data.smoke_threshold    = buf[SMOKE_THRESHOLD]	  * 256 + buf[SMOKE_THRESHOLD + 1];
    configuration_data.temp_calibrate     = buf[TEMP_CALIBRATE]       * 256 + buf[TEMP_CALIBRATE + 1];
	configuration_data.gas_th			  = buf[GAS_TH] 		  	  * 256 + buf[GAS_TH + 1];
    configuration_data.smoke_th		      = buf[SMOKE_TH]	  		  * 256 + buf[SMOKE_TH + 1];
    /* hot work or no person signal mode, independent : 0, link : 1 */
    if (buf[SIGNAL_MODE] == 0x01) {
        configuration_data.hotwork_noperson_mode = 1;
    } else {
        configuration_data.hotwork_noperson_mode = 0;
    }
    /* hot work full motion mode, close : 0x00 open : 0x01 */
    if (buf[HOTWORK_FULL_MOTION] == 0x01) {
        configuration_data.hotwork_fm = 1;
    } else {
        configuration_data.hotwork_fm = 0;
    }
    /* no person full motion mode, close : 0x00 open : 0x01 */
    if (buf[NOPERSON_FULL_MOTION] == 0x01) {
        configuration_data.noperson_fm = 1;
    } else {
        configuration_data.noperson_fm = 0;
    }
    /* hot work enable, no person enacble, 8 channals in 1 Byte */
    one_byte_to_eight_bool(buf[MODE_ENABLE], temp);
    configuration_data.noperson_enable.channel0 = temp[0];
    configuration_data.noperson_enable.channel1 = temp[1];
    configuration_data.noperson_enable.channel2 = temp[2];
    configuration_data.noperson_enable.channel3 = temp[3];
    configuration_data.hotwork_enable.channel0  = temp[4];
    configuration_data.hotwork_enable.channel1  = temp[5];
    configuration_data.hotwork_enable.channel2  = temp[6];
    configuration_data.hotwork_enable.channel3  = temp[7];
    /* audio alert*/
    configuration_data.warn_time  = buf[WARN_TIME];
    configuration_data.alarm_time = buf[ALARM_TIME];
    /* temperature detection */
    configuration_data.warn_temp       = buf[WARN_TEMP] * 256 + buf[WARN_TEMP + 1];
    configuration_data.alarm_temp      = buf[ALARM_TEMP] * 256 + buf[ALARM_TEMP + 1];
    configuration_data.heartbeat_time  = buf[HEARTBEAT_TIME];
    configuration_data.alarm_volume    = buf[ALARM_VOLUME];
	configuration_data.release_alarm   = buf[RELEASE_ALARM];
	configuration_data.break_delay     = buf[BREAK_DELAY];
    /* host id */
//    configuration_data.host_ID[0] = buf[HOST_ID] / 16;
//    configuration_data.host_ID[1] = buf[HOST_ID] % 16;
//    configuration_data.host_ID[2] = buf[HOST_ID + 1] / 16;
//    configuration_data.host_ID[3] = buf[HOST_ID + 1] % 16;
//    configuration_data.host_ID[4] = buf[HOST_ID + 2] / 16;
//    configuration_data.host_ID[5] = buf[HOST_ID + 2] % 16;
//    configuration_data.host_ID[6] = buf[HOST_ID + 3] / 16;
//    configuration_data.host_ID[7] = buf[HOST_ID + 3] % 16;
	for (i = 0; i < 32; i++) {
        configuration_data.host_ID[i]   = buf[HOST_ID + i];
    }
    /* host serial */
    for (i = 0; i < 32; i++) {
        configuration_data.host_serial[i]   = buf[HOST_SERIAL + i];
    }
    /* subscribe theme 0 */
    for (i = 0; i < 16; i++) {
        configuration_data.theme0[i]        = buf[THEME0 + i];
    }
    /* subscribe theme 1 */
    for (i = 0; i < 16; i++) {
        configuration_data.theme1[i]        = buf[THEME1 + i];
    }
    /* server domin */
    for (i = 0; i < 64; i++) {
        configuration_data.server_domin[i]  = buf[SERVER_DOMIN + i];
    }
    /* MQTT accounut */
    for (i = 0; i < 32; i++) {
        configuration_data.MQTT_account[i]  = buf[MQTT_ACCOUNT + i];
    }
    /* MQTT password */
    for (i = 0; i < 32; i++) {
        configuration_data.MQTT_password[i] = buf[MQTT_PASSWORD + i];
    }
	configuration_data.smoke_port[0] = buf[PORT0_SMOKE] << 24 | buf[PORT0_SMOKE+1] << 16 | buf[PORT0_SMOKE+2] << 8 | buf[PORT0_SMOKE+3];
	configuration_data.smoke_port[1] = buf[PORT1_SMOKE] << 24 | buf[PORT1_SMOKE+1] << 16 | buf[PORT1_SMOKE+2] << 8 | buf[PORT1_SMOKE+3];
	configuration_data.smoke_port[2] = buf[PORT2_SMOKE] << 24 | buf[PORT2_SMOKE+1] << 16 | buf[PORT2_SMOKE+2] << 8 | buf[PORT2_SMOKE+3];
	configuration_data.smoke_port[3] = buf[PORT3_SMOKE] << 24 | buf[PORT3_SMOKE+1] << 16 | buf[PORT3_SMOKE+2] << 8 | buf[PORT3_SMOKE+3];
	configuration_data.gas_port[0] = buf[PORT0_GAS] << 24 | buf[PORT0_GAS+1] << 16 | buf[PORT0_GAS+2] << 8 | buf[PORT0_GAS+3];
	configuration_data.gas_port[1] = buf[PORT1_GAS] << 24 | buf[PORT1_GAS+1] << 16 | buf[PORT1_GAS+2] << 8 | buf[PORT1_GAS+3];
	configuration_data.gas_port[2] = buf[PORT2_GAS] << 24 | buf[PORT2_GAS+1] << 16 | buf[PORT2_GAS+2] << 8 | buf[PORT2_GAS+3];
	configuration_data.gas_port[3] = buf[PORT3_GAS] << 24 | buf[PORT3_GAS+1] << 16 | buf[PORT3_GAS+2] << 8 | buf[PORT3_GAS+3];
	configuration_data.temp_port[0] = buf[PORT0_TEMP] << 24 | buf[PORT0_TEMP+1] << 16 | buf[PORT0_TEMP+2] << 8 | buf[PORT0_TEMP+3];
	configuration_data.temp_port[1] = buf[PORT1_TEMP] << 24 | buf[PORT1_TEMP+1] << 16 | buf[PORT1_TEMP+2] << 8 | buf[PORT1_TEMP+3];
	configuration_data.temp_port[2] = buf[PORT2_TEMP] << 24 | buf[PORT2_TEMP+1] << 16 | buf[PORT2_TEMP+2] << 8 | buf[PORT2_TEMP+3];
	configuration_data.temp_port[3] = buf[PORT3_TEMP] << 24 | buf[PORT3_TEMP+1] << 16 | buf[PORT3_TEMP+2] << 8 | buf[PORT3_TEMP+3];
	configuration_data.pir_port[0] = buf[PORT0_PIR] << 24 | buf[PORT0_PIR+1] << 16 | buf[PORT0_PIR+2] << 8 | buf[PORT0_PIR+3];
	configuration_data.pir_port[1] = buf[PORT1_PIR] << 24 | buf[PORT1_PIR+1] << 16 | buf[PORT1_PIR+2] << 8 | buf[PORT1_PIR+3];
	configuration_data.pir_port[2] = buf[PORT2_PIR] << 24 | buf[PORT2_PIR+1] << 16 | buf[PORT2_PIR+2] << 8 | buf[PORT2_PIR+3];
	configuration_data.pir_port[3] = buf[PORT3_PIR] << 24 | buf[PORT3_PIR+1] << 16 | buf[PORT3_PIR+2] << 8 | buf[PORT3_PIR+3];
	
	if(buf[CHANNEL0]){
		configuration_data.channel0.dh_channel = 1;
		configuration_data.channel0.pir_id = buf[CHANNEL0 + 1];
		for (i = 0; i < 32; i++) {
			configuration_data.channel0.camera_id[i]  = buf[CHANNEL0 + 2 +i];
		}
	}
	if(buf[CHANNEL1]){
		configuration_data.channel1.dh_channel = 1;
		configuration_data.channel1.pir_id = buf[CHANNEL1 + 1];
		for (i = 0; i < 32; i++) {
			configuration_data.channel1.camera_id[i]  = buf[CHANNEL1 + 2 +i];
		}
	}
	if(buf[CHANNEL2]){
		configuration_data.channel2.dh_channel = 1;
		configuration_data.channel2.pir_id = buf[CHANNEL2 + 1];
		for (i = 0; i < 32; i++) {
			configuration_data.channel2.camera_id[i]  = buf[CHANNEL2 + 2 +i];
		}
	}
	if(buf[CHANNEL3]){
		configuration_data.channel3.dh_channel = 1;
		configuration_data.channel3.pir_id = buf[CHANNEL3 + 1];
		for (i = 0; i < 32; i++) {
			configuration_data.channel3.camera_id[i]  = buf[CHANNEL3 + 2 +i];
		}
	}
	configuration_data.camera_type = buf[CAMERA_TYPE];
	
    p_configuration_data = &configuration_data;
//    show_data(p_configuration_data);
    return p_configuration_data;
}

void show_data(struct config_data *data)
{
//    int i = 0;
    printf("localip: %d.%d.%d.%d\r\n",  data->local_ip.ipaddr0, data->local_ip.ipaddr1,
           data->local_ip.ipaddr2, data->local_ip.ipaddr3);
    printf("gateway: %d.%d.%d.%d\r\n",  data->gateway.ipaddr0, data->gateway.ipaddr1,
           data->gateway.ipaddr2, data->gateway.ipaddr3);
    printf("dns: %d.%d.%d.%d\r\n",      data->dns.ipaddr0, data->dns.ipaddr1,
           data->dns.ipaddr2, data->dns.ipaddr3);
	 printf("mask: %d.%d.%d.%d\r\n",      data->mask.ipaddr0, data->mask.ipaddr1,
           data->mask.ipaddr2, data->mask.ipaddr3);
    printf("serverip: %d.%d.%d.%d\r\n", data->server_ip.ipaddr0, data->server_ip.ipaddr1,
           data->server_ip.ipaddr2, data->server_ip.ipaddr3);

    printf("UDPaimport: %d\t UDPlocalport: %d\t TCPaimport: %d\t TCPlocalport: %d\r\n",
           data->udp_aim_port, data->udp_local_port, data->tcp_aim_port, data->tcp_local_port);

    printf("warndelay: %d\t alarmdelay: %d\r\n", data->warn_delay, data->alarm_delay);

    printf("hotworkthrd: %d\t tempcal:%d\r\n", data->hot_work_threshold, data->temp_calibrate);

    printf("smokethrd:%d\t gasthrd: %d\r\n", data->smoke_threshold, data->gas_threshold);
	printf("smoketh:%d\t gasth: %d\r\n", data->smoke_th, data->gas_th);

    printf("hotwork_noperson_mode(电流 0, 温度 1): %02x\r\n", data->hotwork_noperson_mode);
	printf("camera_type(网络 1, 接线 0): %02x\r\n", data->camera_type);
	
    printf("hotwork_full_motoion: %02x\r\n", data->hotwork_fm);

    printf("nopersion_full_motoion: %02x\r\n", data->noperson_fm);

    printf("hotwork_enable: channel3: %02x, channel2: %02x, channel1: %02x, channel0: %02x\r\n",
           data->hotwork_enable.channel3, data->hotwork_enable.channel2,
           data->hotwork_enable.channel1, data->hotwork_enable.channel0);

    printf("nopersion_enable: channel3: %02x, channel2: %02x, channel1: %02x, channel0: %02x\r\n",
           data->noperson_enable.channel3, data->noperson_enable.channel2,
           data->noperson_enable.channel1, data->noperson_enable.channel0);

    printf("warn_time: %d\t alarm_time: %d\r\n", data->warn_time, data->alarm_time);

    printf("warn_temp: %d\t alarm_temp: %d\r\n", data->warn_temp, data->alarm_temp);

    printf("heartbeat: %d\t alarm_volume: %d\r\n", data->heartbeat_time, data->alarm_volume);

	printf("release_alarm : %d\t break_delay: %d\r\n", data->release_alarm, data->break_delay);

    printf("host_id: %s\r\n", data->host_ID);
    printf("host_serial: %s\r\n", data->host_serial);
    printf("theme0: %s\r\n", data->theme0);
    printf("theme1: %s\r\n", data->theme1);
    printf("server_domin: %s\r\n", data->server_domin);
    printf("MQTT_account: %s\r\n", data->MQTT_account);
    printf("MQTT_password: %s\r\n", data->MQTT_password);

	printf("smoke  port0: %d\t port1:%d\t port3: %d\t port4: %d\r\n", data->smoke_port[0], data->smoke_port[1], data->smoke_port[2], data->smoke_port[3]);
	printf("gas  port0: %d\t port1:%d\t port3: %d\t port4: %d\r\n", data->gas_port[0], data->gas_port[1], data->gas_port[2], data->gas_port[3]);
	printf("temp  port0: %d\t port1:%d\t port3: %d\t port4: %d\r\n", data->temp_port[0], data->temp_port[1], data->temp_port[2], data->temp_port[3]);
	printf("pir  port0: %d\t port1:%d\t port3: %d\t port4: %d\r\n", data->pir_port[0], data->pir_port[1], data->pir_port[2], data->pir_port[3]);
	
	printf("cahnnel0 en:%02x\t pir_id: %02x\t camera: %s\r\n",data->channel0.dh_channel, data->channel0.pir_id, data->channel0.camera_id);
	printf("cahnnel1 en:%02x\t pir_id: %02x\t camera: %s\r\n",data->channel1.dh_channel, data->channel1.pir_id, data->channel1.camera_id);
	printf("cahnnel2 en:%02x\t pir_id: %02x\t camera: %s\r\n",data->channel2.dh_channel, data->channel2.pir_id, data->channel2.camera_id);
	printf("cahnnel3 en:%02x\t pir_id: %02x\t camera: %s\r\n",data->channel3.dh_channel, data->channel3.pir_id, data->channel3.camera_id);
}

void one_byte_to_eight_bool(uint8_t c, bool b[8])
{
    int i;
    for (i = 0; i < 8; i++) {
        if ((c & (1 << i)) != 0) {
            b[i] = TRUE;
        } else {
            b[i] = FALSE;
        }
    }
}

uint8_t CRC8_check(uint8_t *buf, uint16_t buf_size)
{
    uint8_t check_byte = 0, temp;
    char *hexEncode = "0123456789abcdef";
    int i, j;
    for (i = 0; i < buf_size * 2; i++) {
        j = i / 2;
        if (i % 2 == 0) {
            temp = buf[j] / 16;  //get high 4 bits
        } else {
            temp = buf[j] % 16;  //get low 4 bits
        }
        temp = hexEncode[temp & 0xf];  //hex to string
        check_byte = check_byte ^ temp;
        check_byte = crc8Table[check_byte];
    }
    return check_byte;
}

int handle_configuration_read(uint8_t *data)
{

    if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xBB))
    {
        read_data_and_set_ack(data, CONFIGURATION_DATASIZE);
		//read_audio(data, 400);
        return 0;
    }
    return 2;
}

int handle_configuration_write(uint8_t *data)
{
    if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xAA))
    {
        write_data_to_flash(data, CONFIGURATION_DATASIZE);
        return 1;
    }
    return 2;
}



int handle_configuration_downloaded_wav(uint8_t *data)
{
	uint16_t index_data = 0;
//	uint16_t i = 0;
	if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD0))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO1_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO1_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
//		// ______________________________________
//		spi_flash_buffer_read(data, AUDIO1_ADDRESS + (index_data * AUDIO_DATASIZE), 256);
//		for(i = 0; i< 256; i ++){
//			printf("%x ",data[i]);
//			if(15 == i %16){
//				printf("\r\n");
//			}
//		}
		
		return 1;
    }else if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD1))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO2_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO2_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
		return 2;
    }else if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD2))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO3_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO3_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
		return 3;
    }else if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD3))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO4_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO4_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
		return 4;
    }else if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD4))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO5_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO5_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
		return 5;
    }else if ((data[DATA_HEAD_H] == 0xFF) && (data[DATA_HEAD_L] == 0xD5))
    {
		index_data = data[2] * 256 + data[3];
//		printf("index: %d", index_data);
//		printf("\n");
		 /* erases the specified flash sector */
		if(index_data % 16 == 0){
			spi_flash_sector_erase(AUDIO6_ADDRESS + (index_data * AUDIO_DATASIZE));
		}
		/* write data to the flash */
		spi_flash_buffer_write(data + 4, AUDIO6_ADDRESS + (index_data * AUDIO_DATASIZE), AUDIO_DATASIZE);
		return 6;
    }
    return 7;
}





