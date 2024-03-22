#include <stdio.h>
#include <string.h>

#include "MQTT/MQTTPacket.h"
#include "MQTT/transport.h"
#include "hello_gigadevice.h"
#include "gd32f30x.h"
#include "app_mqtt.h"
#include "main.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/api.h"

#include "app_format.h"

/*
{
    "data_type":"event",
    "stream_id":"heartData",
    "data":{
        "serialNumber":{
            "time":1629010525,
            "value":"72-0066DF313436-3331"
        },
        "leave":{
            "time":1629010525,
            "value":"0"
        },
        "leaveTime":{
            "time":1629010525,
            "value":"60"
        },
        "dhStatus":{
            "time":1629010525,
            "value":"1"
        },
        "tempList":{
            "time":1629010525,
            "value":"[{\"deviceId\":\"HGFD32\",\"param\":\"160\"},{\"deviceId\":\"HGFD31\",\"param\":\"230\"}]"
        },
        "gasList":{
            "time":1629010525,
            "value":"[{\"deviceId\":\"1234\",\"param\":\"10\"},{\"deviceId\":\"5435\",\"param\":\"20\"}]"
        },
        "smokeList":{
            "time":1629010525,
            "value":"[{\"deviceId\":\"3123\",\"param\":\"30\"},{\"deviceId\":\"5435\",\"param\":\"30\"}]"
        },
        "dcfStatus":{
            "time":1629010525,
            "value":"0"
        },
        "dhElectricity":{
            "time":1629010525,
            "value":"800"
        }
    }
}
{
    "data_type":"event",
    "stream_id":"reportdata",
    "data":{
        "serialNumber":{  //主机设备序列号
            "time":1628990872,
            "value":"72-0066DF313436-3331"
        },
        "sensorId":{  //探测器编号
            "time":1628990872,
            "value":"02"
        },
        "alarmType":{  //动火离人预警:DHLR_WARN，动火离人报警:DHLR_ALARM,烟感报警:SMOKE_ALARM,燃气报警:GAS_ALARM ，油温过高报警：DHLR_OIL_TEMP_HIGH  
            "time":1628990872,
            "value":"DHLR_ALARM"
        },
        "alaParam":{  //报警参数
            "time":1628990872,
            "value":"0.15"
        }
    }
}	
*/

#define MQTT_TASK_PRIO  ( tskIDLE_PRIORITY + 5 )

TaskHandle_t TaskHandle = NULL;
static char toStop = 0;
static unsigned char buf[1024];

static void mqtt_pub0sub1_task(void *arg)
{
    uint32_t ulNotifyValue = 0;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int rc = 0;
    int mysock = 0;
    int buflen = sizeof(buf);
    int msgid = 1;
    MQTTString topicString = MQTTString_initializer;
    int req_qos = 0;
    char* payload = "mypayload";
    int payloadlen = strlen(payload);
    int len = 0;
    char *host = "192.168.31.216";
    int port = 1883;
    
    while (1)
    {
        if (xTaskNotifyWait(0UL, 0xffffffffUL, &ulNotifyValue, portMAX_DELAY) == pdTRUE)
        {
            printf("received notify value %u\r\n", ulNotifyValue);
        }

        // 通过ip和端口创建socket
        mysock = transport_open(host, port);
        if (mysock < 0)
        {
            printf("transport_open failed \r\n");
            continue;
        }

        printf("Sending to hostname %s port %d\n", host, port);

        data.clientID.cstring = "me";
        data.keepAliveInterval = 20;
        data.cleansession = 1;
        data.username.cstring = "deepano";
        data.password.cstring = "1";

        // 初始化连接信息然后通过socket发出去
        len = MQTTSerialize_connect(buf, buflen, &data);
        rc = transport_sendPacketBuffer(mysock, buf, len);


        /* wait for connack */
        if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
        {
            unsigned char sessionPresent, connack_rc;

            if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
            {
                printf("Unable to connect, return code %d\n", connack_rc);
                goto exit;
            }
        }
        else
            goto exit;


        /* subscribe */
        topicString.cstring = "substopic";
        len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);

        rc = transport_sendPacketBuffer(mysock, buf, len);
        if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK)  /* wait for suback */
        {
            unsigned short submsgid;
            int subcount;
            int granted_qos;

            rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
            if (granted_qos != 0)
            {
                printf("granted qos != 0, %d\n", granted_qos);
                goto exit;
            }
        }
        else
            goto exit;

        /* loop getting msgs on subscribed topic */
        topicString.cstring = "pubtopic";
        while (!toStop)
        {
            /* transport_getdata() has a built-in 1 second timeout,
            your mileage will vary */
            /*  */
            if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
            {
                unsigned char dup;
                int qos;
                unsigned char retained;
                unsigned short msgid;
                int payloadlen_in;
                unsigned char* payload_in;
                int rc;
                MQTTString receivedTopic;

                rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
                                             &payload_in, &payloadlen_in, buf, buflen);
                printf("message arrived %.*s\n", payloadlen_in, payload_in);
            }
//            json_parse();
            printf("publishing reading\n");
            if (isalarmhappened())
            {
                payload = build_alarm_json_string();
                payloadlen = strlen(payload);
                clearalarm();
                len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
                rc = transport_sendPacketBuffer(mysock, buf, len);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            else
            {
                payload = build_json_string();
                payloadlen = strlen(payload);
                len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)payload, payloadlen);
                rc = transport_sendPacketBuffer(mysock, buf, len);
                vTaskDelay(pdMS_TO_TICKS(20000));
            }
            
        }
        printf("disconnecting\n");
        len = MQTTSerialize_disconnect(buf, buflen);
        rc = transport_sendPacketBuffer(mysock, buf, len);

exit:
        transport_close(mysock);
    }
}

void mqtt_task_init()
{
    xTaskCreate(mqtt_pub0sub1_task, "MQTT", DEFAULT_THREAD_STACKSIZE * 3, NULL, MQTT_TASK_PRIO, &TaskHandle);
}

void mqtt_task_start()
{
    uint32_t ulValueToSend = 100UL;
    toStop = 0;
    xTaskNotify(TaskHandle, ulValueToSend, eSetValueWithOverwrite);
}

void mqtt_task_stop()
{
//    uint32_t ulValueToSend = 200UL;
    if (toStop == 0)
    {
        toStop = 1;
    }
}
