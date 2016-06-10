#include	"adc.h"

void adc_init(void) {
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
	if ((ADC1->CR & ADC_CR_ADEN) != 0){
		ADC1->CR &= (uint32_t)(~ADC_CR_ADEN);
	}
	ADC1->CR |= ADC_CR_ADCAL; 
	while ((ADC1->CR & ADC_CR_ADCAL) != 0) {
	}	
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->CR2 |= RCC_CR2_HSI14ON;
	while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0) {
	}
}

uint32_t get_adc(uint32_t ch) {
uint32_t val=0;
uint8_t i;
	
	ADC1->CR |= ADC_CR_ADEN;
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);
	ADC1->CHSELR = ch;
	ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;
	ADC->CCR |= ADC_CCR_VREFEN;
	for(i=0; i<64; i++) {
		ADC1->CR |= ADC_CR_ADSTART; 
		while ((ADC1->ISR & ADC_ISR_EOC) == 0);
		val += ADC1->DR;
	}
	return val>>6;
}
