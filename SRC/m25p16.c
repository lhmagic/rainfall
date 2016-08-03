#include	"m25p16.h"

void spi_flash_init(void) {
	spi_init();
}

void cmd_rdid(void) {
	cs_low();
	spi_rw_byte(0x9f);
	spi_rw_byte(0x00);
	spi_rw_byte(0x00);
	spi_rw_byte(0x00);
	cs_high(); 
}

uint8_t cmd_rdsr(void) {
uint8_t val;
	
	cs_low();
	spi_rw_byte(0x05);
	val=spi_rw_byte(0x00);
	cs_high();
	
	return val;
}

void cmd_wren(void) {
	cs_low();
	spi_rw_byte(0x06);
	cs_high();
}

void cmd_wrdi(void) {
	cs_low();
	spi_rw_byte(0x04);
	cs_high();
}

uint8_t cmd_wrsr(uint8_t val) {
	cmd_wren();
	cs_low();
	spi_rw_byte(0x01);
	spi_rw_byte(val);
	cs_high();	
	
	return cmd_rdsr();
}

void cmd_sector_erase(uint8_t sector_addr) {
	cmd_wren();
	cs_low();
	spi_rw_byte(0xD8);
	spi_rw_byte(sector_addr);
	spi_rw_byte(0x00);
	spi_rw_byte(0x00);
	cs_high();
	sleep(3);
}

void cmd_bulk_erase(void) {
	cmd_wren();
	cs_low();
	spi_rw_byte(0xC7);
	cs_high();
	sleep(40);
}

void cmd_read_page(uint16_t page_addr, char *data) {
int i;	
	cs_low();
	spi_rw_byte(0x03);
	spi_rw_byte(page_addr>>8);
	spi_rw_byte(page_addr&0xFF);
	spi_rw_byte(0x00);
	
	for(i=0; i<256; i++) {
		data[i] = spi_rw_byte(0x00);
	}
	
	cs_high();
}

void cmd_write_page(uint16_t addr, const char *data) {
int i;
	cmd_wren();
	cs_low();
	spi_rw_byte(0x02);
	spi_rw_byte(addr>>8);
	spi_rw_byte(addr&0xFF);
	spi_rw_byte(0x00);
	
	for(i=0; i<256; i++) {
		spi_rw_byte(data[i]);
	}
	
	cs_high();
	
	delay(5);
}
