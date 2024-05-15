/*!
    \file  hello_gigadevice.c
    \brief TCP server demo program

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "hello_gigadevice.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "lwip/api.h"
#include "app_telnet.h"

#define TELNET_TASK_PRIO  ( tskIDLE_PRIORITY + 5 )
#define MAX_NAME_SIZE      32

#define GREETING           "\n\r======= Deepano Test =======\
                            \n\r== GD32 ==\
                            \n\r== Telnet SUCCESS==\
                            \n\rInput commands\r\n"
#define TIPS               "\r\nECHO:"
#define SUCCESS            "SUCCESS\r\n"
#define ERROR              "ERROR\r\n"

#if LWIP_SOCKET
#include "lwip/sockets.h"

struct recev_packet
{
    int length;
    unsigned char bytes[MAX_NAME_SIZE];
    int done;
} name_recv;

// 声明函数指针变量，指针用来保存数据处理的接口
callback_t telnet_handledata_ptr = NULL;

void register_telnet_handledata_callback(callback_t callback);
static char telnet_handledata(unsigned char * data);
static err_t telnet_recv(int fd, void *data, int len);

/*!
    \brief      called when a data is received on the telnet connection
    \param[in]  fd: the socket id which to send data
    \param[in]  data: pointer to the data received
    \param[in]  len: size of the data
    \param[out] none
    \retval     err_t: error value
*/
static err_t telnet_recv(int fd, void *data, int len)
{
    char *c;
    int i;
    int done;
    char ret;
    done = 0;	
	//printf("data:%s\n", data);
    c = (char *)data;
    /* telnet 命令以\r 或 \n结尾认为发送完成done设置为1，每次最大长度32字节，超过32字节会分包发送，丢弃超过32字节的部分 */

	//printf("len = %d\n", len);
    for (i = 0; i < len && !done; i++) {
		//printf("recv data[%d]:%c\n", i, c[i]);
        done = ((c[i] == '\r') || (c[i] == '\n'));
    }
    // printf("done %d i %d len %d\n", done, i, len);
    /* when the packet length received is no larger than MAX_NAME_SIZE */
    if (0 == name_recv.done) {
        /* havn't received the end of the packet, so the received data length
           is the configured socket reception limit--MAX_NAME_SIZE */
        if (0 == done) {
			//printf("recv data: %s\n", data);
            /* 长度已达到32字节，本次直接处理数据 */
            if (len == MAX_NAME_SIZE)
            {
				//printf("11111111\n");
                memcpy(name_recv.bytes, data, len);
                name_recv.length = len;
                done = 1;
            }
            else
            {
				//printf("22222222\n");
                /* 没有收到\r\n且长度不超过32字节, 保存收到的数据等待\r\n */
                memset(name_recv.bytes, 0, MAX_NAME_SIZE);
                memcpy(name_recv.bytes, data, len);
            }
            name_recv.length = len;
            name_recv.done = 0;
            /* have received the end of the packet */
        } else {
			//printf("333333333\n");
			memset(name_recv.bytes, 0, MAX_NAME_SIZE);
            memcpy(name_recv.bytes, data, len-2);
            name_recv.done = 1;
        }
    }

    if (1 == done) {
		//printf("name_recv.bytes1 : %s\n", name_recv.bytes);
        ret = telnet_handledata(name_recv.bytes);
        //send(fd, (void *)&TIPS, sizeof(TIPS), 0);
		//send(fd, name_recv.bytes, strlen((const char *)name_recv.bytes), 0);
	//	printf("telnet ret == %d\n", ret);
	//	printf("name_recv.bytes2 : %s\n", name_recv.bytes);
        if (ret != 0)
        {
            send(fd, (void *)&ERROR, sizeof(ERROR), 0);
			
        }
        else
        {
//            send(fd, (void *)&SUCCESS, sizeof(SUCCESS), 0);
            /* 调试ECHO消息 */
            send(fd, name_recv.bytes, strlen((const char *)name_recv.bytes), 0);
            send(fd, "\r\n", 2, 0);
        }


        name_recv.done = 0;
        name_recv.length = 0;
    }

    return ERR_OK;
}


/*!
    \brief      telnet task
    \param[in]  arg: user supplied argument
    \param[out] none
    \retval     none
*/
static void telnet_task(void *arg)
{
    int ret;
    int sockfd = -1, newfd = -1;
    uint32_t len;
    int tcp_port = 23;
    int recvnum;
    struct sockaddr_in svr_addr, clt_addr;
    char buf[MAX_NAME_SIZE];

    /* bind to port 23 at any interface */
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(tcp_port);
    svr_addr.sin_addr.s_addr = htons(INADDR_ANY);

    name_recv.length = 0;
    name_recv.done = 0;

    while (1) {
        /* create a TCP socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }

        ret = bind(sockfd, (struct sockaddr *)&svr_addr, sizeof(svr_addr));
        if (ret < 0) {
            lwip_close(sockfd);
            sockfd = -1;
            continue;
        }

        /* listen for incoming connections (TCP listen backlog = 1) */
        ret = listen( sockfd, 1 );
        if (ret < 0) {
            lwip_close(sockfd);
            continue;
        }

        len = sizeof(clt_addr);

        /* grab new connection */
        newfd = accept(sockfd, (struct sockaddr *)&clt_addr, (socklen_t *)&len);
        if (-1 != newfd) {
            send(newfd, (void *)&GREETING, sizeof(GREETING), 0);
        }

        while (-1 != newfd) {
            /* reveive packets, and limit a reception to MAX_NAME_SIZE bytes */
            /* 经测试，数据回车后会分成数据内容和\r\n分开发送，无操作时也会发送数据 */
			//fflush(stdin);
            recvnum = recv(newfd, buf, MAX_NAME_SIZE, 0);
            if (recvnum <= 0) {
                lwip_close(newfd);
                newfd = -1;
                break;
            }
            /* 接收命令 */
            telnet_recv(newfd, buf, recvnum);
            /* TODO: 查询，将结果发送出去 */
			
        }

        lwip_close(sockfd);
        sockfd = -1;
    }
}
#endif /* LWIP_SOCKET */


/**
 * @function [telnet的任务初始化]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-20T19:53:32+0800
 * @version  [1.0.0]
 */
void telnet_task_init(void)
{
    xTaskCreate(telnet_task, "TELNET", DEFAULT_THREAD_STACKSIZE, NULL, TELNET_TASK_PRIO, NULL);
}

/**
 * @function [注册回调函数，回调函数处理字符串数据]
 * @Author   xujh@deepano.com
 * @DataTime 2023-12-20T19:18:39+0800
 * @version  [1.0.0]
 * @param    data                     [程序接口]
 */
void register_telnet_handledata_callback(callback_t callback)
{
    if (telnet_handledata_ptr == NULL)
    {
        telnet_handledata_ptr = callback;
    }
    else
    {
//        printf("handle data function already registered \r\n");
    }
}

static char telnet_handledata(unsigned char * data)
{
    char ret = 1;
    if (telnet_handledata_ptr)
    {
        ret = telnet_handledata_ptr(data);
    }
    else
    {
//        printf("handle data function not registered \r\n");
    }
    return ret;
}
