#include	"bsp.h"

extern s_param  rtu_param;
extern const char *dev_num;
extern const char *rtu_num;
extern const char fw_version[2];
static s_var_data local_record[256/sizeof(s_var_data)];


static void rtu_xmit_data(char *msg, uint32_t rainfall, char *time, uint8_t rssi, uint16_t bat_volt, uint16_t solar_volt); 
static void read_param_n_net_puts(char *msg);
static uint16_t get_bat_volt(void) ;
static uint16_t get_solar_volt(void);
static void puts_local_records(char *msg) ;
static void read_page_pointer(void);


int main(void) {
char msg[RTU_MSG_SIZE];

	RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;
	DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;
	DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM15_STOP;	
	board_init();
	read_local_param();
	read_page_pointer();
	
	while(1) {
		IWDG_REFRESH();
		
		if(get_bat_volt() < 115) {
			CHARGE_ON();
		} else if(get_bat_volt() > 140) {
			CHARGE_OFF();
		}
		
		if(is_rcv_nwtime()) {
			update_time();
			usart2_buf_clr();
		}	
				
		if(is_raining() || is_time_to_report()) {
			read_param_n_net_puts(msg);
		}
		
		if(is_ring(rtu_param.phone1)) {
			puts_local_records(msg);
		}
		
		if(is_usart1_rx_done()) {
			rs485_handle(get_usart1_buf(), get_usart1_rx_cnt());
			usart1_buf_clr();
		}
		
		rtc_check_n_update();
	}
}

static void rtu_xmit_data(char *msg, uint32_t rainfall, char *time, uint8_t rssi, uint16_t bat_volt, uint16_t solar_volt) {
uint32_t p_cnt, i;
s_rcv_cfg *cfg;
	
	cfg = (s_rcv_cfg *)PARAM_SAVE_ADDR;
	p_cnt = __REV(rainfall);
	rssi = ((rssi/10)<<4)+rssi%10;
	bat_volt = __REV16(bat_volt);
	solar_volt = __REV16(solar_volt);
	memset(msg, 0, 1024);
	
	memcpy(msg+0x00, &p_cnt, 0x02*2);
	memcpy(msg+0x5C*2, rtu_param.apn, 18);
	memcpy(msg+0x66*2, rtu_param.uname, 8);
	memcpy(msg+0x6B*2, rtu_param.passwd, 8);
	
	memcpy(msg+0x70*2, dev_num, 8);
	memcpy(msg+0x80*2, &solar_volt, 2);
	memcpy(msg+0x81*2, &bat_volt, 2);
	memcpy(msg+0x82*2, time, 6);
	memcpy(msg+0x85*2, rtu_num, 8);
	memcpy(msg+0x89*2, fw_version, 2);
	
	memcpy(msg+0x90*2, cfg->bat_type, 2);
	memcpy(msg+0x91*2, cfg->rtu_type, 2);
	memset(msg+0x92*2+1, 1, 1); //run flag
	memset(msg+0x93*2+1, 5, 1);
	
	memcpy(msg+0xA0*2, cfg->send_time, 2);
	memcpy(msg+0xA1*2, cfg->rainfall_spec, 2);
	memcpy(msg+0xA2*2, cfg->waterlevel_time, 2);
	memcpy(msg+0xA3*2, cfg->sms_retx, 2);
	memcpy(msg+0xA8*2, &rssi, 1);
	memset(msg+0xA8*2+1,0x99, 1);
	memcpy(msg+0xB0*2, cfg->server[0], 40);
	memcpy(msg+0xC4*2, cfg->server[1], 40);
	memcpy(msg+0xD8*2, cfg->server[2], 40);
	memcpy(msg+0xEC*2, cfg->server[3], 40);
	memcpy(msg+0x100*2, cfg->server[4], 40);
	
	memcpy(msg+0x120*2, cfg->phone[0], 40);
	memcpy(msg+0x12A*2, cfg->phone[1], 40);
	memcpy(msg+0x134*2, cfg->phone[2], 40);
	memcpy(msg+0x13E*2, cfg->phone[3], 40);
	memcpy(msg+0x148*2, cfg->phone[4], 40);
	
	for(i=0xB0*2; i<RTU_MSG_SIZE; i++) {
		if((msg[i]=='\r') || (msg[i]=='\n')) {
			msg[i] = ' ';
		}
	}
}

