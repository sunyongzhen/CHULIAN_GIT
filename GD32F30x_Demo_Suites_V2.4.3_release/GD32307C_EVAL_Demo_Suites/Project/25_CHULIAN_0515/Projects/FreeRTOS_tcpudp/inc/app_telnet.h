#ifndef APP_TELNET_H
#define APP_TELNET_H


// 声明回调函数类型
typedef char (*callback_t)(unsigned char *);

/* function declarations */
/* initialize the telnet application */
void telnet_task_init(void);
void register_telnet_handledata_callback(callback_t callback);

#endif /* APP_TELNET_H */
