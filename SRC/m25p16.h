#ifndef		__M25P16_H__
#define		__M25P16_H__

#include	"bsp.h"

#define		PAGE_SIZE			256
#define		MAX_PAGE			8192
#define		SECTOR_SIZE		65536
#define		MAX_SECTOR		32

#define		cs_low()			(GPIOA->ODR &= ~GPIO_ODR_4)
#define		cs_high()			(GPIOA->ODR |= GPIO_ODR_4)

void spi_flash_init(void);
void cmd_rdid(void);
void cmd_wrdi(void);
uint8_t cmd_rdsr(void);
uint8_t cmd_wrsr(uint8_t val);
void cmd_wren(void);
void cmd_sector_erase(uint8_t sector_addr);
void cmd_bulk_erase(void);
void cmd_read_page(uint16_t addr, char *data);
void cmd_write_page(uint16_t addr, const char *data);

#endif		//__M25P16_H__
