/**
  @file ulaw.c
  @brief
  ulaw companding module

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

#include "ft900.h"



////////////////////////////////////////////////////////////////////////////////////////

#define	SIGN_BIT    (0x80)
#define	QUANT_MASK  (0xf)
#define	SEG_SHIFT   (4)
#define	SEG_MASK    (0x70)
#define BIAS        (0x84)
#define CLIP        (8159)



////////////////////////////////////////////////////////////////////////////////////////

static short seg_uend[8] = {0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF};

static short search(
   short val,
   short *table,
   short size)
{
   short i;

   for (i = 0; i < size; i++) {
      if (val <= *table++)
	 return (i);
   }
   return (size);
}

unsigned char linear2ulaw(short pcm_val)
{
   short mask;
   short seg;
   unsigned char uval;

   pcm_val = pcm_val >> 2;
   if (pcm_val < 0) {
      pcm_val = -pcm_val;
      mask = 0x7F;
   } else {
      mask = 0xFF;
   }
   if ( pcm_val > CLIP ) pcm_val = CLIP;
   pcm_val += (BIAS >> 2);

   seg = search(pcm_val, seg_uend, 8);
   if (seg >= 8)
      return (unsigned char) (0x7F ^ mask);
   else {
      uval = (unsigned char) (seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
      return (uval ^ mask);
   }
}

short ulaw2linear(unsigned char	u_val)
{
   u_val = ~u_val;
   short t = ((u_val & QUANT_MASK) << 3) + BIAS;
   t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;
   return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}




////////////////////////////////////////////////////////////////////////////////////////

void pcm16_to_ulaw(int src_length, const char *src_samples, char *dst_samples)
{
    int i;
    const unsigned short *s_samples;

    s_samples = (const unsigned short *)src_samples;
    for (i=0; i < src_length / 2; i++) {
        dst_samples[i] = linear2ulaw((short) s_samples[i]);
    }
}

void ulaw_to_pcm16(int src_length, const char *src_samples, char *dst_samples)
{
    int i;
    unsigned char *s_samples;
    unsigned short *d_samples;

    s_samples = (unsigned char *) src_samples;
    d_samples = (unsigned short *)dst_samples;

    for (i=0; i < src_length; i++) {
        d_samples[i] = ulaw2linear(s_samples[i]);
    }
}


