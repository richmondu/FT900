/**
  @file at.h
  @brief
  ESP32 Wifi module.

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-04-25 : Created v1
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */
#if (COMMUNICATION_IO==2) // WiFi
#ifndef _AT_H
#define _AT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PACKED __attribute__((__packed__))

#define AT_STRING_LENGTH(A) (sizeof(A) - 1) // remove the terminating NULL from the length
#define AT_MIN_COMMAND (22 + AT_STRING_LENGTH(CRLF) )
#define AT_MAX_NUMBER 10 // Maximum length of a 32-bit integer expressed as decimal
#define AT_MIN_RESPONSE (AT_STRING_LENGTH(CRLF "ERR CODE:0xnnnnnnnn" CRLF CRLF "ERROR" CRLF))

// Add in possibility of escaping each character and opening/closing
// quotes and null terminator.
#define AT_MAX_SSID 32
#define AT_MAX_SSID_ESCAPED ((AT_MAX_SSID * 2) + 4)
#define AT_MAX_PWD 64
#define AT_MAX_PWD_ESCAPED ((AT_MAX_PWD * 2) + 4)
#define AT_MAX_MAC 18
#define AT_MAX_IP 16
#define AT_MAX_TRANSPORT 8
#define AT_MAX_BSSID AT_MAX_MAC
#define AT_MAX_BSSID_ESCAPED (AT_MAX_MAC + 4)
#define AT_MAX_DOMAIN 256

#define AT_LINK_ID_MAX 4
#define AT_LINK_ID_MIN 0
#define AT_LINK_ID_COUNT (AT_LINK_ID_MAX + 1)


enum PACKED {
	AT_OK,
	AT_NO_DATA = 1,
	AT_DATA_WAITING = 2,
	AT_ERROR_TIMEOUT = -1,
	AT_ERROR_RESPONSE = -2,
	AT_ERROR_QUERY = -3,
	AT_ERROR_SET = -4,
	AT_ERROR_PARAMETERS = -5,
	AT_ERROR_TIMER = -16,
	AT_ERROR_RESOURCE = -17,
	AT_ERROR_NOT_SUPPORTED = -127,
};

struct at_cwgmr_s {
	char at_version[80];
	char sdk_version[80];
	char compile_time[80];
};

struct at_cwuart_s {
	uint32_t baud;
	uint8_t databits;
	uint8_t stopbits;
	uint8_t parity;
	uint8_t flow;
};

struct at_set_cwjap_s {
	char ssid[AT_MAX_SSID];
	char pwd[AT_MAX_PWD];
	char bssid[AT_MAX_MAC];
};

struct at_query_cwjap_s {
	char ssid[AT_MAX_SSID];
	char bssid[AT_MAX_MAC];
	int channel;
	int strength;
};

struct at_cwlap_s {
	int ecn;
	char ssid[AT_MAX_SSID];
	int strength;
	char bssid[AT_MAX_MAC];
	int channel;
};

enum at_link_type {
	at_link_type_tcp = 0,
	at_link_type_udp = 1,
};

enum at_tetype {
	at_tetype_client = 0,
	at_tetype_server = 1,
};

struct at_cipstatus_s {
	int8_t link_id;
	enum at_link_type type;
	char remote_ip[AT_MAX_IP];
	uint16_t remote_port;
	uint16_t local_port;
	enum at_tetype tetype;
};

enum PACKED at_enable {
	at_disable = 0,
	at_enable = 1,
};

struct at_cwdhcp_s {
	enum at_enable station;
	enum at_enable soft_ap;
};

enum PACKED at_mode {
	at_mode_station = 1,
	at_mode_soft_ap = 2,
	at_mode_station_soft_ap = 3,
};

enum PACKED at_echo {
	at_echo_off = at_disable,
	at_echo_on = at_enable,
};

enum PACKED at_cwlap_sort {
	at_cwlap_sort_ordered = 0,
	at_cwlap_sort_unordered = 1,
};

enum PACKED at_cwlap_mask {
	at_cwlap_mask_ecn = 1,
	at_cwlap_mask_ssid = 2,
	at_cwlap_mask_strength = 4,
	at_cwlap_mask_bssid = 8,
	at_cwlap_mask_channel = 16,
	at_cwlap_mask_all = 31,
};

enum PACKED at_cwdhcp_mask {
	at_cwdhcp_station = 1,
	at_cwdhcp_soft_ap = 2,
};

enum PACKED at_cwdhcp_operation {
	at_cwdhcp_disable = at_disable,
	at_cwdhcp_enable = at_enable,
};

enum PACKED at_smartconfig_type {
	at_smartconfig_esp_touch = 1,
	at_smartconfig_airkiss = 2,
	at_smartconfig_esp_touch_airkiss = 3,
};

enum PACKED at_cipstatus {
	at_cipstatus_error = 1,
	at_cipstatus_connected  = 2,
	at_cipstatus_transmission  = 3,
	at_cipstatus_disconnected  = 4,
	at_cipstatus_not_connected  = 5,
};

enum PACKED at_txmode {
	at_txmode_normal = 0,
	at_txmode_passthrough = 1,
};

enum PACKED at_ipd_status {
	at_ipd_status_not_ready = 0,
	at_ipd_status_waiting = 1,
	at_ipd_status_data = 2,
};

enum PACKED at_connection {
	at_not_connected = 0,
	at_connected = 1,
};

