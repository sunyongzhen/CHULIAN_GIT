#include "gd32f30x.h"
#include <stdio.h>

void rtc_nvic_configuration(void);
void rtc_configuration(void);
uint32_t time_regulate(void);
void time_adjust(void);
void time_display(uint32_t timevar);
void time_show(void);
//uint8_t usart_scanf(uint32_t value);
void rtc_calendar_init(void);
int time_process_cmd(int year,int month,int day,int hour,int minute,int second);
int is_leapyear(uint32_t year);
int is_day_right(uint32_t year, uint32_t month, uint32_t day);
uint32_t time_caculate(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second);
