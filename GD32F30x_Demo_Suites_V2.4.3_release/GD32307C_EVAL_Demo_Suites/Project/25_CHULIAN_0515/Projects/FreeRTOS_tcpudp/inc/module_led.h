#ifndef MODULE_LED_H
#define MODULE_LED_H

void led_group_init(void);
int led_process_cmd(int cmd_id, int * param, int paramLen);

#endif
