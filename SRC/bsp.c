#include	"bsp.h"

void board_init(void) {
	iwdg_init();
	gpio_init();
	adc_init();
	rtc_init();
	usart1_init(9600);
	usart2_init(115200);
	PWR_LED_ON();
	CHARGE_ON();
	mg_init();
}

void sleep(uint16_t sec) {
uint16_t i;
	for(i=0; i<sec; i++) {
		delay(1000);
		IWDG_REFRESH();
	}
}

void delay(uint16_t ms) {
	if(ms <= 0)
		return;
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC = SYSCLK/1000-1;
	TIM14->ARR = ms;
	
	TIM14->CNT = 1;
	TIM14->CR1 |= TIM_CR1_CEN;
	TIM14->SR &= ~TIM_SR_UIF;
	while((TIM14->SR & TIM_SR_UIF) != TIM_SR_UIF);
	TIM14->CR1 &= ~TIM_CR1_CEN;
	
	RCC->APB1ENR &= ~RCC_APB1ENR_TIM14EN;
}

void tim15_init(uint16_t sec) {
	RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
	TIM15->PSC = 64000;
	TIM15->ARR = 125*sec;
	
	TIM15->CNT = 1;
	TIM15->CR1 |= TIM_CR1_URS;
	TIM15->SR &= ~TIM_SR_UIF;
	TIM15->CR1 |= TIM_CR1_CEN;
	TIM15->DIER |= TIM_DIER_UIE;
//	TIM15->SR &= ~TIM_SR_UIF;
//	NVIC_ClearPendingIRQ(TIM15_IRQn);
	NVIC_SetPriority(TIM15_IRQn, 0);
	NVIC_EnableIRQ(TIM15_IRQn);	
}

void tim15_disable(void) {
	TIM15->CR1 &= ~TIM_CR1_CEN;
	RCC->APB2ENR &= ~RCC_APB2ENR_TIM15EN;
}

void tim15_handle(void) {
	if(TIM15->SR & TIM_SR_UIF) {
		TIM15->SR &= ~TIM_SR_UIF;
		set_raining();
	}		
}

static uint8_t raining;
uint8_t is_raining(void) {
uint8_t ret = raining;	
	raining = 0;
	return ret;
}

void set_raining(void) {
	raining = 1;
}

void iwdg_init(void) {
	IWDG->KR = 0xCCCC; 
	IWDG->KR = 0x5555;
	IWDG->PR = IWDG_PR_PR; 
	IWDG->RLR = (40000/256)*2; 
	while (IWDG->SR) ;
	IWDG->KR = 0xAAAA; 
}

void gpio_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
	
	GPIOA->MODER |= GPIO_MODER_MODER15_0;
	GPIOA->ODR |= GPIO_ODR_15;
	
	GPIOB->MODER |= (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 |\
									 GPIO_MODER_MODER6_0 | GPIO_MODER_MODER12_0);
	
	GPIOF->MODER |= GPIO_MODER_MODER7_0;
	
	GPIOB->ODR &= ~GPIO_ODR_12;		//PWR LED
	
	GPIOB->MODER |= GPIO_MODER_MODER1 | GPIO_MODER_MODER0;
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR0);	//PB0 PB1 AIN
	
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;
	//PA0 INTERRUPT CONFIGURATION
	EXTI->IMR |= EXTI_IMR_IM0;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	EXTI->FTSR |= 0x01;
	NVIC_SetPriority(EXTI0_1_IRQn, 0);
	NVIC_EnableIRQ(EXTI0_1_IRQn);		
}

void pulse_cnt_handle(void) {
	if(EXTI->PR & EXTI_PR_PIF0) {
		EXTI->PR |= EXTI_PR_PIF0;	
		if((GPIOA->IDR & GPIO_IDR_0) == 0) {
			//RTC->BKP0R is used to save pulse count
			RTC->BKP0R++;
			set_raining();
			tim15_init(300);
		}
	}
}

uint32_t get_pulse_cnt(void) {
	return RTC->BKP0R;
}

void clr_pulse_cnt(void) {
	RTC->BKP0R = 0;
}

uint32_t get_rainfall(void) {
	return get_pulse_cnt() * get_rainfall_spec();
}
