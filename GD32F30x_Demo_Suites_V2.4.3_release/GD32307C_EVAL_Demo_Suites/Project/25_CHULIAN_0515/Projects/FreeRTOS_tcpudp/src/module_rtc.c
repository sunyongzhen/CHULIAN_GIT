#include "gd32f30x.h"
#include "gd32f307c_eval.h"
#include <stdio.h>
#include "module_key.h"
#include "module_rtc.h"

/* enter the second interruption,set the second interrupt flag to 1 */
__IO uint32_t timedisplay;
uint32_t RTCSRC_FLAG = 0; 

int time_process_cmd(int year,int month,int day,int hour,int minute,int second)
{
	uint32_t tmp_t = 0;
	tmp_t = time_caculate(year, month, day, hour, minute, second);
	 /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(tmp_t);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
	
	return 0;
}

void rtc_calendar_init(){
	 /* NVIC config */
//	printf("-----rtc_calendar_init start-----\r\n");
//    rtc_nvic_configuration();
//	printf( "This is a RTC demo...... \r\n" );
    
    /* get RTC clock entry selection */
    RTCSRC_FLAG = GET_BITS(RCU_BDCTL, 8, 9);
	
	if ((0xA5A5 != bkp_read_data(BKP_DATA_0)) || (0x00 == RTCSRC_FLAG)){
        /* backup data register value is not correct or not yet programmed
        or RTC clock source is not configured (when the first time the program 
        is executed or data in RCU_BDCTL is lost due to Vbat feeding) */
//        printf("RTC not yet configured....\r\n");

        /* RTC configuration */
        rtc_configuration();

//        printf("RTC configured....\r\n ");
	 /* adjust time by values entred by the user on the hyperterminal */
        time_adjust();
        bkp_write_data(BKP_DATA_0, 0xA5A5);
    }else{
        /* check if the power on reset flag is set */
        if (rcu_flag_get(RCU_FLAG_PORRST) != RESET){
//            printf("Power On Reset occurred....\r\n");
        }else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET){
            /* check if the pin reset flag is set */
//            printf("External Reset occurred....\r\n");
        }

        /* allow access to BKP domain */
        pmu_backup_write_enable();

//        printf("No need to configure RTC....\r\n ");
        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();
		/* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
        /* enable the RTC second */
//        rtc_interrupt_enable(RTC_INT_SECOND);
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
    }
	
#ifdef RTCCLOCKOUTPUT_ENABLE
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* disable the tamper pin */
    bkp_tamper_detection_disable();

    /* enable RTC clock output on tamper Pin */
    bkp_rtc_calibration_output_enable();
#endif

    /* clear reset flags */
    rcu_all_reset_flag_clear();

	time_show();
}

/*!
    \brief      configure the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_nvic_configuration(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(RTC_IRQn,2,0);
}

/*!
    \brief      configure the RTC
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32767);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/*!
    \brief      return the time entered by user, using Hyperterminal
    \param[in]  none
    \param[out] none
    \retval     current time of RTC counter value
*/
uint32_t time_regulate(void)
{
    uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;
	uint32_t year = 0xFF, month = 0xFF, day = 0xFF;
	uint32_t time;
//    printf("==============Time Settings==============\r\n");
//	 printf("Please Set year ( > 2020)");
    while (year == 0xFF){
//        year = usart_scanf(2500);
		year = 2024;
    }
//    printf(":  %d\r\n", year);
//	
//    printf("Please Set month");
    while (month == 0xFF){
//        month = usart_scanf(12);
		month = 1;
    }
//    printf(":  %d\r\n", month);
//	
//    printf("Please Set day");
    while (day == 0xFF){
//        day = usart_scanf(31);
		day = 11;
		if(!is_day_right(year, month, day)){
//			printf("day not right\r\n");
			return 0;
		}
    }
//    printf(":  %d\r\n", day);
////	
//    printf("Please Set Hours");
    while (tmp_hh == 0xFF){
//        tmp_hh = usart_scanf(23);
		tmp_hh = 15;
    }
//    printf(":  %d\r\n", tmp_hh);
////	
//    printf("Please Set Minutes");
    while (tmp_mm == 0xFF){
//        tmp_mm = usart_scanf(59);
		tmp_mm = 00;
    }
//    printf(":  %d\r\n", tmp_mm);
////	
//    printf("Please Set Seconds");
    while (tmp_ss == 0xFF){
//        tmp_ss = usart_scanf(59);
		tmp_ss = 00;
    }
//    printf(":  %d\r\n", tmp_ss);
	
	time = time_caculate(year, month, day, tmp_hh, tmp_mm, tmp_ss);
    /* return the value  store in RTC counter register */
    return time;
}

