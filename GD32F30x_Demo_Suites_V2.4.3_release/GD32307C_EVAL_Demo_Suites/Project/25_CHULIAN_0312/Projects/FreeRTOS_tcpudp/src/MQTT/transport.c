#include "transport.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "string.h"
#include "netdb.h"

static int mysock = -1;

/************************************************************************
** 函数名称: transport_sendPacketBuffer
** 函数功能: 以TCP方式发送数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
    int rc;
    rc = send(sock, buf, buflen, 0);
    return rc;
}

/************************************************************************
** 函数名称: transport_getdata
** 函数功能: 以阻塞的方式接收TCP数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int count：数据长度
** 出口参数: <=0接收数据失败
************************************************************************/
int transport_getdata(unsigned char* buf, int count)
{
    int rc;
    //这个函数在这里不阻塞
    rc = recv(mysock, buf, count, 0);
    return rc;
}



/************************************************************************
** 函数名称: transport_open
** 函数功能: 打开一个接口，并且和服务器 建立连接
** 入口参数: char* servip:服务器域名
**           int   port:端口号
** 出口参数: <0打开连接失败
************************************************************************/
int transport_open(char* servip, int port)
{
    int *sock = &mysock;
    int ret;
    int opt;
    struct sockaddr_in addr;
    struct hostent *l_hostent;
    ip_addr_t ipaddr;
    
    // 解析域名
    l_hostent = lwip_gethostbyname(servip);
    
    //初始换服务器信息
    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    //填写服务器端口号
    addr.sin_port = htons(port);
    //填写服务器IP地址
    memcpy(&addr.sin_addr.s_addr, *l_hostent->h_addr_list, l_hostent->h_length);

    //创建SOCK
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if (*sock != -1)
    {
        printf("sock %d\r\n", *sock);
    }
    //连接服务器
    ret = connect(*sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret != 0)
    {
        //关闭链接
        close(*sock);
        printf("connect failed \r\n");
        //连接失败
        return -1;
    }
    //连接成功,设置超时时间1000ms
    opt = 1000;
    setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(int));

    //返回套接字
    return mysock;
}


/************************************************************************
** 函数名称: transport_close
** 函数功能: 关闭套接字
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int transport_close(int sock)
{

    int rc;
//  rc = close(mysock);
    rc = shutdown(sock, SHUT_WR);
    rc = recv(sock, NULL, (size_t)0, 0);
    rc = close(sock);
    return rc;
}
