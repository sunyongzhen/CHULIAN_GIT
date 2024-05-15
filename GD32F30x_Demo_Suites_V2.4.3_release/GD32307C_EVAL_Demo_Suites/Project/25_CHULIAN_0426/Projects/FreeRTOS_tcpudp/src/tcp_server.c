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


#define TCP_SERVER_TASK_PRIO            ( tskIDLE_PRIORITY + 7)
#define MAX_BUF_SIZE                    400
#define TIME_WAITING_FOR_CONNECT        ( ( portTickType ) 500 )


extern int start_record;
extern int stop_record;

struct request_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint8_t data[32];
};


struct recv_request_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint8_t data[32];
};


struct ack_packet
{
    uint16_t len;
    uint16_t version;
    uint16_t seq;
    uint16_t type;
    uint32_t reserve;
    uint8_t data[32];
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

struct request_packet create_request_packet(uint16_t type, uint8_t *data, size_t data_size)
{
    struct request_packet packet;
    packet.len = sizeof(struct request_packet) + data_size;
    packet.version = 0x0001;   
    packet.seq = 6666; 
    packet.type = type;
    packet.reserve = 0;
    memset(packet.data, 0 , 32);
    memcpy(packet.data, data, data_size);
    return packet;
}

struct ack_packet create_ack_packet(uint16_t type, uint8_t *data, size_t data_size)
{

    struct ack_packet packet;
    packet.len = sizeof(struct ack_packet) + data_size;
    packet.version = 0x0002;   
    packet.seq = 8888; 
    packet.type = type;
    packet.reserve = 0;
    memset(packet.data, 0 , 32);
    memcpy(packet.data, data, data_size);

    return packet;

}


static void tcp_server_task(void *arg)
{
	int ret;
	int tcp_port = 24416;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
//	unsigned char buf[MAX_BUF_SIZE];
	
	
	
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket error");
        return ;
    }
	
	
	servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(tcp_port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	printf("tcp_server_task..................\n");
	start_record = 0;
 
	ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        perror("bind error");
        return ;
    }
	
	ret = listen(listenfd, 5);
    if (ret == -1) {
        perror("listen error");
        return ;
    }
	

	while(1)
	{
		int connfd;
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddr_len);
        if (connfd == -1) {
            perror("accept error");
            break;
        }
		
		while(1)
		{
			int i = 0;
			struct recv_request_packet recv_req_packet;
            ret = recv(connfd, &recv_req_packet, sizeof(recv_req_packet), 0);
            if (ret == -1) {
                perror("recv error");
                break;
            }

            printf("recv_req_packet.type : 0x%04x\n", recv_req_packet.type);
            printf("recv_req_packet.data : ");
            for(i = 0; i < 4; i++)
            {
                printf("0x%02x ", recv_req_packet.data[i]);
            }
            printf("\n");
			
			
			if (recv_req_packet.type == 0x0101) {
                uint8_t ack_status[] = {0x00};
                struct ack_packet ack_pack = create_ack_packet(0x0101, ack_status, sizeof(ack_status));
                ret = send(connfd, &ack_pack, sizeof(ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
                    break;
                }
                
            } 
            else if (recv_req_packet.type == 0x1001) {
                uint8_t ack_status[] = {0x00};
                struct ack_packet ack_pack = create_ack_packet(0x1001, ack_status, sizeof(ack_status));
                ret = send(connfd, &ack_pack, sizeof(ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
                    break;
                }
            } 
            else if (recv_req_packet.type == 0x0001) {
                struct ack_packet ack_pack = create_ack_packet(0x0001, NULL, 0);
                ret = send(connfd, &ack_pack, sizeof(ack_pack), 0);
                if (ret == -1) {
                    perror("send error");
                    break;
                }
            }
			
			if(start_record)
			{
				uint8_t request_data[] = {0x00, 0x11, 0x11, 0x11, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
				struct request_packet req_packet = create_request_packet(0x2001, request_data, sizeof(request_data));
				struct recv_ack_packet recv_ack_pack;
				printf("send_pack_type : 0x%04x\n", req_packet.type);
				printf("send_pack_data : ");
				
				for (i = 0; i < sizeof(request_data); i++) {
					printf("%02X ", req_packet.data[i]);
				}
				printf("\n");

				if (send(connfd, &req_packet, sizeof(req_packet), 0) == -1) {
					perror("send error");
					break;
				}
				

				memset(&recv_ack_pack, 0, sizeof(recv_ack_pack));
				ret = recv(connfd, &recv_ack_pack, sizeof(recv_ack_pack), 0);
				if (ret == -1) {
					perror("recv error");
					break;
				}
				printf("recv_ack_pack_type : 0x%04x\n", recv_ack_pack.type);
				printf("recv_ack_from_server : 0x");
				for(i = 0; i < 2; i++)
				{
					printf("%02x", recv_ack_pack.data[i]);
				}
				printf("\n");
				
				start_record = 0;
			}
			
			if(stop_record)
			{
				uint8_t request_data[] = {0x00, 0x11, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
				struct request_packet req_packet = create_request_packet(0x2001, request_data, sizeof(request_data));
				struct recv_ack_packet recv_ack_pack;
				printf("send_pack_type : 0x%04x\n", req_packet.type);
				printf("send_pack_data : ");
				
				for (i = 0; i < sizeof(request_data); i++) {
					printf("%02X ", req_packet.data[i]);
				}
				printf("\n");

				if (send(connfd, &req_packet, sizeof(req_packet), 0) == -1) {
					perror("send error");
					break;
				}
				

				memset(&recv_ack_pack, 0, sizeof(recv_ack_pack));
				ret = recv(connfd, &recv_ack_pack, sizeof(recv_ack_pack), 0);
				if (ret == -1) {
					perror("recv error");
					break;
				}
				printf("recv_ack_pack_type : 0x%04x\n", recv_ack_pack.type);
				printf("recv_ack_from_server : 0x");
				for(i = 0; i < 2; i++)
				{
					printf("%02x", recv_ack_pack.data[i]);
				}
				printf("\n");
				
				stop_record = 0;
			}
		}
	}
}



void tcp_server_init(void)
{
    xTaskCreate(tcp_server_task, "TCP_SERVER", DEFAULT_THREAD_STACKSIZE, NULL, TCP_SERVER_TASK_PRIO, NULL);
}
