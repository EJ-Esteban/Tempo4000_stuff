#include <msp430.h>
#include "I2C.h"
#include "AD7152.h"
//#include "comm.h"

unsigned char rxBuffer[32];
int ID, generalCall;
volatile char a;
extern char AD7152RxBuffer[16];

void clockSetup(void) {
	//lazy copied clock setup...
	__bis_SR_register(SCG0);
	// Disable the FLL control loop
	UCSCTL0 = 0x0000; // Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5; // Select DCO range 16MHz operation
	UCSCTL2 = FLLD_1 + 249; // Set DCO Multiplier for 8MHz
							// (N + 1) * FLLRef = Fdco
							// (249 + 1) * 32768 = 8MHz
	__bic_SR_register(SCG0);
	// Enable the FLL control loop

	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
	__delay_cycles(250000);

}

void portInit(void) {
	P1DIR = 0xFF;
	P1OUT = 0xFF;
	P4DIR = 0xFF;
	P6DIR = 0xFB;
	P6REN = 0x02;

}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

	clockSetup();
	portInit();

	initializeB0I2C();

	ID = registerCDC(CDC_CHEN1 | CDC_CHEN2 | CDC_SAMPLEC);
	setCDC(ID);

	_enable_interrupts();

	startCDC();

	while (1) {
	readCDC();

		_nop();
	}
}
