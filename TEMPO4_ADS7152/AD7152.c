/*
 * CDC1114.h
 * lower level functions and constants for the CDC1114 analog to
 * digital converter (ADC). Can also work with CDC1113, CDC1013, CDC1014,
 * and (after some expansion) CDC1115, CDC1015
 */

#include <msp430f5342.h>
#include "AD7152.h"
#include "comm.h"
//#include "final_testboard_hal.h"
#include "tempo4hal.h"
#include "clocks.h"

unsigned char AD7152RxBuffer[16];
unsigned char CDCconfigs[MAX_CDC];
//there will never be more than one comm resource--only one address!
int CDCCommID = -1;
int currentCDCConfig = -1;
int devCount = 0;

static usciConfig AD7152Conf = { (UCB1_I2C + CDC_ADDR), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), AD7152RxBuffer };


int registerCDC(int config) {
	if (devCount == MAX_CDC - 1) {
		return -1; //too many devices!
	}

	if (CDCCommID == -1) { //setup an I2C if we don't have them
		CDCCommID = registerComm(&AD7152Conf);

		if (CDCCommID == -1) {
			return -2; //failed to register comm
		}
	}

	CDCconfigs[devCount] = config;
	devCount++;
	return devCount - 1;
}

int writeRegCDC(char rAddr, char value){
	i2cPacket packet; // Packet for i2c transmission
	int ret = 0; // Return value

	packet.commID = CDCCommID; // Set up the commID for the packet transfer
	packet.regAddr = rAddr; // Set up the config register
	packet.len = 1; // Set the packet length to 2 bytes byte

	unsigned char cmd = value;
	packet.data = &cmd; // Pack the commands to write into the packet

	ret = i2cB1Write(&packet); // Write the packet to USCI B1
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return

	return ret;
}

//returns only the first register, but fills a buffer of some length
int readRegCDC(char rAddr, int length) {
	i2cPacket packet;

	packet.commID = CDCCommID; // Set up the commID for the packet transfer
	packet.regAddr = rAddr; // Set up the config register
	packet.len = length; // Set the packet length to a single byte

	packet.data = &AD7152RxBuffer[0]; // Aim the read pointer at the rx buffer

	if (i2cB1Read(&packet) == -1) // Attempt to read the packet from USCI B1
		return -1;
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return
	return AD7152RxBuffer[0];//return first thing retrieved
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
	return 1;
}

void resetCDC(){
	writeRegCDC(CDC_RESET,0x00);
}

