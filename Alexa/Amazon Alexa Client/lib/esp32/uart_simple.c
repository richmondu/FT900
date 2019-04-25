/**
    @file

    @brief
    Simple UART

    
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

/* INCLUDES ************************************************************************/
#include <string.h>
#include "ft900_uart_simple.h"
#include <ft900_asm.h>
#include "ft900_uart_registers.h"
#include <registers/ft900_registers.h>
#include <private/ft900_internals.h>


/* CONSTANTS ***********************************************************************/
#define PRESCALER_MAX       (31)
#define DIVISOR_MAX         (65535)

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/* MACROS **************************************************************************/

#define uart_wait_whilst_transmitting(pUart) \
    while (!(pUart->LSR_ICR_XON2 & MASK_UART_LSR_THRE))
#define uart_wait_whilst_receiving(pUart) \
    while (!(pUart->LSR_ICR_XON2 & MASK_UART_LSR_DR))

/* LOCAL FUNCTIONS / INLINES *******************************************************/
static inline void uart_spr_write(ft900_uart_regs_t *uart, uint8_t location, uint8_t val)
{
  /* A memory barrier is necessary to stop the compiler from optimizing away multiple writes to the 
  LSR_ICR_XON2 when this function is inlined */
  uart->SPR_XOFF2 = location;
  uart->LSR_ICR_XON2 = val;
  __asm__("" ::: "memory");
}

/* FUNCTIONS ***********************************************************************/

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
 *  @returns 0 on success, -1 otherwise (invalid device).
 */
int8_t uart_open(ft900_uart_regs_t *dev, uint8_t prescaler, uint32_t divisor, uart_data_bits_t databits,
        uart_parity_t parity, uart_stop_bits_t stop)
{
    int8_t iRet = 0;
    uint8_t LCR_RFL;
    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        uart_spr_write(dev, OFFSET_UART_SPR_ACR, 0x00);

        /* Write the prescaler... */
        uart_spr_write(dev, OFFSET_UART_SPR_CPR, prescaler << 3);
        /* Write the clock samples... */
        uart_spr_write(dev, OFFSET_UART_SPR_TCR, 4);

        /* Write the divisor... */
        dev->LCR_RFL |= MASK_UART_LCR_DLAB; /* Enable the divisor latch */
        dev->RHR_THR_DLL = (divisor >> 0) & 0xFF;
        dev->IER_DLH_ASR = (divisor >> 8) & 0xFF;
        dev->LCR_RFL &= ~MASK_UART_LCR_DLAB; /* Disable the divisor latch */

        LCR_RFL = 0x00;
        /* Set up data bits... */
        switch (databits)
        {
            case uart_data_bits_5:
            case uart_data_bits_6:
            case uart_data_bits_7:
            case uart_data_bits_8:
                LCR_RFL = (databits - uart_data_bits_5) << BIT_UART_LCR_WLS;
            break;

            default: break;
        }


        /* Set up stop bits... */
        switch(stop)
        {
            case uart_stop_bits_1:
                LCR_RFL &= ~MASK_UART_LCR_STB;
            break;

            case uart_stop_bits_1_5:
                if (databits == uart_data_bits_5)
                {
                    LCR_RFL |= MASK_UART_LCR_STB;
                }
                else
                {
                    iRet = -1;
                }
            break;

            case uart_stop_bits_2:
                if (databits != uart_data_bits_5)
                {
                    LCR_RFL |= MASK_UART_LCR_STB;
                }
                else
                {
                    iRet = -1;
                }
            break;

            default: break;
        }


        /* Set up parity... */
        switch(parity)
        {
            case uart_parity_none:
                LCR_RFL &= ~MASK_UART_LCR_PEN; /* Disable parity */
            break;

            case uart_parity_odd:
                LCR_RFL |= MASK_UART_LCR_PEN; /* Enable parity */
                LCR_RFL &= ~MASK_UART_LCR_EPS;
            break;

            case uart_parity_even:
                LCR_RFL |= MASK_UART_LCR_PEN | MASK_UART_LCR_EPS; /* Enable parity */
            break;

            default: break;
        }
        dev->LCR_RFL = LCR_RFL;
        dev->ISR_FCR_EFR = 0x00; /* FIFOs off 16450 mode. */
        dev->MCR_XON1_TFL = 0x02; /* RTS High */

        /* Enable transmit and receive */

        /* Check to see if there is any garbage left over */
        if (dev->LSR_ICR_XON2 & MASK_UART_LSR_DR)
        {
            FT900_ATTR_UNUSED char c;
            c = dev->RHR_THR_DLL;
        }
    }

    return iRet;
}


