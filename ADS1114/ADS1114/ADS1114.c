/*
 * ADS1114.h
 * lower level functions and constants for the ADS1114 analog to
 * digital converter (ADC). Can also work with ADS1113, ADS1013, ADS1014,
 * and (after some expansion) ADS1115, ADS1015
 */
#include "ADS1114.h"
#include "I2C.h"

//right now this uses Bill's comm library. port to the TEMPO4k comm's!

unsigned char result[2];
unsigned int ADSconfigs[MAX_ADS];
int ADSCommIDs[MAX_ADS];
int currentADS = -1;
int devCount = 0;

//since the single-shot conversion requires you to resend the convert bit,
//this loads them into global vars.
int registerADS(unsigned char I2CAddress, int config) {
	if (devCount == MAX_ADS - 1) {
		return -1; //too many devices!
	}

	ADSCommIDs[devCount] = setupI2CDevice(I2CAddress, BAUD_ADS, &result[0]);
	if (ADSCommIDs[devCount] == -1) {
		return -2; //failed to register comm
	}

	ADSconfigs[devCount] = config;
	devCount++;
	return devCount - 1;
}

int writeRegADS(char rAddr, int value) {

	unsigned char cmd[3];
	cmd[0] = rAddr;
	cmd[1] = BytesH(&value);
	cmd[2] = BytesL(&value);
	UCB0I2CWrite(3, &cmd[0]);
	while (isBusy())
		;

	return 1;
}

int readRegADS(char rAddr) {
	UCB0I2CRead(2, &result[0], rAddr);
	while (isBusy())
		;

	return result[0] << 8 | result[1];
}

#ifdef ADS_USE_SINGLE_SHOT
//single shot return ADS. Make sure the config's actually, y'know, setup for single shots
int singleShotADS() {
	writeRegADS(ADS_CONFIG, ADSconfigs[currentADS]);
	return readRegADS(ADS_CONV);
}

#endif

#ifdef ADS_USE_CONTINUOUS
//continuous reading ADS. Make sure the config's actually, y'know, setup for that
int startADS(){
	return 	writeRegADS(ADS_CONFIG, ADSconfigs[currentADS]);
}

int readADS() {
	return readRegADS(ADS_CONV);
}

#endif

//get the real address, to write to registers directly
int getADSCommID(int ID) {
	if (ID >= devCount || ID < 0) {
		return -1;
	}
	return ADSCommIDs[ID];

}

//change which ADS/ADS config we're talking to
int setADS(int ID) {
	if (ID >= devCount || ID < 0) {
		return -1;
	}
	if (currentADS == ID) {
		return 0;
	}
	currentADS = ID;
	changeI2CDevice(ADSCommIDs[ID]);
	return 1;

}
