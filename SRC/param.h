#ifndef		__PARAM_H__
#define		__PARAM_H__

#include	"bsp.h"

#define		RCV_CFG_LEN					375
#define		READ_CFG_LEN				8
#define		RSP_CFG_LEN					431

typedef struct {
	uint8_t upload_period;
	uint8_t rainfall_spec;
	char ip1[40];
	char phone1[20];
	char apn[20];
	char uname[10];
	char passwd[10];
} s_param;

typedef	struct {
	uint8_t header[2];
	uint8_t send_time[2];
	uint8_t rainfall_spec[2];
	uint8_t waterlevel_time[2];
	uint8_t sms_retx[2];
	uint8_t waterlevel_spec[2];
	uint8_t bat_type[2];
	uint8_t rtu_type[2];
	uint8_t server[5][40];
	uint8_t phone[5][20];
	uint8_t crc1[2];
	uint8_t apn[20];
	uint8_t uname[10];
	uint8_t passwd[10];
	uint8_t fix_f829[2];
	uint8_t protocol[5];
	uint8_t water_encode[8];
	uint8_t crc[2];
} s_rcv_cfg;

uint8_t read_local_param(void);
void rs485_handle(char *buf, uint16_t cnt) ;
uint8_t parse_param(char *buf, uint16_t cnt);
void construct_rsp(char *buf, uint16_t len);
uint8_t is_time_to_report(void);

#endif		//__PARAM_H__
