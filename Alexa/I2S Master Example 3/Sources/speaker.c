/**
  @file speaker.c
  @brief
  Speaker module

 */
/*
 * ============================================================================
 * History
 * =======
 * 2019-02-14 : Created v1
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

#include <stdint.h>
#include <ft900.h>
#include "tinyprintf.h"
#include "speaker.h"



#define WM8731_WR_ADDR (0x34)
#define WM8731_RD_ADDR (0x35)
#define BITS 16

typedef struct
{
    uint8_t addr;
    uint8_t data;
} reg_t;


static reg_t i2c_data[] =
{
    /* 0xF Reset -
       Writing 0x00 to register resets device
    */
    {0x0F << 1, 0x00},


    /* 0x0 Left Line In -
       bit 4:0 = 10111 : LINVOL[4:0] - 10111 = Input Volume to 0dB
       bit 7   = 0     : LINMUTE     - 0 = Disable Mute
    */
    {0x00 << 1, 0x79},

    /* 0x0 Right Line In -
       bit 4:0 = 10111 : LINVOL[4:0] - 10111 = Input Volume to 0dB
       bit 7   = 0     : LINMUTE     - 0 = Disable Mute
    */
    {0x01 << 1, 0x79},


    /* 0x2 Left Headphone Out -
       bit 6:0 = 1111001 : LHPVOL[6:0] - 1111001 = Output Volume to 0dB
       bit 7   = 1       : LZCEN       - Left Channel Zero Cross detect 1 = Enable
    */
    {0x02 << 1, 0x79},

    /* 0x2 Right Headphone Out -
       bit 6:0 = 1111001 : LHPVOL[6:0] - 1111001 = Output Volume to 0dB
       bit 7   = 1       : RZCEN       - Right Channel Zero Cross detect 1 = Enable
    */
    {0x03 << 1, 0x79},


    /* 0x4 Analogue Audio Path Control -
       bit 0 = 1 : MICBOOST     - 1 = Enable Boost
       bit 1 = 1 : MUTEMIC      - 1 = Enable Mute
       bit 2 = 0 : INSEL        - 0 = Microphone Input Select to ADC
       bit 3 = 0 : BYPASS       - 0 = Disable Bypass
       bit 4 = 1 : DACSEL       - 1 = Select DAC
       bit 5 = 0 : SIDETONE     - 0 = Disable Side Tone Attenuation
       bit 6 = 0 : SIDEATT[1:0] - Ignored
    */
    {0x04 << 1, 0x15},

    /* 0x5 Digital Audio Path Control -
       bit 0   = 0  : ADCHPD     - 0 = Enable High Pass Filter
       bit 2:1 = 00 : DEEMP[1:0] - De-emphasis Control 00 = Disable
       bit 3   = 0  : DACMU      - 0 = Disable Soft Mute
       bit 4   = 0  : HPOR       - 0 = Clear Offset
    */
    {0x05 << 1, 0x06},

    /* 0x6 Power Down Control -
       bit 0 = 1 : LINEINPD - 1 = Enable Power Down
       bit 1 = 1 : MICPD    - 1 = Enable Power Down
       bit 2 = 0 : ADCPD    - 0 = Disable Power Down
       bit 3 = 0 : DACPD    - 0 = Disable Power Down
       bit 4 = 0 : OUTPD    - 0 = Disable Power Down
       bit 5 = 0 : OSCPD    - 0 = Disable Power Down
       bit 6 = 1 : CLKOUTPD - 1 = Enable Power Down
       bit 7 = 0 : POWEROFF - 0 = Disable POWEROFF
    */
    {0x06 << 1, 0x00},

    /* 0x7 Digital Audio Interface Format -
       bit 1:0 = 10 : FORMAT[1:0] - 10 = I2S
       bit 3:2 = 00 : IWL[1:0]    - 00 = 16bit
       bit 4   = 0  : LRP         - 0 = Right Ch when DACLRC Low
       bit 5   = 0  : LRSWAP      - 0 = Disable DAC Left Right Clock Swap
       bit 6   = 0  : MS          - 0 = Enable Slave Mode
       bit 7   = 0  : BCLKINV     - 0 = Don't Invert BCLK
    */
    {0x07 << 1, 0x02}, // 16 bit

    /* 0x8 Sampling Control -
       bit 0   = 0    : USB/NORMAL - 0 = Normal Mode  (256/384fs)
       bit 1   = 0    : BOSR       - 0 = 250 fs (for 48000 Hz sampling)
                                   - 1 = 272 fs (for 44100 Hz sampling)
       bit 5:2 = 1000 : SR[3:0]    - 0000 = 48 kHz sample rate
                                   - 1000 = 44.1 kHz sample rate
                                   - 0110 = 32 kHz sample rate
                                   - 0011 = 8 kHz sample rate
       bit 6   = 0    : CLKIDIV2   - 0 = Not applicable
       bit 7   = 0    : CLKODIV2   - 0 = CLOCKOUT is Core
    */
    {0x08 << 1, 0x00},

    /* 0x9 Active Control -
       bit 0 = 1 : ACTIVE - 1 = Active
    */
    {0x09 << 1, 0x01}
};