/** @brief Calculate the prescaler and divisor from a baudrate
 *
 *  @param target_baud The baud rate to use
 *  @param samples The number of samples the UART will take for a bit, the default for this is 4
 *  @param f_perif Peripheral frequency, the default for this is 100,000,000
 *  @param divisor A pointer to store the divisor
 *  @param prescaler A pointer to store the prescaler, if this is NULL the prescaler will be set to 1
 *
 *  Baud rate is calculated by:
 *  \f[ baud = f_{peripherals} / (N_{samples} \times Divisor \times Prescaler) \f]
 *
 *  For \f$ N_{samples} = 4 \f$ the following baud rates can be obtained from
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
int32_t uart_calculate_baud(uint32_t target_baud, uint8_t samples, uint32_t f_perif, uint16_t *divisor, uint8_t *prescaler)
{
    uint32_t lBaud = 0;
    uint16_t wDivisor = 0;
    uint8_t bPrescaler = 1, bPrescalerLoopEnd = PRESCALER_MAX+1;
    uint16_t wHopSize = 0;

    uint8_t bPrescalerBestMatch = 1;
    uint16_t wDivisorBestMatch = 65535;
    int32_t lBaudErrorBestMatch = target_baud;

    /* Only work on a prescaler of 1 if the user doesn't ask for a prescaler back */
    if (prescaler == NULL) bPrescalerLoopEnd = 2;

    /* Loop through all the possible current permutations to calculate the baud rate */
    /* Exit after all calculations or if we find an exact match */

    for (bPrescaler = 1; (bPrescaler < bPrescalerLoopEnd) && (lBaudErrorBestMatch != 0UL); ++bPrescaler)
    {
        /* Binary search through the available range, hone in on the right divisor... */
        wDivisor = wHopSize = (DIVISOR_MAX / 2) + 1; /* log2(2^16) hops = 16 hops */

        while((wHopSize > 0) && (lBaudErrorBestMatch != 0UL))
        {
            lBaud = f_perif / (samples * wDivisor * bPrescaler);

            /* Check to see if we got any closer to the target */
            if (labs(lBaud - target_baud) < labs(lBaudErrorBestMatch))
            {
                /* We found a better match... */
                lBaudErrorBestMatch = lBaud - target_baud;
                wDivisorBestMatch = wDivisor;
                bPrescalerBestMatch = bPrescaler;
            }

            /* Half our jump size */
            wHopSize = wHopSize >> 1;

            if (lBaud < target_baud)
            {
                /* Too slow, lower divisor */
                wDivisor = wDivisor - wHopSize;
            }
            else
            {
                /* Too fast, higher divisor */
                wDivisor = wDivisor + wHopSize;
            }
        }
    }


    /* Copy out the calculated settings */
    *divisor = wDivisorBestMatch;
    *prescaler = bPrescalerBestMatch;

    /* Return what our error was */
    return lBaudErrorBestMatch;
}


/** @brief Close a UART for communication
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_close(ft900_uart_regs_t *dev)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        /* Disable the clock to the baudrate generator */
        dev->LCR_RFL |= MASK_UART_LCR_DLAB; /* Enable the divisor latch */
        dev->RHR_THR_DLL = 0;
        dev->IER_DLH_ASR = 0;
        dev->LCR_RFL &= ~MASK_UART_LCR_DLAB; /* Disable the divisor latch */
    }

    return iRet;
}


/** @brief Write a data word to a UART
 *
 *  @param dev The device to use
 *  @param buffer The data to send
 *
 *  @returns The number of bytes written or -1 otherwise
 */
size_t uart_write(ft900_uart_regs_t *dev, uint8_t buffer)
{
        size_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        uart_wait_whilst_transmitting(dev);
        dev->RHR_THR_DLL = buffer;
        iRet = 1;
    }

    return iRet;
}


/** @brief Write a series of data words to a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the array to send
 *  @param len The size of buffer
 *
 *  @returns The number of bytes written or -1 otherwise
 */
size_t uart_writen(ft900_uart_regs_t *dev, uint8_t *buffer, size_t len)
{
        size_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        while (len)
        {
            uart_wait_whilst_transmitting(dev);
            dev->RHR_THR_DLL = *buffer;
            buffer++;
            iRet++;
            len--;
        }
    }

    return iRet;
}


