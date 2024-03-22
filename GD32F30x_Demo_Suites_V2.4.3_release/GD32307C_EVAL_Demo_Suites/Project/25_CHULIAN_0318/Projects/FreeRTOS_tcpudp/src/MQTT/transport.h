#ifndef __TRANSPORT_H
#define __TRANSPORT_H
#include "gd32f30x.h"
#include "main.h"
/************************************************************************
** 函数名称: transport_sendPacketBuffer
** 函数功能: 以TCP方式发送数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);

/************************************************************************
** 函数名称: transport_getdata
** 函数功能: 以阻塞的方式接收TCP数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int count：数据长度
** 出口参数: <=0接收数据失败
************************************************************************/
int transport_getdata(unsigned char* buf, int count);

/************************************************************************
** 函数名称: transport_open
** 函数功能: 打开一个接口，并且和服务器 建立连接
** 入口参数: char* servip:服务器域名
**           int   port:端口号
** 出口参数: <0打开连接失败
************************************************************************/
int transport_open(char* host, int port);

/************************************************************************
** 函数名称: transport_close
** 函数功能: 关闭套接字
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int transport_close(int sock);

#endif
