#include	"param.h"

s_param  rtu_param;

const char *dev_num = (char *)UID_SAVE_ADDR;
const char *rtu_num = (char *)(UID_SAVE_ADDR+10);
const char fw_version[2] = "\x03\x80";
static const char unknown_data[36] = "\x00\x00\x88\xA4\x00\x04\x00\x7B\x7F\xBF\xFD\xBF\xFF\xBF";

static const uint8_t upload_time_table[][2] = {
//start, interval
	0, 0,
	8, 12,
	8, 8,
	8, 6,
	8, 4,
	8, 1,
};

static const uint8_t rainfall_spec_table[5] = {
	0, 1, 2, 5, 10
};

uint8_t get_rainfall_spec(void) {
	return rainfall_spec_table[rtu_param.rainfall_spec];
}

uint8_t is_time_to_report(void) {
	if(is_hour_flag()) {
	uint8_t start, interval, hour;
	start = upload_time_table[rtu_param.upload_period][0];
	interval = upload_time_table[rtu_param.upload_period][1];
	hour = read_hour();
		while(start < 24) {
			if(start%hour != 0) {
				start += interval;
			} else {
				tim15_disable();		//平安报时间到时停止每隔5分钟上传数据
				return 1;
			}
		}
	}
	
	return 0;
}

uint8_t read_local_param(void) {
char *buf;
uint8_t	ret;
	
		buf = (char *)PARAM_SAVE_ADDR;
		ret = parse_param(buf, RCV_CFG_LEN);
		
		return ret;
}

void rs485_handle(char *buf, uint16_t cnt) {
char rsp[] = "\x01\x10\x00\x00\x00\x92\x02\x00";
	
	switch (cnt) {
		case RCV_CFG_LEN:
			flash_write(PARAM_SAVE_ADDR, (uint8_t *)buf, cnt);
			parse_param(buf, cnt);
			clr_pulse_cnt();
			
			for(cnt=0; cnt<8; cnt++) {
				xputc(rsp[cnt]);
			}
			break;
		case READ_CFG_LEN:	
			if(memcmp(buf, "\x03\x03\x00\x00\x00\xA6\xC4\x52", cnt) == 0) {
				construct_rsp(buf, cnt);
				//response code in construct_rsp function.
			}
		case WRITE_UID_LEN:
			//format
			//01 10 	0A 	dev_num 	rtu_num crc16
			update_n_wirte_uid(buf);
			break;
		case 4:
			if(memcmp(buf, "TIME", 4) == 0) {
			uint32_t t;
			char *time;
				time = read_bcd_time();
				for(cnt=0; cnt<6;cnt++) {
					xputc(time[cnt]);
				}
				t = get_pulse_cnt();
				for(cnt=0; cnt<4; cnt++) {
					xputc(*((char *)&t+cnt));
				}
			}
		default:
			break;
	}
}

uint8_t update_n_wirte_uid(char *buf) {
uint16_t crc;
uint8_t temp[20];	
	
	crc = (buf[WRITE_UID_LEN-2]<<8) | (buf[WRITE_UID_LEN-1]);

	if(crc16((uint8_t *)buf, WRITE_UID_LEN-2) != crc)
		return 1;
	
	if(buf[2] != 10)
		return 1;
	
	memcpy(temp, buf+3, 20);
	flash_write(UID_SAVE_ADDR, temp, 20);
	
	xputs("\x01\x10\x0A\xAD\xC7");
	
	return 0;
}

uint8_t parse_param(char *buf, uint16_t cnt) {
uint16_t i, crc;
uint8_t offset;
s_rcv_cfg *cfg;
	
	crc = (buf[RCV_CFG_LEN-2]<<8) | (buf[RCV_CFG_LEN-1]);

	if(crc16((uint8_t *)buf, RCV_CFG_LEN-2) != crc)
		return 1;

	cfg = (s_rcv_cfg *)buf;
	rtu_param.upload_period = cfg->send_time[1];
	rtu_param.rainfall_spec = cfg->rainfall_spec[1];
	
	//server
	for(i=0, offset=0; i<40; i++) {
		if(cfg->server[0][i] == '"') {
			offset++;
			continue;
		}else	if(cfg->server[0][i] == '\r') {
			rtu_param.ip1[i-offset] = 0;
			break;
		}else if(cfg->server[0][i] == ',') {
			rtu_param.ip1[i-offset] = ':';
			continue;
		}
		rtu_param.ip1[i-offset] = cfg->server[0][i];
	}
	
	//phone
	for(i=0, offset=2; i<20; i++) {
		if(cfg->phone[0][i] == '"') {
			offset++;
			continue;
		}else	if(cfg->phone[0][i] == '\r') {
			rtu_param.phone1[i-offset] = 0;
			break;
		}
		rtu_param.phone1[i-offset] = cfg->phone[0][i];
	}	
	
	//apn
	for(i=0, offset=0; i<20; i++) {
		if(cfg->apn[i] == '"') {
			offset++;
			continue;
		}else	if(cfg->apn[i] == '\r') {
			rtu_param.apn[i-offset] = 0;
			break;
		}
		rtu_param.apn[i-offset] = cfg->apn[i];
	}		
	
	//uname
	for(i=0, offset=0; i<20; i++) {
		if(cfg->uname[i] == '"') {
			offset++;
			continue;
		}else	if(cfg->uname[i] == '\r') {
			rtu_param.uname[i-offset] = 0;
			break;
		}
		rtu_param.uname[i-offset] = cfg->uname[i];
	}		

	//passwd
	for(i=0, offset=0; i<20; i++) {
		if(cfg->passwd[i] == '"') {
			offset++;
			continue;
		}else	if(cfg->passwd[i] == '\r') {
			rtu_param.passwd[i-offset] = 0;
			break;
		}
		rtu_param.passwd[i-offset] = cfg->passwd[i];
	}
	
	return 0;
}

void construct_rsp(char *buf, uint16_t len) {
char msg[512];
s_rcv_cfg *cfg;
uint16_t crc, i;	
	
		cfg = (s_rcv_cfg *)PARAM_SAVE_ADDR;
	
		memcpy(msg, "\x03\x03\x00", 3);
		memcpy(msg+3, (const char *)(cfg->send_time), 10);
		memcpy(msg+13, unknown_data, 36);
		memcpy(msg+49, fw_version, 2);
		memcpy(msg+51, (const char *)cfg->bat_type+1, 1);
		memcpy(msg+52, (const char *)cfg->rtu_type+1, 1);
		//pad 2 byte
		memcpy(msg+55, (const char *)(cfg->server[0]), 200);
		memcpy(msg+255, (const char *)(cfg->phone[0]), 100);
		memcpy(msg+355, dev_num, 18);	//serial num
		memcpy(msg+373, (const char *)(cfg->apn), 20);
		memcpy(msg+393, (const char *)(cfg->uname), 10);
		memcpy(msg+403, (const char *)(cfg->passwd), 10);
		crc = crc16((uint8_t *)msg, 413);
		msg[413] = crc >> 8;
		msg[414] = crc & 0xFF;
		memcpy(msg+415, (const char *)(cfg->protocol), 6);
		memcpy(msg+421, (const char *)(cfg->water_encode), 8);
		crc = crc16((uint8_t *)msg, 429);
		msg[429] = crc >> 8;
		msg[430] = crc & 0xFF;
		
		for(i=0; i<RSP_CFG_LEN; i++) {
			xputc(msg[i]);
		}
}
