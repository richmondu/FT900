/**
  @file uart_registers.h
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
#ifndef FT900_UART_REGSITERS_H_
#define FT900_UART_REGSITERS_H_


/* INCLUDES ************************************************************************/
#include <stdint.h>

/* CONSTANTS ***********************************************************************/
#define BIT_UART_LCR_WLS                (0)
#define MASK_UART_LCR_WLS               (0x3 << BIT_UART_LCR_WLS)
#define BIT_UART_LCR_STB                (2)
#define MASK_UART_LCR_STB               (1 << BIT_UART_LCR_STB)
#define BIT_UART_LCR_PEN                (3)
#define MASK_UART_LCR_PEN               (1 << BIT_UART_LCR_PEN)
#define BIT_UART_LCR_EPS                (4)
#define MASK_UART_LCR_EPS               (1 << BIT_UART_LCR_EPS)
#define BIT_UART_LCR_STKPAR             (5)
#define MASK_UART_LCR_STKPAR            (1 << BIT_UART_LCR_STKPAR)
#define BIT_UART_LCR_BRK                (6)
#define MASK_UART_LCR_BRK               (1 << BIT_UART_LCR_BRK)
#define BIT_UART_LCR_DLAB               (7)
#define MASK_UART_LCR_DLAB              (1 << BIT_UART_LCR_DLAB)

#define BIT_UART_LSR_DR                 (0)
#define MASK_UART_LSR_DR                (1 << BIT_UART_LSR_DR)
#define BIT_UART_LSR_OE                 (1)
#define MASK_UART_LSR_OE                (1 << BIT_UART_LSR_OE)
#define BIT_UART_LSR_PE_RXDATA          (2)
#define MASK_UART_LSR_PE_RXDATA         (1 << BIT_UART_LSR_PE_RXDATA)
#define BIT_UART_LSR_FE                 (3)
#define MASK_UART_LSR_FE                (1 << BIT_UART_LSR_FE)
#define BIT_UART_LSR_BI                 (4)
#define MASK_UART_LSR_BI                (1 << BIT_UART_LSR_BI)
#define BIT_UART_LSR_THRE               (5)
#define MASK_UART_LSR_THRE              (1 << BIT_UART_LSR_THRE)
#define BIT_UART_LSR_TEMT               (6)
#define MASK_UART_LSR_TEMT              (1 << BIT_UART_LSR_TEMT)
#define BIT_UART_LSR_RHRERR             (7)
#define MASK_UART_LSR_RHRERR            (1 << BIT_UART_LSR_RHRERR)

#define BIT_UART_IER_ERBFI              (0)
#define MASK_UART_IER_ERBFI             (1 << BIT_UART_IER_ERBFI)
#define BIT_UART_IER_ETBEI              (1)
#define MASK_UART_IER_ETBEI             (1 << BIT_UART_IER_ETBEI)
#define BIT_UART_IER_ELSI               (2)
#define MASK_UART_IER_ELSI              (1 << BIT_UART_IER_ELSI)
#define BIT_UART_IER_EDSSI              (3)
#define MASK_UART_IER_EDSSI             (1 << BIT_UART_IER_EDSSI)
#define BIT_UART_IER_ESM                (4)
#define MASK_UART_IER_ESM               (1 << BIT_UART_IER_ESM)
#define BIT_UART_IER_ESCH               (5)
#define MASK_UART_IER_ESCH              (1 << BIT_UART_IER_ESCH)
#define BIT_UART_IER_RTSIMASK           (6)
#define MASK_UART_IER_RTSIMASK          (1 << BIT_UART_IER_RTSIMASK)
#define BIT_UART_IER_CTSIMASK           (7)
#define MASK_UART_IER_CTSIMASK          (1 << BIT_UART_IER_CTSIMASK)

