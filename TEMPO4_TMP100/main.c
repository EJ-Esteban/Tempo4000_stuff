/*
 * main.c
 *
 *
 */
#include <msp430.h>
#include "tempo4hal.h"
#include "clocks.h"
#include "timing.h"
#include "TMP100.h"

int myTMP;
volatile int a;

#define TMP_CONT //comment out to demo single shot

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;

	_disable_interrupts();

	clkInit(DCO_FREQ);
	CHG_CONFIG();
	//LED_RED_CONFIG();
	//LED_GREEN_CONFIG();
	//SW1_CONFIG();
	_enable_interrupts();

#ifdef TMP_CONT
	//register ADC with default codeword for continuous sampling
	myTMP = registerTMP(TMP100_01, TMP_DEFAULTC);

	setTMP(myTMP);
	startTMP();

	writeLongTMP(TMP_HI_THRESH, TMP_REF_100);//demo write register

	while (1) {
		a = readLongTMP(TMP_LO_THRESH);
		_nop();
		a = readLongTMP(TMP_HI_THRESH);
		_nop();
		a = readConfigTMP();
		_nop();
		a = readLongTMP(TMP_TEMP);
		_nop();

	}
#else
	myTMP = registerTMP(TMP100_01, TMP_DEFAULT1); //register ADc with default codeword for 1 shot sampling
	setTMP(myTMP);
	while(1){
	a = singleShotTMP();
	}

#endif
}
