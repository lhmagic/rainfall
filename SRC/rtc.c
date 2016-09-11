#include	"rtc.h" 

volatile static uint8_t hour_flag;

/**********************************
	初始化RTC
 **********************************/
void rtc_init(void) {
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_DBP;
	RCC->BDCR |= RCC_BDCR_LSEON;
	while(!(RCC->BDCR & RCC_BDCR_LSERDY));
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	RCC->BDCR |= RCC_BDCR_RTCEN;
	
	set_rtc_alarm();
}

/**********************************
	设置RTC时间，输入参数：字符串
	"08:00:00"
 **********************************/
void set_time(const char *time_str) {
uint32_t time;
uint8_t hour, min, sec, ht, hu, mt, mu, st, su;	
	
	if((memcmp(time_str+0, "00", 2) < 0) || (memcmp(time_str+0, "24", 2) > 0)) return; //hour
	if((memcmp(time_str+3, "00", 2) < 0) || (memcmp(time_str+3, "60", 2) > 0)) return; //minute
	if((memcmp(time_str+6, "00", 2) < 0) || (memcmp(time_str+6, "60", 2) > 0)) return; //second	
	
	debug("update time.\r\n");
	
	ht = time_str[0]-'0';
	hu = time_str[1]-'0';
	mt = time_str[3]-'0';
	mu = time_str[4]-'0';
	st = time_str[6]-'0';
	su = time_str[7]-'0';

	hour = (ht<<4) + (hu&0x0F);
	min = (mt<<4) + (mu&0x0F);
	sec = (st<<4) + (su&0x0F);	
	
	time = (hour << 16) | (min << 8) | (sec << 0);	

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	RTC->CR |= RTC_CR_BYPSHAD;
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	RTC->PRER = 0x7F00FF;
	RTC->TR = time;
	
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0x00;
	RTC->WPR = 0x55;
}

/**********************************
	设置RTC日期，输入参数：字符串
	"16/06/05"
 **********************************/
void set_date(const char *time_str) {
uint32_t date;
uint8_t year, month, day, yt, yu, mt, mu, dt, du;
	
	if((memcmp(time_str+0, "00", 2) < 0) || (memcmp(time_str+0, "99", 2) > 0)) return; //year
	if((memcmp(time_str+3, "00", 2) < 0) || (memcmp(time_str+3, "12", 2) > 0)) return; //month
	if((memcmp(time_str+6, "00", 2) < 0) || (memcmp(time_str+6, "31", 2) > 0)) return; //day	
	
	debug("update date.\r\n");
	
	yt = time_str[0]-'0';
	yu = time_str[1]-'0';
	mt = time_str[3]-'0';
	mu = time_str[4]-'0';
	dt = time_str[6]-'0';
	du = time_str[7]-'0';
	
	year = (yt<<4) + (yu&0x0F);
	month = (mt<<4) + (mu&0x0F);
	day = (dt<<4) + (du&0x0F);	
	
	date = (year << 16) | (month << 8) | (day << 0);	
	
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	RTC->CR |= RTC_CR_BYPSHAD;
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	RTC->PRER = 0x7F00FF;
	RTC->DR = date;
	
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0x00;
	RTC->WPR = 0x55;
}

/**********************************
	获取当前小时 0~23 DEC
 **********************************/
uint8_t read_hour(void) {
uint8_t hour;
	hour = (RTC->TR >> 16)&0x3F;
	hour = (hour>>4)*10+(hour&0x0F);
	return hour;
}

char * read_bcd_time(void) {
static char time[6];
uint32_t dr, tr;
	dr = RTC->DR & 0xFF1F3F;
	tr = RTC->TR & 0x3F7F7F;
	time[0] = (dr>>16) & 0xFF;
	time[1] = (dr>>8 ) & 0xFF;
	time[2] = (dr>>0 ) & 0xFF;
	time[3] = (tr>>16) & 0xFF;
	time[4] = (tr>>8 ) & 0xFF;
	time[5] = (tr>>0 ) & 0xFF;
	
	return time;
}

