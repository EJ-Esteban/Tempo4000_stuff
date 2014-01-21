/****************************************************************//**
 * \file timing.h
 *
 * \author 	Ben Boudaoud
 * \date 	June 2013
 *
 * \brief 	This library provides simple clock initialization and
 * 			control routines for the MSP430F5342
 *******************************************************************/

#ifndef TIMING_H_
#define TIMING_H_

#include <msp430.h>

#define CLOCK_STAB_PERIOD	2500000		///< Clock stabilization period
#define	FLLN_MASK			0x03FF		///< FLLN Mask for UCSCTL2 Register
#define ACLK_SRC_MASK		0x0700		///< Bit mask for ACLK source control
#define ACLK_DIV_MASK		0x0700		///< Bit mask for ACLK divisor control

typedef enum auxClkSrc					/// Enumerated type for ACLK source
{
	LFXT = SELA_0,						///< Off-chip low frequency external oscillator (crystal <50kHz)
	VLO =  SELA_1,						///< On-chip very low power, low frequency oscillator (~10kHz)
	REFO = SELA_2,						///< On-chip reference oscillator (32.768kHz)
	DCO =  SELA_3,						///< On-chip digitally controlled oscillator (up to 25MHz)
	DCODIV = SELA_4						///< On-chip division of DCO output
} aClkSrc;

typedef struct auxClockConfig			/// Enumerated type for ACLK configuration
{
	aClkSrc src;						///< ACLK source control
	unsigned char div;					///< ACLK divisor
} aclkConf;

clkInit(DCO_FREQ);
#endif /* TIMING_H_ */
