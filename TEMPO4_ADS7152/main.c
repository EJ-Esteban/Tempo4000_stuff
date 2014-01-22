/*
 * main.c
 */
#include <msp430.h>
#include "tempo4hal.h"
#include "clocks.h"
#include "timing.h"
#include "AD7152.h"

int myCDC;
volatile int a;
volatile char testReg, testVal;

#define CDC_CONT //comment out to demo single shot
extern char AD7152RxBuffer[16];

void main(void) {

	WDTCTL = WDTPW + WDTHOLD;

	_disable_interrupts();

	clkInit(DCO_FREQ);
	CHG_CONFIG();
	//LED_RED_CONFIG();
	//LED_GREEN_CONFIG();
	//SW1_CONFIG();
	_enable_interrupts();

#ifdef CDC_CONT
	//register ADC with default codeword for continuous sampling
	myCDC = registerCDC(CDC_CHEN1 | CDC_CHEN2 | CDC_SAMPLEC);

//	resetCDC();
	__delay_cycles(250000);

	setCDC(myCDC);

	writeRegCDC(CDC_POS, CDC_DACPEN | 0x01); //setup the CAPDAC (vertical offset capacitance compensation)
	writeRegCDC(CDC_SETUP1, CDC_SE_25pF); //set the full scale range

	startCDC();

	while (1) {
		_nop();
//		writeRegCDC(testReg, testVal);
		_nop();
		readRegCDC(CDC_STATUS, 16);
	}

#else
	myADC = registerADS(ADS_01, ADS_DEFAULT1); //register ADc with default codeword for 1 shot sampling
	setADS(myADC);
	while(1) {
		a = singleShotADS();
	}

#endif
}