#define BIT_UART_MCR_DTR                (0)
#define MASK_UART_MCR_DTR               (1 << BIT_UART_MCR_DTR)
#define BIT_UART_MCR_RTS                (1)
#define MASK_UART_MCR_RTS               (1 << BIT_UART_MCR_RTS)
#define BIT_UART_MCR_OUT1               (2)
#define MASK_UART_MCR_OUT1              (1 << BIT_UART_MCR_OUT1)
#define BIT_UART_MCR_OUT2               (3)
#define MASK_UART_MCR_OUT2              (1 << BIT_UART_MCR_OUT2)
#define BIT_UART_MCR_LOOP               (4)
#define MASK_UART_MCR_LOOP              (1 << BIT_UART_MCR_LOOP)
#define BIT_UART_MCR_XON                (5)
#define MASK_UART_MCR_XON               (1 << BIT_UART_MCR_XON)
#define BIT_UART_MCR_IRDA               (6)
#define MASK_UART_MCR_IRDA              (1 << BIT_UART_MCR_IRDA)
#define BIT_UART_MCR_PRESCALAR          (7)
#define MASK_UART_MCR_PRESCALAR         (1 << BIT_UART_MCR_PRESCALAR)

#define BIT_UART_MSR_DCTS               (0)
#define MASK_UART_MSR_DCTS              (1 << BIT_UART_MSR_DCTS)
#define BIT_UART_MSR_DDSR               (1)
#define MASK_UART_MSR_DDSR              (1 << BIT_UART_MSR_DDSR)
#define BIT_UART_MSR_TERI               (2)
#define MASK_UART_MSR_TERI              (1 << BIT_UART_MSR_TERI)
#define BIT_UART_MSR_DDCD               (3)
#define MASK_UART_MSR_DDCD              (1 << BIT_UART_MSR_DDCD)
#define BIT_UART_MSR_CTS                (4)
#define MASK_UART_MSR_CTS               (1 << BIT_UART_MSR_CTS)
#define BIT_UART_MSR_DSR                (5)
#define MASK_UART_MSR_DSR               (1 << BIT_UART_MSR_DSR)
#define BIT_UART_MSR_RI                 (6)
#define MASK_UART_MSR_RI                (1 << BIT_UART_MSR_RI)
#define BIT_UART_MSR_DCD                (7)
#define MASK_UART_MSR_DCD               (1 << BIT_UART_MSR_DCD)

/* SPECIAL REGISTERS */

#define OFFSET_UART_SPR_ACR             (0U)

#define OFFSET_UART_SPR_CPR             (1U)

#define OFFSET_UART_SPR_TCR             (2U)

#define OFFSET_UART_SPR_CKS             (3U)

#define OFFSET_UART_SPR_TTL             (4U)

#define OFFSET_UART_SPR_RTL             (5U)

#define OFFSET_UART_SPR_FCL             (6U)

#define OFFSET_UART_SPR_FCH             (7U)

#define OFFSET_UART_SPR_ID1             (8U)

#define OFFSET_UART_SPR_ID2             (9U)

#define OFFSET_UART_SPR_ID3             (10U)

#define OFFSET_UART_SPR_REV             (11U)

#define OFFSET_UART_SPR_CSR             (12U)

#define OFFSET_UART_SPR_NMR             (13U)
#define BIT_UART_SPR_NMR_9EN            (0)
#define MASK_UART_SPR_NMR_9EN           (1 << BIT_UART_SPR_NMR_9EN)

#define OFFSET_UART_SPR_MDM             (14U)

#define OFFSET_UART_SPR_RFC             (15U)

#define OFFSET_UART_SPR_GDS             (16U)

/* TYPES ***************************************************************************/
/** @brief Register mappings for UART registers */
typedef struct
{
    volatile uint8_t RHR_THR_DLL;
    volatile uint8_t IER_DLH_ASR;
    volatile uint8_t ISR_FCR_EFR;
    volatile uint8_t LCR_RFL;
    volatile uint8_t MCR_XON1_TFL;
    volatile uint8_t LSR_ICR_XON2;
    volatile uint8_t MSR_XOFF1;
    volatile uint8_t SPR_XOFF2;
} ft900_uart_regs_t;

/* GLOBAL VARIABLES ****************************************************************/

/* MACROS **************************************************************************/

/* FUNCTION PROTOTYPES *************************************************************/

#endif /* FT900_UART_REGSITERS_H_ */
#endif
