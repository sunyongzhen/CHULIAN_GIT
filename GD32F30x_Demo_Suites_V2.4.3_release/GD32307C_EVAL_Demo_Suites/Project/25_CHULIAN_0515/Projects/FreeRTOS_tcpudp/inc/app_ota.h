#include <stdio.h>
#include <stdint.h>
int handle_otadata(uint8_t *data);
int version_ack(uint8_t *data);
void erase_page(uint16_t num_pages, uint16_t page_num);