/**
    @file

    @brief
    UART


 **/
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

#ifndef FT900_UART_SIMPLE_H_
#define FT900_UART_SIMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* INCLUDES ************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "ft900_uart_registers.h"
#include <registers/ft900_registers.h>

/* CONSTANTS ***********************************************************************/
/** @def UART_DIVIDER_110_BAUD 
 *  @brief Predefined divider for 110 baud */
#define UART_DIVIDER_110_BAUD (56818)
/** @def UART_DIVIDER_150_BAUD 
 *  @brief Predefined divider for 150 baud */
#define UART_DIVIDER_150_BAUD (55555)
/** @def UART_DIVIDER_300_BAUD 
 *  @brief Predefined divider for 300 baud */
#define UART_DIVIDER_300_BAUD (27778)
/** @def UART_DIVIDER_1200_BAUD 
 *  @brief Predefined divider for 1200 baud */
#define UART_DIVIDER_1200_BAUD (20833)
/** @def UART_DIVIDER_2400_BAUD 
 *  @brief Predefined divider for 2400 baud */
#define UART_DIVIDER_2400_BAUD (10417)
/** @def UART_DIVIDER_4800_BAUD 
 *  @brief Predefined divider for 4800 baud */
#define UART_DIVIDER_4800_BAUD (5208)
/** @def UART_DIVIDER_9600_BAUD 
 *  @brief Predefined divider for 9600 baud */
#define UART_DIVIDER_9600_BAUD (2604)
/** @def UART_DIVIDER_19200_BAUD 
 *  @brief Predefined divider for 19200 baud */
#define UART_DIVIDER_19200_BAUD (1302)
/** @def UART_DIVIDER_31250_BAUD 
 *  @brief Predefined divider for 31250 baud */
#define UART_DIVIDER_31250_BAUD (800)
/** @def UART_DIVIDER_38400_BAUD 
 *  @brief Predefined divider for 38400 baud */
#define UART_DIVIDER_38400_BAUD (651)
/** @def UART_DIVIDER_57600_BAUD 
 *  @brief Predefined divider for 57600 baud */
#define UART_DIVIDER_57600_BAUD (434)
/** @def UART_DIVIDER_115200_BAUD 
 *  @brief Predefined divider for 115200 baud */
#define UART_DIVIDER_115200_BAUD (217)
/** @def UART_DIVIDER_230400_BAUD 
 *  @brief Predefined divider for 230400 baud */
#define UART_DIVIDER_230400_BAUD (109)
/** @def UART_DIVIDER_460800_BAUD 
 *  @brief Predefined divider for 460800 baud */
#define UART_DIVIDER_460800_BAUD (54)
/** @def UART_DIVIDER_921600_BAUD 
 *  @brief Predefined divider for 921600 baud */
#define UART_DIVIDER_921600_BAUD (27)
/** @def UART_DIVIDER_1000000_BAUD 
 *  @brief Predefined divider for 1000000 baud */
#define UART_DIVIDER_1000000_BAUD (25)

/* TYPES ***************************************************************************/
/** @brief UART Data bits */
typedef enum
{
	uart_data_bits_5 = 5,   /**< 5 data bits */
	uart_data_bits_6,       /**< 6 data bits */
	uart_data_bits_7,       /**< 7 data bits */
	uart_data_bits_8        /**< 8 data bits */
} uart_data_bits_t;

/** @brief UART Parity bits */
typedef enum
{
	uart_parity_none,       /**< No parity */
	uart_parity_odd,        /**< Odd parity */
	uart_parity_even        /**< Even parity */
} uart_parity_t;

/** @brief UART Stop bits */
typedef enum
{
	uart_stop_bits_1,       /**< 1 stop bit */
	uart_stop_bits_1_5,     /**< 1.5 stop bit */
	uart_stop_bits_2        /**< 2 stop bit */
} uart_stop_bits_t;     

/** @brief UART Interrupts */
typedef enum
{
	uart_interrupt_none = 0x01,            /**< No Interrupt */
	uart_interrupt_tx = 0x02,              /**< Transmit Interrupt */
	uart_interrupt_rx = 0x04,              /**< Receive Interrupt */
	uart_interrupt_dcd_ri_dsr_cts = 0x00,  /**< DCD/RI/DSR/CTS Change Interrupt */
	uart_interrupt_xon_xoff = 0x10,        /**< In-band flow control Interrupt
     	 	 	 	 	 	 	 	 	 	 	 (in 16950 mode only)*/
	uart_interrupt_rts_cts = 0x20,         /**< Out-of-band flow control Interrupt
												(in 16950 mode only)*/
} uart_interrupt_t;

