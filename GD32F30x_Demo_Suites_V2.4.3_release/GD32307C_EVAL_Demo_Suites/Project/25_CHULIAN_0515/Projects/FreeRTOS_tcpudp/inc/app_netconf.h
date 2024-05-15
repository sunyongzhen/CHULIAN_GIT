/*!
    \file  netconf.h
    \brief the header file of netconf 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef APP_NETCONF_H
#define APP_NETCONF_H

#define USE_STATIC_IP        0
#define USE_DHCP             1
/* function declarations */
/* initializes the LwIP stack */
void lwip_stack_init(unsigned char use_dhcp);
/* dhcp_task */
void dhcp_task(void * pvParameters);

#endif /* APP_NETCONF_H */