static const char* I2C_statusbits[] = {
    "I2C_BUSY (The Bus is currently busy transmitting/receiving)",
    "I2C_ERR (An error occurred (ADDR_ACK, DATA_ACK, ARB_LOST))",
    "ADDR_ACK (Slave address was not acknowledged)",
    "DATA_ACK (Data was not acknowledged)",
    "ARB_LOST (Arbitration lost)",
    "I2C_IDLE (The I2C Controller is idle)",
    "BUS_BUSY (The I2C Bus is busy)",
    "-"
};


static void speaker_config(int samplingRate)
{
    /* Set up the WM8731 ... */
    for (int i = 0; i < (sizeof(i2c_data)/sizeof(reg_t)); ++i) {
        // update based on samplingRate
        if (i2c_data[i].addr == 0x08 << 1) {
            switch (samplingRate) {
                case SAMPLING_RATE_44100HZ:
                    i2c_data[i].data = 0x22;
                    break;
                case SAMPLING_RATE_48KHZ:
                    i2c_data[i].data = 0x00;
                    break;
                case SAMPLING_RATE_32KHZ:
                    i2c_data[i].data = 0x18;
                    break;
                case SAMPLING_RATE_8KHZ:
                    i2c_data[i].data = 0x08;
                    break;
                default:
                    // use existing value
                    break;
            }
        }

        int8_t retval = 0;
        //tfp_printf("I2C Write: addr 0x%02x, reg 0x%02x, data 0x%02x\r\n",
        //        WM8731_WR_ADDR, i2c_data[i].addr, i2c_data[i].data);

        retval = i2cm_write(WM8731_WR_ADDR, i2c_data[i].addr, &(i2c_data[i].data), 1);
        if (retval) {
            uint8_t status = i2cm_get_status();
            int j;

            //tfp_printf("i2cm_write() returned %d\r\n", retval);
            for(j=0; j<8; ++j) {
                if (status & (1<<j)) {
                    tfp_printf("    %s\r\n",I2C_statusbits[j]);
                }
            }
        }
    }
    delayms(100);
}

