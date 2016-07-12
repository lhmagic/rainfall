#include	"bsp.h"

void RTC_IRQHandler(void) {
	rtc_hour_irq_handle();
}

void EXTI0_1_IRQHandler(void) {
	pulse_cnt_handle();
}

void USART1_IRQHandler(void) {
	usart1_rx_handle();
}

void USART2_IRQHandler(void) {
	usart2_rx_handle();
}

void TIM15_IRQHandler(void) {
	tim15_handle();
}
