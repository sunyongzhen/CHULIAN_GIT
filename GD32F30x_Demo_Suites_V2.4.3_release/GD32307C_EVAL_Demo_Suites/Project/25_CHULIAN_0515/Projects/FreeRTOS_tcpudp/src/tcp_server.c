#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "tcp_server.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/api.h"
#include "app_ota.h"
#include "app_configure.h"
#include "app_rs485net.h"

#pragma pack(1)
#define TCP_SERVER_TASK_PRIO            ( tskIDLE_PRIORITY + 7)
#define MAX_BUF_SIZE                    400
#define TIME_WAITING_FOR_CONNECT        ( ( portTickType ) 500 )


extern int start_record;
extern int stop_record;

struct record_request_packet
{
	uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint16_t chn;
    uint16_t event;   
    uint32_t event_timel;
	uint32_t event_timer;
};


struct recv_request_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint8_t data[256];
};


struct ack_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint16_t data;
};

struct heart_ack_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
};

struct recv_ack_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint8_t data[32];
};

struct record_request_packet create_record_request_packet(uint16_t type, uint16_t seq, uint16_t reserve, uint16_t chn, uint16_t event, uint32_t event_timel, uint32_t event_timer)
{
    struct record_request_packet packet;
	
	packet.len = sizeof(struct record_request_packet);
	packet.version = 0x0001;
	packet.seq = seq;
    packet.type = type;
	packet.reserve = reserve;
    packet.chn = chn;   
    packet.event = event; 
    packet.event_timel = event_timel;
    packet.event_timer = event_timer;
//	
//	printf("record_packet.type = %d\n", packet.type);
//	printf("record_packet.event = %d\n", packet.event);

    return packet;
}

struct ack_packet create_ack_packet(uint16_t type, uint16_t status, uint16_t ver, uint16_t seq, uint32_t reserve)
{

   struct ack_packet packet;
    packet.len = sizeof(struct ack_packet);
    packet.version = ver;   
    packet.seq = seq; 
    packet.type = type;
    packet.reserve = reserve;
	packet.data = status;
//    memset(packet.data, 0 , 32);
//    memcpy(packet.data, data, data_size);
	
//	printf("ack_packet.len = %d\n", packet.len);
//	printf("packet.seq = %d\n", packet.seq);

    return packet;

}


struct heart_ack_packet create_heart_ack_packet(uint16_t type, uint16_t ver, uint16_t seq, uint32_t reserve)
{

    struct heart_ack_packet heart_packet;
    heart_packet.len = sizeof(struct heart_ack_packet);
    heart_packet.version = ver;   
    heart_packet.seq = seq; 
    heart_packet.type = type;
    heart_packet.reserve = reserve;

//	printf("heart_ack_packet.len = %d\n", heart_packet.len);
//	printf("heart_ack_packet.seq = %d\n", heart_packet.seq);

    return heart_packet;

}