void read_param_n_net_puts(char *msg) {
char header[] = "460029125715486";
char send_success = 0;	
int i, max_record;	

	if(!is_gm301_on()) {
		mg_init();
	}
	
	if(read_local_param() == 0) {
		set_profile(0, rtu_param.ip1, rtu_param.apn, rtu_param.uname, rtu_param.passwd);
		
		if(net_open(0) == 0) {
			net_puts(0, header);
			sleep(5);
			net_read(0, msg, 32);
			if(msg[0] == '#') {
			char date[10], time[10];
				date[0] = msg[1]; date[1] = msg[2]; date[2] = '/';
				date[3] = msg[3]; date[4] = msg[4]; date[5] = '/';
				date[6] = msg[5]; date[7] = msg[6]; date[8] = 0;
				
				time[0] = msg[7]; time[1] = msg[8]; time[2] = ':';
				time[3] = msg[9]; time[4] = msg[10]; time[5] = ':';
				time[6] = msg[11]; time[7] = msg[12]; time[8] = 0;

				set_date(date);
				set_time(time);
			}
			sleep(5);
			rtu_xmit_data(msg, get_rainfall(), read_bcd_time(), get_rssi(), get_bat_volt(), get_solar_volt());
			if(net_write(0, msg, 0x152*2) == 0) {
			char net_msg[32];
				sleep(5);
				net_read(0, net_msg, 32);
				if(strstr(net_msg, "OK") != NULL) {
					send_success = 1;
				}
			}
			net_close(0);
		}
		
		if(send_success == 0) {
			max_record = 256/sizeof(s_var_data);
			if(local_record[max_record-1].flag == 0xA5) {
				if((RTC->BKP1R % (SECTOR_SIZE/PAGE_SIZE)) == 0) {
					cmd_sector_erase(RTC->BKP1R/PAGE_SIZE);
				}
				cmd_write_page(RTC->BKP1R++, (char *)local_record);
				memset((uint8_t *)local_record, 0, 256);
			}
			
			for(i=0; i<max_record; i++) {
				if(local_record[i].flag != 0xA5) {
					local_record[i].flag = 0xA5;
					local_record[i].rssi = get_rssi();
					memcpy(local_record[i].time, read_bcd_time(), 6);
					local_record[i].rainfall = get_rainfall();
					local_record[i].b_volt = get_bat_volt();
					local_record[i].s_volt = get_solar_volt();
					break;
				}
			}			
		}
	}
}

static void read_page_pointer(void) {
char buf[256];
s_var_data *record;	
int i;
	
	for(i=0; i<MAX_PAGE; i++) {
		cmd_read_page(i, buf);
		record = (s_var_data *)buf;
		if(record[i].flag == 0xFF) {
			RTC->BKP1R = i;
			return;
		}
	}
	
	if(i >= MAX_PAGE) {
		RTC->BKP1R = 0;
	}
}

static void puts_local_records(char *msg) {
char buf[PAGE_SIZE];
int i, j, max_record;
s_var_data *record;
char header[] = "460029125715486";
uint8_t server_online=0;	
	
	max_record = 256/sizeof(s_var_data);
	
	if(read_local_param() == 0) {
		set_profile(0, rtu_param.ip1, rtu_param.apn, rtu_param.uname, rtu_param.passwd);
		
		if(net_open(0) == 0) {
			net_puts(0, header);
			sleep(2);
			if(net_read(0, msg, 32) != 0) {
				server_online = 1;
			}
			if(msg[0] == '#') {
			char date[10], time[10];
				date[0] = msg[1]; date[1] = msg[2]; date[2] = '/';
				date[3] = msg[3]; date[4] = msg[4]; date[5] = '/';
				date[6] = msg[5]; date[7] = msg[6]; date[8] = 0;
				
				time[0] = msg[7]; time[1] = msg[8]; time[2] = ':';
				time[3] = msg[9]; time[4] = msg[10]; time[5] = ':';
				time[6] = msg[11]; time[7] = msg[12]; time[8] = 0;

				set_date(date);
				set_time(time);
			}
		}
	}
	
	if(server_online == 0)
		return;
	
	for(i=0; i<MAX_PAGE; i++) {
		IWDG_REFRESH();
		cmd_read_page(i, buf);
		record = (s_var_data *)buf;
		if(record[i].flag == 0xFF) {
			break;
		} else if(record[i].flag != 0xA5) {
			continue;
		}
		
		for(j=0; j<max_record; j++) {
			if(record[j].flag == 0xA5) {
				sleep(1);
				rtu_xmit_data(msg, record[j].rainfall, record[j].time, record[j].rssi, record[j].b_volt, record[j].s_volt);
				net_write(0, msg, 0x152*2);
			}
		}
		
		memset(buf, 0, 256);
		cmd_write_page(i, buf);
	}
	
	RTC->BKP1R = i;
	
	for(j=0; j<max_record; j++) {
		if(local_record[j].flag == 0xA5) {
			sleep(1);
			rtu_xmit_data(msg, local_record[j].rainfall, local_record[j].time, local_record[j].rssi, local_record[j].b_volt, local_record[j].s_volt);
			net_write(0, msg, 0x152*2);
		} else {
			break;
		}
	}
	
	memset((uint8_t *)local_record, 0, 256);
	
	net_close(0);
}

static uint16_t get_bat_volt(void) {
	return get_adc(ADC_CHSELR_CHSEL9)*3.3/409.5*5.7;
}

static uint16_t get_solar_volt(void) {
	return get_adc(ADC_CHSELR_CHSEL8)*3.3/409.5*17.5;
}