/** @brief Read a data word from a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the data word to store into
 *
 *  @returns The number of bytes read or -1 otherwise
 */
size_t uart_read(ft900_uart_regs_t *dev, uint8_t *buffer)
{
        size_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        uart_wait_whilst_receiving(dev);
        *buffer = dev->RHR_THR_DLL;
        iRet = 1;
    }

    return iRet;
}


/** @brief Read a series of data words from a UART
 *
 *  @param dev The device to use
 *  @param buffer A pointer to the array of data words to store into
 *  @param len The number of data words to read
 *
 *  @returns The number of bytes read or -1 otherwise
 */
size_t uart_readn(ft900_uart_regs_t *dev, uint8_t *buffer, size_t len)
{
        size_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        while (len)
        {
            uart_wait_whilst_receiving(dev);
            *buffer = dev->RHR_THR_DLL;
            buffer++;
            iRet++;
            len--;
        }
    }

    return iRet;
}


/** @brief Write a string to the serial port
 *  @param dev The device to use
 *  @param str The null-terminated string to write
 *  @return The number of bytes written or -1 otherwise
 */
size_t uart_puts(ft900_uart_regs_t *dev, char *str)
{
    return uart_writen(dev, (uint8_t *)str, strlen(str));
}


/** @brief Enable an interrupt on the UART
 *  @param dev The device to use
 *  @param interrupt The interrupt to enable
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_enable_interrupt(ft900_uart_regs_t *dev, uart_interrupt_t interrupt)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        switch(interrupt)
        {
            case uart_interrupt_tx: dev->IER_DLH_ASR |= MASK_UART_IER_ETBEI; break;
            case uart_interrupt_rx: dev->IER_DLH_ASR |= MASK_UART_IER_ERBFI; break;
            case uart_interrupt_dcd_ri_dsr_cts: dev->IER_DLH_ASR |= MASK_UART_IER_EDSSI; break;
            default: iRet = -1; break;
        }
    }

    return iRet;
}


/** @brief Disable an interrupt on the UART
 *  @param dev The device to use
 *  @param interrupt The interrupt to disable
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_disable_interrupt(ft900_uart_regs_t *dev, uart_interrupt_t interrupt)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        switch(interrupt)
        {
            case uart_interrupt_tx: dev->IER_DLH_ASR &= ~MASK_UART_IER_ETBEI; break;
            case uart_interrupt_rx: dev->IER_DLH_ASR &= ~MASK_UART_IER_ERBFI; break;
            case uart_interrupt_dcd_ri_dsr_cts: dev->IER_DLH_ASR &= ~MASK_UART_IER_EDSSI; break;
            default: iRet = -1; break;
        }
    }

    return iRet;
}


/** @brief Enable a UART to interrupt
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_enable_interrupts_globally(ft900_uart_regs_t *dev)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        if (dev == UART0)
        {
            SYS->MSC0CFG |= MASK_SYS_MSC0CFG_UART1_INTSEL;
        }
        else if (dev == UART1)
        {
            SYS->MSC0CFG |= MASK_SYS_MSC0CFG_UART2_INTSEL;
        }
#if defined(__FT930__)
        else if (dev == UART2)
        {
            SYS->MSC0CFG |= MASK_SYS_MSC0CFG_UART3_INTSEL;
        }
        else if (dev == UART3)
        {
            SYS->MSC0CFG |= MASK_SYS_MSC0CFG_UART4_INTSEL;
        }
#endif
        else
        {
            iRet = -1;
        }
    }

    return 0;
}


/** @brief Disable a UART from interrupting
 *  @param dev The device to use
 *  @returns 0 on success, -1 otherwise
 */
int8_t uart_disable_interrupts_globally(ft900_uart_regs_t *dev)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        if (dev == UART0)
        {
            SYS->MSC0CFG &= ~MASK_SYS_MSC0CFG_UART1_INTSEL;
        }
        else if (dev == UART1)
        {
            SYS->MSC0CFG &= ~MASK_SYS_MSC0CFG_UART2_INTSEL;
        }
#if defined(__FT930__)
        else if (dev == UART2)
        {
            SYS->MSC0CFG &= ~MASK_SYS_MSC0CFG_UART3_INTSEL;
        }
        else if (dev == UART3)
        {
            SYS->MSC0CFG &= ~MASK_SYS_MSC0CFG_UART4_INTSEL;
        }