static void tcp_server_task(void *arg)
{
	int ret;
	int connfd;
	int listenfd;
	int tcp_port = 24416;
	uint16_t server_seq = 0x0000;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
//	unsigned char buf[MAX_BUF_SIZE];
	

	servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(tcp_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//printf("tcp_server_task..................\n");
	start_record = 0;
 
	while(1)
	{	
		listenfd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenfd == -1) {
			perror("socket error");
			continue;
		}
		ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if (ret == -1) {
			perror("bind error");
			lwip_close(listenfd);
			continue;
		}
		
		ret = listen(listenfd, 5);
		if (ret == -1) {
			perror("listen error");
			lwip_close(listenfd);
			continue;
		}
	
	
		
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddr_len);
        if (connfd == -1) {
            perror("accept error");
			lwip_close(listenfd);
            break;
        }
		
		while(connfd != -1)
		{
			int i = 0;
			struct recv_request_packet recv_req_packet;
			memset(&recv_req_packet, 0, sizeof(struct recv_request_packet));
            ret = recv(connfd, &recv_req_packet, sizeof(recv_req_packet), 0);
            if (ret <= 0) {
                perror("recv error");
				lwip_close(connfd);
				connfd = -1;
                break;
            }
			
//			if(recv_req_packet.len == 0x0000)
//			{
//				printf("*******************null packet\r\n");
//				break;
//			}

			recv_req_packet.len = ntohs(recv_req_packet.len);
            recv_req_packet.version = ntohs(recv_req_packet.version);
            recv_req_packet.seq = ntohs(recv_req_packet.seq);
            recv_req_packet.type = ntohs(recv_req_packet.type);
			recv_req_packet.reserve = ntohl(recv_req_packet.reserve);
			
//			printf("recv_req_packet.len = 0x%04x\n", recv_req_packet.len);
//			printf("recv_req_packet.version = 0x%04x\n", recv_req_packet.version);
//			printf("recv_req_packet.seq = 0x%04x\n", recv_req_packet.seq);
//			printf("recv_req_packet.type = 0x%04x\n", recv_req_packet.type);
//			printf("recv_req_packet.reserve = 0x%08x\n", recv_req_packet.reserve);
//            printf("recv_req_packet.data : ");
//            for(i = 0; i < 25; i++)
//            {
//                printf("0x%02x ", recv_req_packet.data[i]);
//            }
//            printf("\n");
			
			
			if (recv_req_packet.type == 0x0101) {
                uint16_t ack_status = 0x0000;
                struct ack_packet ack_pack = create_ack_packet(0x0101, ack_status, recv_req_packet.version, recv_req_packet.seq, recv_req_packet.reserve);
				
				//host to net
                ack_pack.len = htons(ack_pack.len);
                ack_pack.version = htons(ack_pack.version); 
                ack_pack.seq = htons(ack_pack.seq);
                ack_pack.type = htons(ack_pack.type);
                ack_pack.reserve = htonl(ack_pack.reserve);
                ack_pack.data = htons(ack_pack.data);
				
//				printf("**********ack_pack.seq: %d\n", ack_pack.seq);
                ret = send(connfd, &ack_pack, sizeof(ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
					lwip_close(connfd);
					connfd = -1;
                    break;
                }
                
            } 
            else if (recv_req_packet.type == 0x1001) {
                uint16_t ack_status = {0x0000};
				uint16_t chn = recv_req_packet.data[0] << 8 | recv_req_packet.data[1];
				uint16_t status = recv_req_packet.data[4] << 8 | recv_req_packet.data[5];
                struct ack_packet ack_pack = create_ack_packet(0x1001, ack_status, recv_req_packet.version, recv_req_packet.seq, recv_req_packet.reserve);
				
//				printf("---------- channel %d \t status %d\r\n", chn, status);
				set_camera_status(chn, status);
				
				//host to net
                ack_pack.len = htons(ack_pack.len);
                ack_pack.version = htons(ack_pack.version); 
                ack_pack.seq = htons(ack_pack.seq);
                ack_pack.type = htons(ack_pack.type);
                ack_pack.reserve = htonl(ack_pack.reserve);
                ack_pack.data = htons(ack_pack.data);
				
                ret = send(connfd, &ack_pack, sizeof(ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
					lwip_close(connfd);
					connfd  = -1;
                    break;
                }
            } 
            else if (recv_req_packet.type == 0x0001) {
                struct heart_ack_packet heart_ack_pack = create_heart_ack_packet(0x0001, recv_req_packet.version, recv_req_packet.seq, recv_req_packet.reserve);
				
				//host to net
                heart_ack_pack.len = htons(heart_ack_pack.len);
                heart_ack_pack.version = htons(heart_ack_pack.version); 
                heart_ack_pack.seq = htons(heart_ack_pack.seq);
                heart_ack_pack.type = htons(heart_ack_pack.type);
                heart_ack_pack.reserve = htonl(heart_ack_pack.reserve);
				
                ret = send(connfd, &heart_ack_pack, sizeof(heart_ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
					lwip_close(connfd);
					connfd = -1;
                    break;
                }
            }
			
			if(start_record)
			{
				uint16_t chn_data = 0x0000;
				uint16_t record_status = 0x0001;
				
				
				uint64_t seconds64;
				uint32_t event_time;
				uint32_t seconds1;
				uint32_t seconds2;
				struct record_request_packet record_request_pack;
				

				event_time = rtc_counter_get();
				seconds64 = (uint64_t)event_time;
//				printf("seconds64: %llu\n", seconds64);
				seconds1 = (uint32_t)(seconds64 >> 32);
				seconds2 = (uint32_t)seconds64;

				record_request_pack = create_record_request_packet(0x2001, server_seq++, recv_req_packet.reserve, chn_data, record_status, seconds1, seconds2);
				
//				printf("Seconds1: %u\n", seconds1);
//				printf("Seconds2: %u\n", seconds2);
				
				record_request_pack.len = htons(record_request_pack.len);
				record_request_pack.version = htons(record_request_pack.version);
				record_request_pack.seq = htons(record_request_pack.seq);
				record_request_pack.type = htons(record_request_pack.type);
				record_request_pack.reserve = htonl(record_request_pack.reserve);
				record_request_pack.chn = htons(record_request_pack.chn);
				record_request_pack.event = htons(record_request_pack.event);
				record_request_pack.event_timel = htonl(record_request_pack.event_timel);
				record_request_pack.event_timer = htonl(record_request_pack.event_timer);
				
//				printf("record_request_pack.len = 0x%04x\n", record_request_pack.len);
//				printf("record_request_pack.version = 0x%04x\n", record_request_pack.version);
//				printf("record_request_pack.seq = 0x%04x\n", record_request_pack.seq);
//				printf("record_request_pack.type = 0x%04x\n", record_request_pack.type);
//				printf("record_request_pack.reserve = 0x%08x\n", record_request_pack.reserve);
//				printf("record_request_pack.chn = 0x%04x\n", record_request_pack.chn);
//				printf("record_request_pack.event = 0x%04x\n", record_request_pack.event);
//				printf("record_request_pack.event_timel = 0x%08x\n", record_request_pack.event_timel);
//				printf("record_request_pack.event_timer = 0x%08x\n", record_request_pack.event_timer);

				if (send(connfd, &record_request_pack, sizeof(record_request_pack), 0) == -1) {
					perror("send error");
					lwip_close(connfd);
					connfd = -1;
					break;
				}

				start_record = 0;
//				printf("**************start record over\r\n");
			}
			
			if(stop_record)
			{
				uint16_t chn_data = 0x0000;
				uint16_t record_status = 0x0000;
				
				
				uint64_t seconds64;
				uint32_t event_time;
				uint32_t seconds1;
				uint32_t seconds2;
				struct record_request_packet record_request_pack;
				

				event_time = rtc_counter_get();
				seconds64 = (uint64_t)event_time;
//				printf("seconds64: %llu\n", seconds64);
				seconds1 = (uint32_t)(seconds64 >> 32);
				seconds2 = (uint32_t)seconds64;

				record_request_pack = create_record_request_packet(0x2001, server_seq++, recv_req_packet.reserve, chn_data, record_status, seconds1, seconds2);
				
//				printf("Seconds1: %u\n", seconds1);
//				printf("Seconds2: %u\n", seconds2);
				
				record_request_pack.len = htons(record_request_pack.len);
				record_request_pack.version = htons(record_request_pack.version);
				record_request_pack.seq = htons(record_request_pack.seq);
				record_request_pack.type = htons(record_request_pack.type);
				record_request_pack.reserve = htonl(record_request_pack.reserve);
				record_request_pack.chn = htons(record_request_pack.chn);
				record_request_pack.event = htons(record_request_pack.event);
				record_request_pack.event_timel = htonl(record_request_pack.event_timel);
				record_request_pack.event_timer = htonl(record_request_pack.event_timer);
				
//				printf("record_request_pack.len = 0x%04x\n", record_request_pack.len);
//				printf("record_request_pack.version = 0x%04x\n", record_request_pack.version);
//				printf("record_request_pack.seq = 0x%04x\n", record_request_pack.seq);
//				printf("record_request_pack.type = 0x%04x\n", record_request_pack.type);
//				printf("record_request_pack.reserve = 0x%08x\n", record_request_pack.reserve);
//				printf("record_request_pack.chn = 0x%04x\n", record_request_pack.chn);
//				printf("record_request_pack.event = 0x%04x\n", record_request_pack.event);
//				printf("record_request_pack.event_timel = 0x%08x\n", record_request_pack.event_timel);
//				printf("record_request_pack.event_timer = 0x%08x\n", record_request_pack.event_timer);
				
				if (send(connfd, &record_request_pack, sizeof(record_request_pack), 0) == -1) {
					perror("send error");
					lwip_close(connfd);
					connfd = -1;
					break;
				}

				stop_record = 0;
				
//				printf("**************stop record over\r\n");
			}
		}
		lwip_close(listenfd);
		listenfd = -1;
		//close(connfd);	
	}
}



void tcp_server_init(void)
{
    xTaskCreate(tcp_server_task, "TCP_SERVER", DEFAULT_THREAD_STACKSIZE, NULL, TCP_SERVER_TASK_PRIO, NULL);
}
