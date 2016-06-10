#include	"bsp.h"

void board_init(void) {
	gpio_init();
	rtc_init();
	usart1_init(9600);
	usart2_init(115200);
	PWR_LED_ON();
	CHARGE_ON();
	mg_init();
}

void sleep(uint16_t sec) {
uint16_t i;
	for(i=0; i<sec; i++) 
		delay(1000);
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
		//RTC->BKP0R is used to save pulse count
		RTC->BKP0R++;
	}
}

uint32_t get_pulse_cnt(void) {
	return RTC->BKP0R;
}

void clr_pulse_cnt(void) {
	RTC->BKP0R = 0;
}
