#include	"uart.h"

volatile static char usart1_buf[USART1_BUF_MAX];
volatile static uint16_t usart1_rx_cnt;	
volatile static uint8_t usart1_rx_done;

volatile static char usart2_buf[USART1_BUF_MAX];
volatile static uint16_t usart2_rx_cnt;	
volatile static uint8_t usart2_rx_done;

//connect to RS485
void usart1_init(uint32_t baudrate) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1 | GPIO_MODER_MODER12_1);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR10_0 | GPIO_PUPDR_PUPDR12_0);
	GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2 | GPIO_AFRH_AFRH4);
	GPIOA->AFR[1] |= ((1<<4)|(1<<8)|(1<<16));
	
	USART1->BRR = SYSCLK/baudrate;
	USART1->CR3 |= USART_CR3_DEM;
	USART1->CR2 |= USART_CR2_STOP_1;
	USART1->CR1 |= (USART_CR1_RTOIE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
	NVIC_SetPriority(USART1_IRQn, 1);
	NVIC_EnableIRQ(USART1_IRQn);
}

void usart1_rx_handle(void) {
	
	if(USART1->ISR & USART_ISR_RXNE) {
		char ch = USART1->RDR;
		USART1->CR2 |= USART_CR2_RTOEN;
		USART1->RTOR = 10;
		
		usart1_buf[usart1_rx_cnt++] = USART1->RDR;
		usart1_rx_cnt %= USART1_BUF_MAX;
	} else if(USART1->ISR & USART_ISR_ORE) {
		//overrun err handle
		USART1->ICR |= USART_ICR_ORECF;
	}
	
	if(USART1->ISR & USART_ISR_RTOF) {
		USART1->ICR |= USART_ICR_RTOCF;
		USART1->CR2 &= ~USART_CR2_RTOEN;
		usart1_rx_done = 1;
	}
}

void xputc(const char ch) {
	while(!(USART1->ISR & USART_ISR_TXE));
	USART1->TDR = ch;
}

void xputs(const char *msg) {
	while(*msg) {
		while(!(USART1->ISR & USART_ISR_TXE));
		USART1->TDR = *msg++;
	}	
}

uint8_t is_usart1_rx_done(void) {
	if(usart1_rx_done == 1) {
		usart1_rx_done = 0;
		return 1;
	}
	return 0;
}

char *get_usart1_buf(void) {
char *p;
	p = (char *)usart1_buf;
	return p;
}

uint16_t get_usart1_rx_cnt(void) {
	return usart1_rx_cnt;
}

void usart1_buf_clr(void) {
uint16_t i;
	for(i=0; i<USART1_BUF_MAX; i++) {
		usart1_buf[i] = 0;
	}
	usart1_rx_cnt = 0;
	usart1_rx_done = 0;
}

//connect to MG301
void usart2_init(uint32_t baudrate) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	
	GPIOA->MODER |= (GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR3_0);
	GPIOA->AFR[0] &= ~(GPIO_AFRH_AFRH2 | GPIO_AFRH_AFRH3);
	GPIOA->AFR[0] |= ((1<<8)|(1<<12));
	
	USART2->BRR = SYSCLK/baudrate;
//	USART2->CR2 |= USART_CR2_SWAP;
	USART2->CR1 |= (USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);
}

void usart2_rx_handle(void) {
	
	if(USART2->ISR & USART_ISR_RXNE) {
		char ch = USART2->RDR;
		if(ch == 0) {
			usart2_buf_clr();
		} else {
			usart2_buf[usart2_rx_cnt++] = USART2->RDR;
			usart2_rx_cnt %= USART2_BUF_MAX;
		}
	} else if(USART2->ISR & USART_ISR_ORE) {
		//overrun err handle
		USART2->ICR |= USART_ICR_ORECF;
	}
}

void yputc(const char ch) {
	while(!(USART2->ISR & USART_ISR_TXE));
	USART2->TDR = ch;
}

void yputs(const char *msg) {
	while(*msg) {
		while(!(USART2->ISR & USART_ISR_TXE));
		USART2->TDR = *msg++;
	}
}

//uint8_t is_usart2_rx_done(void) {
//	if(usart2_rx_done == 1) {
//		usart2_rx_done = 0;
//		return 1;
//	}
//	return 0;
//}

char *get_usart2_buf(void) {
char *p;
	p = (char *)usart2_buf;
	return p;
}

uint16_t get_usart2_rx_cnt(void) {
	return usart2_rx_cnt;
}

void usart2_buf_clr(void) {
uint16_t i;
	for(i=0; i<USART2_BUF_MAX; i++) {
		usart2_buf[i] = 0;
	}
	usart2_rx_cnt = 0;
	usart2_rx_done = 0;
}
