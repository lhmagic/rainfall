#include	"bsp.h"

extern s_param  rtu_param;
extern const char *dev_num;
extern const char *rtu_num;
extern const char fw_version[2];

static void rtu_xmit_data(char *msg); 
static void read_param_n_net_puts(char *msg);

int main(void) {
char msg[RTU_MSG_SIZE];
	
	board_init();
	read_param_n_net_puts(msg);
	
	while(1) {
		IWDG_REFRESH();
		
		if(is_time_to_report() || is_ring(rtu_param.phone1)) {
			read_param_n_net_puts(msg);
		}
		
		if(is_rcv_nwtime()) {
			update_time();
		}
		
		if(is_usart1_rx_done()) {
			rs485_handle(get_usart1_buf(), get_usart1_rx_cnt());
			usart1_buf_clr();
		}
	}
}

static void rtu_xmit_data(char *msg) {
char *time;
uint8_t rssi;
uint32_t p_cnt, i;
uint16_t solar_volt=0, bat_volt;
s_rcv_cfg *cfg;
	
	cfg = (s_rcv_cfg *)PARAM_SAVE_ADDR;
	p_cnt = __REV(get_pulse_cnt());
	time = read_bcd_time();
	rssi = get_rssi();
	rssi = ((rssi/10)<<4)+rssi%10;
	bat_volt = get_adc(ADC_CHSELR_CHSEL9)*3.3/409.5*13;
	bat_volt = __REV16(bat_volt);
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
	
	for(i=0; i<RTU_MSG_SIZE; i++) {
		if((msg[i]=='\r') || (msg[i]=='\n')) {
			msg[i] = ' ';
		}
	}
}

void read_param_n_net_puts(char *msg) {
char header[] = "460029125715486";
	
	if(read_local_param() == 0) {
		set_profile(0, rtu_param.ip1, rtu_param.apn, rtu_param.uname, rtu_param.passwd);
		
		if(net_open(0) == 0) {
			net_puts(0, header);
			sleep(2);
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
			rtu_xmit_data(msg);
			net_write(0, msg, 0x152*2);
			net_close(0);
		}	else {
			send_sms(rtu_param.phone1,"connect server timeout.");
		}	
	}		
}
