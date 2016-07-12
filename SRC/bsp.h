#ifndef		__BSP_H__
#define		__BSP_H__

#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	"stm32f051c8.h"
#include	"rtc.h"
#include	"uart.h"
#include	"flash.h"
#include	"crc16.h"
#include	"param.h"
#include	"mg301.h"
#include	"adc.h"


#define		SYSCLK								8000000

#define		PARAM_SAVE_ADDR				0x0800F000
#define		UID_SAVE_ADDR					0x0800F800

#define		USART1_BUF_MAX				512
#define		USART2_BUF_MAX				256

#define		RTU_MSG_SIZE				1024

//	
#define		CHARGE_ON()					(GPIOA->ODR |= GPIO_ODR_15)
#define		CHARGE_OFF()				(GPIOA->ODR &= ~GPIO_ODR_15)

#define		MG301_HALT()				(GPIOB->ODR &= ~0x60)
#define		MG301_PWON()				(GPIOB->ODR |= 0x60)

#define		UNKNOWN_ON()				(GPIOF->ODR |= GPIO_ODR_7)
#define		UNKNOWN_OFF()				(GPIOF->ODR &= ~GPIO_ODR_7)

#define		PWR_LED_ON()				(GPIOB->ODR &= ~GPIO_ODR_12)
#define		PWR_LED_OFF()				(GPIOB->ODR |= GPIO_ODR_12)

#define		IWDG_REFRESH()			IWDG->KR = 0xAAAA

//public function prototype
void board_init(void);
void sleep(uint16_t sec);
void delay(uint16_t ms);
void tim15_init(uint16_t sec);
void tim15_disable(void);
void tim15_handle(void);
uint8_t is_raining(void) ;
void set_raining(void);
void pulse_cnt_handle(void);
uint32_t get_pulse_cnt(void);
void clr_pulse_cnt(void);
void gpio_init(void);
void iwdg_init(void);
uint32_t get_rainfall(void);

#endif		//__BSP_H__