/** @brief UART Flow Control */
typedef enum
{
	uart_flow_none,             /**< No flow control */
	uart_flow_rts_cts,			/**< RTS/CTS flow control */
	uart_flow_dtr_dsr,			/**< DTR/DSR flow control */
	uart_flow_xon_xoff			/**< XON/XOFF flow control */
} uart_flow_t;

/** @brief UART Mode Control */
typedef enum
{
	uart_mode_16450,             /**< 16450 mode No FIFO enabled */
	uart_mode_16550,             /**< 16550 mode 16 byte FIFO enabled */
	uart_mode_16650,             /**< 16650 mode 128 byte FIFO enabled, autoRTS/CTS, XON/XOFF */
	uart_mode_16750,             /**< 16750 mode 128 byte FIFO enabled, autoRTS/CTS */
	uart_mode_16950,             /**< 16950 mode 128 byte FIFO enabled, autoRTS/CTS, autoDTR/DSR, XON/XOFF, RS485 */
} uart_mode_t;

/* GLOBAL VARIABLES ****************************************************************/

/* MACROS **************************************************************************/

/* FUNCTION PROTOTYPES *************************************************************/

/** @brief Open a UART for communication
 *
 *  @param dev The device to use
 *  @param prescaler The value of the prescaler
 *  @param divisor The value of the divisor
 *  @param databits The number of data bits
 *  @param parity The parity scheme
 *  @param stop The number of stop bits
 *
 *  @warning 1.5 stop bits is only available in 5 bit mode
 *
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_open(ft900_uart_regs_t *dev, uint8_t prescaler, uint32_t divisor, uart_data_bits_t databits,
		uart_parity_t parity, uart_stop_bits_t stop);

/** @brief Calculate the prescaler and divisor from a baudrate
 *
 *  @param target_baud The baud rate to use
 *  @param samples The number of samples the UART will take for a bit, the default for this is 4
 *  @param f_perif Peripheral frequency, the default for this is 100,000,000
 *  @param divisor A pointer to store the divisor
 *  @param prescaler A pointer to store the prescaler, if this is NULL the prescaler will be set to 1
 *
 *  For Nsamples = 4 the following baud rates can be obtained from
 *  these Divisors and Prescalers:
 *
 *  | Desired baud | Prescaler | Divisor | Actual baud   | Error   |
 *  | :----------- | --------: | ------: | ------------: | ------: |
 *  | 110          | 4         |   56818 |     110.00035 | ~0.000% |
 *  | 150          | 3         |   55555 |     150.00150 | +0.001% |
 *  | 300          | 3         |   27778 |     299.99760 | ~0.000% |
 *  | 1200         | 1         |   20833 |    1200.01920 | +0.002% |
 *  | 2400         | 1         |   10417 |    2399.92320 | -0.003% |
 *  | 4800         | 1         |    5208 |    4800.30720 | +0.006% |
 *  | 9600         | 1         |    2604 |    9600.61440 | +0.006% |
 *  | 19200        | 1         |    1302 |    19201.2288 | +0.006% |
 *  | 31250        | 1         |     800 |   31250.00000 |  0.000% |
 *  | 38400        | 1         |     651 |   38402.45800 | +0.006% |
 *  | 57600        | 1         |     434 |   57603.68700 | +0.006% |
 *  | 115200       | 1         |     217 |  115207.37000 | +0.006% |
 *  | 230400       | 1         |     109 |  229357.80000 | -0.452% |
 *  | 460800       | 1         |      54 |  462962.96000 | -0.469% |
 *  | 921600       | 1         |      27 |  925925.93000 | -0.469% |
 *  | 1000000      | 1         |      25 | 1000000.00000 |  0.000% |
 *
 *  @returns The absolute error from the target baud rate
 */
/*  Baud rate is calculated by: 
 \f[ baud = f_{peripherals} / (N_{samples} \times Divisor \times Prescaler) \f] */
int32_t uart_calculate_baud(uint32_t target_baud, uint8_t samples, uint32_t f_perif, uint16_t *divisor, uint8_t *prescaler);

/** @brief Close a UART for communication
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_close(ft900_uart_regs_t *dev);

/** @brief Write a data word to a UART
 *
 *  @param dev The device to use
 *  @param buffer The data to send
 *
 *  @returns The number of bytes written or -1 otherwise (invalid device).
 */
