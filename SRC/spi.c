#include	"spi.h"

void spi_init(void) {
	
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	SPI1->CR1 |= (SPI_CR1_MSTR);
	SPI1->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2 | SPI_CR2_SSOE | SPI_CR2_FRXTH);
	
	SPI1->CR1 |= SPI_CR1_SPE;
}

uint8_t spi_rw_byte(uint8_t val) {
	while(!(SPI1->SR & SPI_SR_TXE));
	*(uint8_t *)&SPI1->DR = val;
	while(!(SPI1->SR & SPI_SR_RXNE));
	val = (uint8_t)SPI1->DR;
	
	return val;
}
