#include	"bsp.h"

extern s_param  rtu_param;
extern char dev_num[18];
extern char serial_num[18];
extern char fw_version[2];

const char header[] = "460029125715486";

static void rtu_xmit(char *msg);

int main(void) {
char msg[RTU_MSG_SIZE];
	
	board_init();
	if(read_local_param() == 0) {
		set_profile(0, rtu_param.ip1, rtu_param.apn, rtu_param.uname, rtu_param.passwd);
		if(net_open(0) == 0) {
			net_puts(0, header);
			sleep(3);		
			rtu_xmit(msg);
			net_write(0, msg, 0x152*2);
			net_close(0);
		}		
	}
		
	while(1) {
		
		if(is_time_to_report() || is_ring(rtu_param.phone1)) {
			if(read_local_param() == 0) {
				set_profile(0, rtu_param.ip1, rtu_param.apn, rtu_param.uname, rtu_param.passwd);
				
				if(net_open(0) == 0) {
					net_puts(0, header);
					sleep(3);		
					rtu_xmit(msg);
					net_write(0, msg, 0x152*2);
					net_close(0);
				}	else {
					//send_sms(rtu_param.phone1,"connect server timeout.");
				}	
			}				
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

static void rtu_xmit(char *msg) {
char *time;
uint8_t rssi;
uint32_t p_cnt, i;
uint16_t solar_volt=0, bat_volt=0x7a00;
s_rcv_cfg *cfg;
	
	cfg = (s_rcv_cfg *)PARAM_SAVE_ADDR;
	p_cnt = __rev(get_pulse_cnt());
	time = read_bcd_time();
	rssi = get_rssi();
	rssi = ((rssi/10)<<4)+rssi%10;
	
	memset(msg, 0, 1024);
	
	memcpy(msg+0x00, &p_cnt, 0x02*2);
	memcpy(msg+0x5C*2, rtu_param.apn, 18);
	memcpy(msg+0x66*2, rtu_param.uname, 8);
	memcpy(msg+0x6B*2, rtu_param.passwd, 8);
	
	memcpy(msg+0x70*2, dev_num, 8);
	memcpy(msg+0x80*2, &solar_volt, 2);
	memcpy(msg+0x81*2, &bat_volt, 2);
	memcpy(msg+0x82*2, time, 6);
	memcpy(msg+0x85*2, serial_num, 8);
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
