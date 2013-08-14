/*
 * CDC1114.h
 * lower level functions and constants for the CDC1114 analog to
 * digital converter (ADC). Can also work with CDC1113, CDC1013, CDC1014,
 * and (after some expansion) CDC1115, CDC1015
 */
#include "AD7152.h"
#include "I2C.h"

//right now this uses Bill's comm library. port to the TEMPO4k comm's!

unsigned char AD7152RxBuffer[16];
unsigned char CDCconfigs[MAX_CDC];
int CDCCommID = -1;
int currentCDCConfig = -1;
int devCount = 0;

//since the single-shot conversion requires you to resend the convert bit,
//this loCDC them into global vars.
int registerCDC(int config) {
	if (devCount == MAX_CDC - 1) {
		return -1; //too many devices!
	}

	if (CDCCommID == -1) { //setup an I2C if we don't have them
		CDCCommID = setupI2CDevice(CDC_ADDR, BAUD_CDC, &AD7152RxBuffer[0]);

		if (CDCCommID == -1) {
			return -2; //failed to register comm
		}
	}
	CDCconfigs[devCount] = config;
	devCount++;
	return devCount - 1;
}

int writeRegCDC(char rAddr, int value) {

	unsigned char cmd[2];
	cmd[0] = rAddr;
	cmd[1] = value;
	I2CWrite(2, &cmd[0]);
	while (isBusy())
		;

	return 1;
}

//returns only the first register, but fills a buffer of some length
int readRegCDC(char rAddr, int length) {
	I2CRead(length, &AD7152RxBuffer[0], rAddr);
	while (isBusy())
		;
	return AD7152RxBuffer[0];
}

#ifdef CDC_USE_SINGLE_SHOT
//single shot return CDC. Make sure the config's actually, y'know, setup for single shots
int singleShotCDC() {
	writeRegCDC(CDC_CONFIG, currentCDCConfig);
	return readRegCDC(CDC_STATUS, 5);
}

#endif

#ifdef CDC_USE_CONTINUOUS
//continuous reading CDC. Make sure the config's actually, y'know, setup for that
int startCDC() {
	return writeRegCDC(CDC_CONFIG, currentCDCConfig);
}

int readCDC() {
	return readRegCDC(CDC_STATUS, 5);
}

#endif

//get the real address, to write to registers directly
int getCDCCommID() {
	return CDCCommID;
}

//change which CDC config we're using, also activate the device on the bus.
int setCDC(int ID) {
	if (ID >= devCount || ID < 0) {
		return -1; //invalid device
	}
	if (CDCCommID == -1) {
		return 0;
	}
	currentCDCConfig = CDCconfigs[ID];
	changeI2CDevice(CDCCommID);
	return 1;
}

void resetCDC(){
	unsigned char cmd= CDC_RESET;
	I2CWrite(1, &cmd);
	while (isBusy())
		;
}
