#include	"rtc.h" 

volatile static uint8_t hour_flag;

/**********************************
	��ʼ��RTC
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
	����RTCʱ�䣬����������ַ���
	"08:00:00"
 **********************************/
void set_time(const char *time_str) {
uint32_t time;
	
	time = ((time_str[0]-'0') << 20) | ((time_str[1]-'0') << 16) |\
				 ((time_str[3]-'0') << 12) | ((time_str[4]-'0') << 8) | \
				 ((time_str[6]-'0') << 4) | ((time_str[7]-'0') << 0);

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	RTC->PRER = 0x7F00FF;
	RTC->TR = time;
	
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0x00;
}

/**********************************
	����RTC���ڣ�����������ַ���
	"16/06/05"
 **********************************/
void set_date(const char *time_str) {
uint32_t date;
	
	date = ((time_str[0]-'0') << 20) | ((time_str[1]-'0') << 16) |\
				 ((time_str[3]-'0') << 12) | ((time_str[4]-'0') << 8) | \
				 ((time_str[6]-'0') << 4) | ((time_str[7]-'0') << 0);
	
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	RTC->PRER = 32767;
	RTC->DR = date;
	
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0x00;
}

/**********************************
	��ȡ��ǰСʱ 0~23 DEC
 **********************************/
uint8_t read_hour(void) {
uint8_t hour;
	hour = (RTC->TR >> 16)&0x7F;
	hour = (hour>>4)*10+(hour&0x0F);
	return hour;
}

char * read_bcd_time(void) {
static char time[6];
uint32_t dr, tr;
	dr = RTC->DR & 0xFF1FFF;
	tr = RTC->TR;
	time[0] = (dr>>16) & 0xFF;
	time[1] = (dr>>8 ) & 0xFF;
	time[2] = (dr>>0 ) & 0xFF;
	time[3] = (tr>>16) & 0xFF;
	time[4] = (tr>>8 ) & 0xFF;
	time[5] = (dr>>0 ) & 0xFF;
	
	return time;
}

/**********************************
	����RTCÿСʱ�����ж�
 **********************************/
static void set_rtc_alarm(void) {

	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->CR &= ~RTC_CR_ALRAE;
	
	while(!(RTC->ISR & RTC_ISR_ALRAWF));
	
	RTC->ALRMAR = RTC_ALRMAR_MSK4 | RTC_ALRMAR_MSK3;
	RTC->ALRMAR &= ~(RTC_ALRMAR_MNT | RTC_ALRMAR_MNU | RTC_ALRMAR_ST | RTC_ALRMAR_SU);
	//���Ӻ�������Ϊ��ʱ�����ж�
	RTC->ALRMAR |= (0x00<<8);	
	
	RTC->CR |= RTC_CR_ALRAE | RTC_CR_ALRAIE;
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);
	
	EXTI->IMR |= EXTI_IMR_IM17;
	EXTI->RTSR |= EXTI_RTSR_RT17;
	EXTI->FTSR |= EXTI_FTSR_FT17;	
	
	RTC->WPR = 0x00;
	RTC->WPR = 0x00;	
}

/**********************************
	RTC�жϷ������
 **********************************/
void rtc_hour_irq_handle(void) {

	if(RTC->ISR & RTC_ISR_ALRAF) {
		RTC->ISR &= ~RTC_ISR_ALRAF;
		EXTI->PR |= EXTI_PR_PIF17;	
		
		set_hour_flag();
	}
}

/**********************************
	���RTCСʱ�жϱ�־
 **********************************/
uint8_t is_hour_flag(void) {
	if(hour_flag == 1) {
		clr_hour_flag();
		return 1;
	}
	
	return 0;
}

/**********************************
	����RTCСʱ�жϱ�־
 **********************************/
static void set_hour_flag(void) {
	hour_flag = 1;
}

/**********************************
	���RTCСʱ�жϱ�־
 **********************************/
static void clr_hour_flag(void) {
	hour_flag = 0;
}
