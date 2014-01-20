/*
 * main.c
 */
#include <msp430.h>
#include "tempo4hal.h"
#include "clocks.h"
#include "timing.h"
#include "ADS1114new.h"

int myADC;
volatile int a;

#define ADS_CONT //comment out to demo single shot

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;

	_disable_interrupts();

	clkInit(DCO_FREQ);
	CHG_CONFIG();
	//LED_RED_CONFIG();
	//LED_GREEN_CONFIG();
	//SW1_CONFIG();
	_enable_interrupts();

#ifdef ADS_CONT
	//register ADC with default codeword for continuous sampling
	myADC = registerADS(ADS_01, ADS_DEFAULTC);

	setADS(myADC);
	startADS();

	writeRegADS(ADS_HI_THRESH, 0x7FFE);//demo write register

	while (1) {
		a = readRegADS(ADS_LO_THRESH);
		_nop();
		a = readRegADS(ADS_HI_THRESH);
		_nop();
		a = readRegADS(ADS_CONFIG);
		_nop();
		a = readRegADS(ADS_CONV);
		_nop();
	}
#else
	myADC = registerADS(ADS_01, ADS_DEFAULT1); //register ADc with default codeword for 1 shot sampling
	setADS(myADC);
	while(1){
	a = singleShotADS();
	}

#endif
}