/*!
    \brief      adjust time 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_adjust(void)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(time_regulate());
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/*!
    \brief      display the current time 
    \param[in]  timeVar: RTC counter value
    \param[out] none
    \retval     none
*/
void time_display(uint32_t timevar)
{
    uint32_t hour = 0, minute = 0, second = 0;
	uint32_t year = 0, month = 0, day = 0;
	uint32_t daycount = 0, i;
	int daynum[12]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	int daynum_leapyear[12]={0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
	
//	time = ((year-2020-1)*365 + (year-2020-1)/4 + daycount_leapyear[month-1]  + (day-1))*86400
//				+ hour*3600 + minute*60 + second;
	timevar -= 1609430399;
    daycount = timevar / 86400;
	timevar %= 86400;
	/* compute  year*/
	year = daycount*4 / (365*4 + 1) ;
	daycount = daycount - year*365 - year/4;
	year = year + 2020 + 1;
	
	 /* compute  month*/
	for(i = 0; i < 12; i ++){
		if(is_leapyear(year)){
			if(daycount >= daynum_leapyear[i]){
				month = i + 1;
				/* compute day*/
				day = daycount - daynum_leapyear[i] + 1;
			}else{
				break;
			}
		}
		else{
			if(daycount >= daynum[i]){
				month = i + 1;
				/* compute day*/
				day = daycount - daynum[i] + 1;
			}else{
				break;
			}
		}
	}
    /* compute  hours */
    hour = timevar / 3600;
    /* compute minutes */
    minute = (timevar % 3600) / 60;
    /* compute seconds */
    second = (timevar % 3600) % 60;

//    printf("\tTime: %0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d\r\n\r\n",year, month, day, hour, minute, second);
}

/*!
    \brief      show the current time (HH:MM:SS) on the Hyperterminal
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_show(void)
{
//    printf("\r\n");
	timedisplay = 1;
	if (timedisplay == 1){
		/* display current time */
		time_display(rtc_counter_get());
		timedisplay = 0;
	}
}

int is_leapyear(uint32_t year)
{
	if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0){
		return 1;
	}else{
		return 0;
	}
}

int is_day_right(uint32_t year, uint32_t month, uint32_t day)
{
	if(day > 31 || day <= 0 || month <= 0 || month > 12 || year < 2020){
		return 0;
	}
	if(month == 4 || month == 6 || month == 9|| month == 11){
		if(day > 30){
			return 0;
		}
	}
	if(is_leapyear(year) && month == 2 && day > 29){
		return 0;
	}
	if(!is_leapyear(year) && month == 2 && day > 28){
//		printf("not leap year\r\n");
		return 0;
	}
	return 1;
}

uint32_t time_caculate(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
	int daycount[12]={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	int daycount_leapyear[12]={0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
	uint32_t time = 0;
	if(is_leapyear(year)){
		time = ((year-2020-1)*365 + (year-2020-1)/4 + daycount_leapyear[month-1]  + (day-1))*86400
				+ hour*3600 + minute*60 + second + 1609430399;
	}
	else{
		time = ((year-2020-1)*365 + (year-2020-1)/4 + daycount[month-1] + (day-1))*86400
			+ hour*3600 + minute*60 + second + 1609430399;
	}
	return time;
}
/*!
    \brief      get numeric values from the hyperterminal
    \param[in]  value: input value from the hyperterminal
    \param[out] none
    \retval     input value in BCD mode
*/
//uint8_t usart_scanf(uint32_t value)
//{
//    uint32_t index = 0;
//    uint32_t tmp[2] = {0, 0};

//    while (index < 2){
//        /* loop until RBNE = 1 */
//        while (usart_flag_get(USART0, USART_FLAG_RBNE) == RESET);
//        tmp[index++] = (usart_data_receive(USART0));

//        if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39)){
//            printf("\n\rPlease enter valid number between 0 and 9\n");
//            index--;
//        }
//    }
//    /* calculate the Corresponding value */
//    index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);
//    /* check */
//    if (index > value){
//        printf("\n\rPlease enter valid number between 0 and %d\n", value);
//        return 0xFF;
//    }
//    return index;
//}