size_t uart_write(ft900_uart_regs_t *dev, uint8_t buffer);

/** @brief Write a series of data words to a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the array to send
 *  @param len The size of buffer
 *
 *  @returns The number of bytes written or -1 otherwise (invalid device).
 */
size_t uart_writen(ft900_uart_regs_t *dev, uint8_t *buffer, size_t len);

/** @brief Read a data word from a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the data word to store into
 *
 *  @returns The number of bytes read or -1 otherwise (invalid device).
 */
size_t uart_read(ft900_uart_regs_t *dev, uint8_t *buffer);

/** @brief Read a series of data words from a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the array of data words to store into
 *  @param len The number of data words to read
 *
 *  @returns The number of bytes read or -1 otherwise (invalid device).
 */
size_t uart_readn(ft900_uart_regs_t *dev, uint8_t *buffer, size_t len);

/** @brief Write a string to the serial port
 *  @param dev The device to use
 *  @param str The null-terminated string to write
 *  @return The number of bytes written or -1 otherwise (invalid device).
 */
size_t uart_puts(ft900_uart_regs_t *dev, char* str);

/** @brief Enable an interrupt on the UART
 *  @param dev The device to use
 *  @param interrupt The interrupt to enable
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_enable_interrupt(ft900_uart_regs_t *dev, uart_interrupt_t interrupt);

/** @brief Disable an interrupt on the UART
 *  @param dev The device to use
 *  @param interrupt The interrupt to disable
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_disable_interrupt(ft900_uart_regs_t *dev, uart_interrupt_t interrupt);

/** @brief Enable a UART to interrupt
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_enable_interrupts_globally(ft900_uart_regs_t *dev);

/** @brief Disable a UART from interrupting
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_disable_interrupts_globally(ft900_uart_regs_t *dev);

/** @brief Return the currently indicated interrupt.
 *  @param dev The device to use
 *  @returns enum of possible interrupt levels. Cast to
 *  	values defined in enum uart_interrupt_t.
 *  	0xff if invalid device.
 */
uint8_t uart_get_interrupt(ft900_uart_regs_t *dev);

/** @brief Check if an interrupt has been triggered
 *  @param dev The device to use
 *  @param interrupt The interrupt to check
 *  @warning This function clears the current interrupt status bit
 *  @returns 1 when interrupted, 0 when not interrupted, -1 otherwise
 *   (invalid device).
 */
int8_t uart_is_interrupted(ft900_uart_regs_t *dev, uart_interrupt_t interrupt);

/** @brief Enable or disable RTS signal
 *  @param dev The device to use
 *  @param active Non-zero to enable RTS (line high) zero to disable (line low),
 *   -1 otherwise (invalid device).
 */
int8_t uart_rts(ft900_uart_regs_t *dev, int active);

/** @brief Enable or disable DTR signal
 *  @param dev The device to use
 *  @param active Non-zero to enable DTR (line high) zero to disable (line low)
 *   -1 otherwise (invalid device).
 */
int8_t uart_dtr(ft900_uart_regs_t *dev, int active);

/** @brief Test status of CTS signal
 *  @param dev The device to use
 *  @returns 1 when CTS enabled, 0 when not enabled,
 *   -1 otherwise (invalid device).
 */
int8_t uart_cts(ft900_uart_regs_t *dev);

/** @brief Test status of DSR signal
 *  @param dev The device to use
 *  @returns 1 when DSR enabled, 0 when not enabled,
 *   -1 otherwise (invalid device).
 */
int8_t uart_dsr(ft900_uart_regs_t *dev);

/** @brief Test status of RI signal
 *  @param dev The device to use
 *  @returns 1 when RI enabled, 0 when not enabled,
 *   -1 otherwise (invalid device).
 */
int8_t uart_ri(ft900_uart_regs_t *dev);

/** @brief Test status of DCD signal
 *  @param dev The device to use
 *  @returns 1 when DCD enabled, 0 when not enabled,
 *   -1 otherwise (invalid device).
 */
int8_t uart_dcd(ft900_uart_regs_t *dev);

/** @brief Set the mode of the UART
 *  @details After the mode is selected all flow control and UART
 *  settings are reset. The uart_open function must be called
 *  again to re-initialise the UART.
 *  @param dev The device to use
 *  @param mode The mode to select.
 *  @returns 0 if successful, -1 otherwise (invalid device).
 */
int8_t uart_mode(ft900_uart_regs_t *dev, uart_mode_t mode);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* FT900_UART_SIMPLE_H_ */
