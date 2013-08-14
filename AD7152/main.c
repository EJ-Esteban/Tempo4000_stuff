/********************************************************************************
 * This program uses a messy combo of Bill's I2C library and the TEMPO4k comm lib.
 *
 * When properly hooked up to a launchpad, it dumps serial values into the comm port over uart,
 * which is nifty. This program is an example, but the chip can be used by transferring the
 * AD7152.c,  AD7152.h, and pertinent comm library files.
 *
 * The exact hookup besides power and whatnot:
 *
 * MSP5342 pin 25 -> Launchpad p1.2
 * MSP5342 pin 26 -> Launchpad p1.1
 * MSP5342 pin 28 -> CDC pin 10
 * MSP5342 pin 29 -> CDC pin 9
 *
 * make sure the jumpers are set for hardware uart and there's NO MSP in the launchpad
 * socket, otherwise it won't work.
 ******************************************************************************/
#include "msp430f5342.h"
#include "comm.h"
#include "useful.h"
#include "I2C.h"
#include "AD7152.h"


unsigned char RXData[16];
usciConfig myCommPort = { UCA0_UART, UART_8N1, DEF_CTLW1, UBR_DIV(9600),
		&RXData[0] };
int myCommID, I2CID;
int transmit = 1;
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

void timerASetup(void) {

	TA0CTL = TASSEL_2 + ID_3 + MC_1;

	TA0CCTL0 = CCIE;
	TA0CCR0 = 15778; //mock clock calib: ideal value * .9861 scaling factor
	//CCR0 = 16000 //ideal value
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerISR(void) {
	_nop();

	if (transmit) {
		uartA0Write(&AD7152RxBuffer[1], 2, myCommID); //tx ch 1 registers

	}
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
	clockSetup();
	timerASetup();
	myCommID = registerComm(&myCommPort);

	initializeI2C();

	I2CID = registerCDC(CDC_CHEN1 | CDC_SAMPLEC); //channel 1 continuous sampling
	setCDC(I2CID);

	_enable_interrupts();
	resetCDC();
	__delay_cycles(250000);

	writeRegCDC(CDC_POS, CDC_DACPEN | 0x09); //setup the CAPDAC (vertical offset capacitance compensation)
	writeRegCDC(CDC_SETUP1, CDC_SE_25pF); //set the full scale range
	startCDC();



	while (1) {
		if (getUCA0RxSize() > 0) {
			uartA0Read(1, myCommID);
			if (RXData[0] == 'q') {
				transmit ^= 1;
			}
		}
		readCDC();

	}
}


