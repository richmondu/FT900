/**
  @file uartrb.h
  @brief Ringbuffer implementation for FT9xx UART.
  @details Includes flow control and hardware buffering.
 */
/*
 * ============================================================================
 * History
 * =======
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 *  ("Bridgetek") subject to the licence terms set out
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

#ifndef _UART_RB_H
#define _UART_RB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief UART Ring Buffer Flow Control */
typedef enum
{
    uartrb_flow_none, /**< No flow control */
    uartrb_flow_rts_cts, /**< Software controlled RTS/CTS flow control */
    uartrb_flow_rts_cts_auto, /**< Hardware controlled RTS/CTS flow control */
    uartrb_flow_dtr_dsr, /**< DTR/DSR flow control */
	uartrb_flow_xon_xoff /**< XON/XOFF flow control */
} uartrb_flow_t;

void uartrb_setup(ft900_uart_regs_t *dev, uartrb_flow_t flow);
uint16_t uartrb_putc(ft900_uart_regs_t *dev, uint8_t val);
uint16_t uartrb_write(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_write_wait(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_read(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_read_wait(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_readln(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_getc(ft900_uart_regs_t *dev, uint8_t *val);
uint16_t uartrb_available(ft900_uart_regs_t *dev);
uint16_t uartrb_waiting(ft900_uart_regs_t *dev);
uint16_t uartrb_peek(ft900_uart_regs_t *dev, uint8_t *buffer, uint16_t len);
uint16_t uartrb_peekc(ft900_uart_regs_t *dev, uint8_t *val);
void uartrb_timeout(ft900_uart_regs_t *dev);
void uartrb_flush_read(ft900_uart_regs_t *dev);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _UART_RB_H */