void rtc_check_n_update(void) {
static char old_date[10] = "16/01/01";
static char old_time[10] = "23:59:59";
char bcd_time[6];
char new_date[10], new_time[10];
char valid = 1;	
	
	memcpy(bcd_time, read_bcd_time(), 6);
	
	new_date[0] = (bcd_time[0]>>4)+'0'; new_date[1] = (bcd_time[0]&0x0F)+'0'; new_date[2] = '/';
	new_date[3] = (bcd_time[1]>>4)+'0'; new_date[4] = (bcd_time[1]&0x0F)+'0'; new_date[5] = '/';
	new_date[6] = (bcd_time[2]>>4)+'0'; new_date[7] = (bcd_time[2]&0x0F)+'0'; new_date[8] = 0;
	new_time[0] = (bcd_time[3]>>4)+'0'; new_time[1] = (bcd_time[3]&0x0F)+'0'; new_time[2] = ':';
	new_time[3] = (bcd_time[4]>>4)+'0'; new_time[4] = (bcd_time[4]&0x0F)+'0'; new_time[5] = ':';
	new_time[6] = (bcd_time[5]>>4)+'0'; new_time[7] = (bcd_time[5]&0x0F)+'0'; new_time[8] = 0;
	
	if((memcmp(new_date+0, "00", 2) < 0) || (memcmp(new_date+0, "99", 2) > 0)) valid=0; //year
	if((memcmp(new_date+3, "00", 2) < 0) || (memcmp(new_date+3, "12", 2) > 0)) valid=0; //month
	if((memcmp(new_date+6, "00", 2) < 0) || (memcmp(new_date+6, "31", 2) > 0)) valid=0; //day	
	if((memcmp(new_time+0, "00", 2) < 0) || (memcmp(new_time+0, "24", 2) > 0)) valid=0; //hour
	if((memcmp(new_time+3, "00", 2) < 0) || (memcmp(new_time+3, "60", 2) > 0)) valid=0; //minute
	if((memcmp(new_time+6, "00", 2) < 0) || (memcmp(new_time+6, "60", 2) > 0)) valid=0; //second	

	if(valid) {
		memcpy(old_date, new_date, 10);
		memcpy(old_time, new_time, 10);
	} else {
		set_date(old_date);
		set_time(old_time);
		debug("RTC error.\r\n");
	}
}

/**********************************
	设置RTC每小时闹铃中断
 **********************************/
static void set_rtc_alarm(void) {

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->CR &= ~RTC_CR_ALRAE;
	
	while(!(RTC->ISR & RTC_ISR_ALRAWF));
	
	RTC->ALRMAR = RTC_ALRMAR_MSK4 | RTC_ALRMAR_MSK3;
	RTC->ALRMAR &= ~(RTC_ALRMAR_MNT | RTC_ALRMAR_MNU | RTC_ALRMAR_ST | RTC_ALRMAR_SU);
	//分钟和秒数都为零时产生中断
	RTC->ALRMAR |= (0x00<<8);	
	
	RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE;
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);
	
	EXTI->IMR |= EXTI_IMR_IM17;
	EXTI->RTSR |= EXTI_RTSR_RT17;
	EXTI->FTSR |= EXTI_FTSR_FT17;	
	
	RTC->WPR = 0x00;
	RTC->WPR = 0x55;	
}

/**********************************
	RTC中断服务程序
 **********************************/
void rtc_hour_irq_handle(void) {

	if(RTC->ISR & RTC_ISR_ALRAF) {
		RTC->ISR &= ~RTC_ISR_ALRAF;
		EXTI->PR |= EXTI_PR_PIF17;	
		set_hour_flag();
	}
}

/**********************************
	检测RTC小时中断标志
 **********************************/
uint8_t is_hour_flag(void) {
	if(hour_flag == 1) {
		clr_hour_flag();
		return 1;
	}
	
	return 0;
}

/**********************************
	设置RTC小时中断标志
 **********************************/
static void set_hour_flag(void) {
	hour_flag = 1;
}

/**********************************
	清除RTC小时中断标志
 **********************************/
static void clr_hour_flag(void) {
	hour_flag = 0;
}
