/****************************************************************//**
 * \file clocks.h
 *
 * \author 	Ben Boudaoud
 * \date 	June 2013
 *
 * \brief 	This library provides system clock values for
 * 			reference in other time-sensitive libraries
 *******************************************************************/

#ifndef CLOCKS_H_
#define CLOCKS_H_

// Timing definitions for baud rate
#define	DCO_FREQ	8000000					///< DCO Clock Output Frequency
#define	MCLK_FREQ	8000000					///< MCLK Output Frequency
#define SMCLK_FREQ	8000000					///< SMCLK Output Frequency

#define delay_s(x)	__delay_cycles(MCLK_FREQ*(x))			///< Blocking delay macro (seconds)
#define delay_ms(x)	__delay_cycles(MCLK_FREQ*(x)/1000)		///< Blocking delay macro (milliseconds)
#define delay_us(x) __delay_cycles(MCLK_FREQ*(x)/1000000)	///< Blocking delay macro (microseconds)

#endif /* CLOCKS_H_ */
