#ifndef MODULE_KEY_H
#define MODULE_KEY_H

#include "gd32f30x.h"

extern volatile int key_bitmap;
int key_process_cmd(int cmd_id, int * param, int paramLen);
void key_group_init(void);
int get_key_value(void);
void pulse_gpio_set(bit_status status);

#endif
