#ifndef		__RTC_H__
#define		__RTC_H__

#include	"bsp.h"

//public function prototype
void rtc_init(void);
void set_time(const char *time_str);
void set_date(const char *time_str);
uint8_t read_hour(void);
void rtc_hour_irq_handle(void);
uint8_t is_hour_flag(void);
char * read_bcd_time(void);

//private function prototype
static void set_rtc_alarm(void);
static void set_hour_flag(void);
static void clr_hour_flag(void);

#endif		//__RTC_H__