#endif
        else
        {
            iRet = -1;
        }
    }

    return 0;
}

/** @brief Return the currently indicated interrupt.
 *  @param dev The device to use
 *  @returns enum of possible interrupt levels. Cast to
 *  	values defined in enum uart_interrupt_t.
 */
uint8_t uart_get_interrupt(ft900_uart_regs_t *dev)
{
    /* UART ISR is auto clearing */
    uint8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = 0xff;
    }

    if (iRet == 0)
    {
    	iRet = dev->ISR_FCR_EFR & 0x3F;
    }

    return iRet;
}

/** @brief Check if an interrupt has been triggered
 *  @param dev The device to use
 *  @param interrupt The interrupt to check
 *  @warning This function clears the current interrupt status bit
 *  @returns 1 when interrupted, 0 when not interrupted, -1 otherwise
 */
int8_t uart_is_interrupted(ft900_uart_regs_t *dev, uart_interrupt_t interrupt)
{
    /* UART ISR is auto clearing */
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
        switch(interrupt)
        {
            case uart_interrupt_tx:
                if ((dev->ISR_FCR_EFR & 0x3F) == 2)
                {
                    iRet = 1;
                }
            break;

            case uart_interrupt_rx:
                if ((dev->ISR_FCR_EFR & 0x3F) == 4)
                {
                    iRet = 1;
                }
            break;

            case uart_interrupt_dcd_ri_dsr_cts:
                if ((dev->ISR_FCR_EFR & 0x3F) == 0)
                {
                    iRet = 1;
                }
            break;

            default: iRet = -1; break;
        }
    }

    return iRet;
}

int8_t uart_rts(ft900_uart_regs_t *dev, int active)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MCR_XON1_TFL;
		if (active) mcr |= MASK_UART_MCR_RTS;
		else mcr &= (~MASK_UART_MCR_RTS);
		dev->MCR_XON1_TFL = mcr;
    }

    return iRet;
}

int8_t uart_dtr(ft900_uart_regs_t *dev, int active)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MCR_XON1_TFL;
		if (active) mcr |= MASK_UART_MCR_DTR;
		else mcr &= (~MASK_UART_MCR_DTR);
		dev->MCR_XON1_TFL = mcr;
    }

	return iRet;
}

int8_t uart_cts(ft900_uart_regs_t *dev)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MSR_XOFF1;
		if (mcr & MASK_UART_MSR_CTS) return 1;
		return 0;
    }

    return iRet;
}

int8_t uart_dsr(ft900_uart_regs_t *dev)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MSR_XOFF1;
		if (mcr & MASK_UART_MSR_DSR) return 1;
		return 0;
    }

	return iRet;
}

int8_t uart_ri(ft900_uart_regs_t *dev)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MSR_XOFF1;
		if (mcr & MASK_UART_MSR_RI) return 1;
		return 0;
    }

	return iRet;
}

int8_t uart_dcd(ft900_uart_regs_t *dev)
{
	uint8_t mcr;
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
		mcr = dev->MSR_XOFF1;
		if (mcr & MASK_UART_MSR_DCD) return 1;
		return 0;
    }

	return iRet;
}

int8_t uart_mode(ft900_uart_regs_t *dev, uart_mode_t mode)
{
    int8_t iRet = 0;

    if (dev == NULL)
    {
        /* Unknown device */
        iRet = -1;
    }

    if (iRet == 0)
    {
    	switch (mode)
    	{
    	case uart_mode_16450:
            dev->ISR_FCR_EFR = 0x00; /* FIFOs off 16450 mode. */
            break;
    	case uart_mode_16550:
            dev->ISR_FCR_EFR = 0x01; /* 16 byte FIFOs 16550 mode. */
            break;
    	case uart_mode_16650:
    	case uart_mode_16950:
    		dev->LCR_RFL = 0xbf;
            dev->ISR_FCR_EFR = 0x10; /* Set EFR[4] */
    		dev->LCR_RFL = 0x00;
            dev->ISR_FCR_EFR = 0x01; /* 128 byte FIFOs 16650/16960 mode. */
            break;
    	case uart_mode_16750:
            dev->ISR_FCR_EFR = 0x81; /* FCR[5] protected by FCR[7] */
            dev->ISR_FCR_EFR = 0xa1;
            dev->ISR_FCR_EFR = 0x21; /* 128 byte FIFOs 16750 mode. */
            break;
    	}
    }

	return iRet;
}