// Initialise timers and ports
int8_t at_init(ft900_uart_regs_t *at, ft900_uart_regs_t *monitor);
int8_t at_timeout_comms(int timeout);
int8_t at_timeout_tx_comms(int timeout);
int8_t at_timeout_rx_comms(int timeout);
int8_t at_timeout_cmd(int timeout);
int8_t at_timeout_inet(int timeout);
int8_t at_timeout_ipd(int timeout);
int8_t at_timeout_ap(int timeout);

// Override
int8_t at_command(const char *command, uint16_t *length, char *response, int rxtimeout);
int8_t at_passthrough(void);

// Chapter 3 Basic AT Commands
int8_t at_at(void);
int8_t at_rst(void);
int8_t at_gmr(struct at_cwgmr_s *rsp_gmr);
int8_t at_set_gslp(int timeout);
int8_t at_ate(int8_t on);
int8_t at_query_ate(enum at_echo *echo);
int8_t at_restore(void);
int8_t at_set_uart_cur(struct at_cwuart_s *uart);
int8_t at_query_uart_cur(struct at_cwuart_s *uart);
int8_t at_set_uart_def(struct at_cwuart_s *uart);
int8_t at_query_uart_def(struct at_cwuart_s *uart);
int8_t at_set_sleep(enum at_enable on);
int8_t at_query_sleep(enum at_enable *on);

// Chapter 4 Wi-Fi AT Commands
int8_t at_set_cwmode(enum at_mode mode);
int8_t at_query_cwmode(enum at_mode *mode);
int8_t at_set_cwjap(struct at_set_cwjap_s *cwjap);
int8_t at_query_cwjap(struct at_query_cwjap_s *cwjap);
int8_t at_set_cwlapopt(int8_t sort, int8_t mask);
int8_t at_cwlap(struct at_cwlap_s *cwlap, int8_t *entries);
int8_t at_cwqap(void);
int8_t at_query_cwdhcp(struct at_cwdhcp_s *cwdhcp);
int8_t at_set_cwdhcp(enum at_enable operation, struct at_cwdhcp_s *cwdhcp);
int8_t at_query_cwsap(void); // Not implemented
int8_t at_set_cwsap(void); // Not implemented
int8_t at_query_cwlif(void); // Not implemented
int8_t at_query_cwdhcps(void); // Not implemented
int8_t at_set_cwdhcps(void); // Not implemented
int8_t at_set_cwautoconn(enum at_enable enable);
int8_t at_query_cwautoconn(enum at_enable *enable);
int8_t at_query_cipstamac(char *mac);
int8_t at_set_cipstamac(char *mac);
int8_t at_query_cipapmac(char *mac);
int8_t at_set_cipapmac(char *mac);
int8_t at_query_cipsta(char *ip, char *gateway, char *mask);
int8_t at_set_cipsta(char *ip, char *gateway, char *mask);
int8_t at_query_cipap(char *ip, char *gateway, char *mask);
int8_t at_set_cipap(char *ip, char *gateway, char *mask);
int8_t at_cwstartsmart(void);
int8_t at_set_cwstartsmart(enum at_smartconfig_type type);
int8_t at_cwstopsmart(void);
int8_t at_wps(enum at_enable type);

// Chapter 5 TCP/IP Related AT Commands
int8_t at_query_cipstatus(enum at_cipstatus *status, int8_t *count, struct at_cipstatus_s *cipstatus);
int8_t at_query_cipdomain(char* domain, char* ipChar);
int8_t at_set_cipdomain(char *domain);
int8_t at_set_cipstart_tcp(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive);
int8_t at_set_cipstart_udp(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t local_port, int8_t udp_mode);
int8_t at_set_cipstart_ssl(int8_t link_id, char *remote_ip, uint16_t remote_port, uint16_t tcp_keep_alive);
int8_t at_set_cipsend(int8_t link_id, uint16_t length, uint8_t *buffer);
int8_t at_set_cipsend_udp(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port);
int8_t at_cipsend_start(void);
int8_t at_cipsend_finish(void);
int8_t at_set_cipsendex(int8_t link_id, uint16_t length, uint8_t *buffer);
int8_t at_set_cipsendex_udp(int8_t link_id, uint16_t length, uint8_t *buffer, char *remote_ip, uint16_t remote_port);
int8_t at_set_cipclose(int8_t link_id);
int8_t at_query_cifsr(char *soft_ap, char *station);
int8_t at_query_cipmux(enum at_enable *enable);
int8_t at_set_cipmux(enum at_enable enable);
int8_t at_set_cipserver(enum at_enable mode, uint16_t port);
int8_t at_set_cipmode(enum at_txmode mode);
int8_t at_query_cipmode(enum at_txmode *mode);
int8_t at_set_cipsto(uint16_t timeout);
int8_t at_query_cipsto(uint16_t *timeout);
int8_t at_query_cipdinfo(enum at_enable *enable);
int8_t at_set_cipdinfo(enum at_enable enable);
int8_t at_register_ipd(uint16_t length, uint8_t *buffer);
int8_t at_delete_ipd(uint8_t *buffer);
int8_t at_ipd(int8_t *link_id, uint16_t *length, uint8_t **buffer);
int8_t at_ipd_info(int8_t *link_id, char *remote_ip, uint16_t *remote_port, uint16_t *length, uint8_t **buffer);
uint16_t at_recv(int8_t link_id, uint16_t length, uint8_t *buffer);

int8_t at_is_wifi_connected();
int8_t at_wifi_station_ip();
enum at_connection at_is_server_connected();
enum at_connection at_is_link_id_connected(int8_t link_id);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _AT_H */
#endif