static void speaker_initi2s(int samplingRate)
{
    /* Refer to Table 15.1 and 15.2 for FT900 User Manual  */
    // Only the 44100Hz sounds acceptable
    if (samplingRate == SAMPLING_RATE_44100HZ) {
        i2s_init(i2s_mode_master,
                 i2s_length_16,
                 i2s_format_i2s,
                 i2s_padding_0,
                 i2s_master_input_clk_22mhz,
                 i2s_bclk_div_16,
                 i2s_mclk_div_2,
                 i2s_bclk_per_channel_16
                 );
    }
    else if (samplingRate == SAMPLING_RATE_48KHZ) {
        i2s_init(i2s_mode_master,
                 i2s_length_16,
                 i2s_format_i2s,
                 i2s_padding_0,
                 i2s_master_input_clk_24mhz,
                 i2s_bclk_div_16,
                 i2s_mclk_div_2,
                 i2s_bclk_per_channel_16
                 );
    }
    else if (samplingRate == SAMPLING_RATE_32KHZ) {
        i2s_init(i2s_mode_master,
                 i2s_length_16,
                 i2s_format_i2s,
                 i2s_padding_0,
                 i2s_master_input_clk_24mhz,
                 i2s_bclk_div_24,
                 i2s_mclk_div_3,
                 i2s_bclk_per_channel_16
                 );
    }
    else if (samplingRate == SAMPLING_RATE_8KHZ) {
        i2s_init(i2s_mode_master,
                 i2s_length_16,
                 i2s_format_i2s,
                 i2s_padding_0,
                 i2s_master_input_clk_24mhz,
                 i2s_bclk_div_48,
                 i2s_mclk_div_12,
                 i2s_bclk_per_channel_32
                 );
    }
}

void speaker_setup(void (*speaker_isr)(void), int samplingRate)
{
    /* EVM: Bring the output amplifiers out of Power Down ... */
    gpio_dir(65, pad_dir_output);
    gpio_write(65, 1);

    /* Set up the I2S peripheral, make sure GPIO 62 and 63 are set up for I2S Slave ... */
    sys_enable(sys_device_i2s);
    gpio_function(60, pad_i2s_sdao); gpio_pull(60, pad_pull_none); /* I2S SDAO */
    gpio_function(61, pad_i2s_sdai); gpio_pull(61, pad_pull_none); /* I2S SDAI */
    gpio_function(62, pad_i2s_bclk); gpio_pull(62, pad_pull_none); /* I2S Slave BCLK */
    gpio_function(63, pad_i2s_lrclk); gpio_pull(63, pad_pull_none); /* I2S Slave LRCLK */
    gpio_function(64, pad_i2s_mclk); gpio_pull(64, pad_pull_none); /* I2S MCLK */
    gpio_function(65, pad_i2s_clk22); gpio_pull(65, pad_pull_none); /* I2S CLK22 */
    gpio_function(66, pad_i2s_clk24); gpio_pull(66, pad_pull_none); /* I2S CLK24 */

    /* Initialize I2S master  */
    speaker_initi2s(samplingRate);

    /* Set up the I2C peripheral ... */
    sys_enable(sys_device_i2c_master);
    sys_i2c_swop(1); /* Swap master and slave pins, so I2CM uses GPIO 46, 47 */
    /* Note that even when swapping is enabled, gpio_function() should be used with the original pins */
    gpio_function(44, pad_i2c0_scl); /* I2C0 SCL */
    gpio_function(45, pad_i2c0_sda); /* I2C0 SDA */
    /* But gpio_pull() should be used with the new pins */
    gpio_pull(46, pad_pull_pullup);
    gpio_pull(47, pad_pull_pullup);
    i2cm_init(I2CM_NORMAL_SPEED, 100000);

    /* Set up the WM8731 ... */
    speaker_config(samplingRate);

    if (speaker_isr) {
        /* Set up the ISR for the I2S device... */
        i2s_clear_int_flag(0xFFFF);
        interrupt_attach(interrupt_i2s, (uint8_t)interrupt_i2s, speaker_isr);
        i2s_enable_int(MASK_I2S_IE_FIFO_TX_EMPTY | MASK_I2S_IE_FIFO_TX_HALF_FULL);

        /* Start streaming audio */
        i2s_start_tx();
        interrupt_enable_globally();
    }
}

void speaker_begin()
{
    i2s_start_tx();
}

void speaker_end()
{
    i2s_stop_tx();
}

int speaker_ready()
{
    uint16_t uwFlag = i2s_get_status();
    return uwFlag & MASK_I2S_PEND_FIFO_TX_EMPTY;
}

void speaker_play(char* data, int size)
{
    if (size) {
        i2s_write((uint8_t*)data, size);
    }
}
