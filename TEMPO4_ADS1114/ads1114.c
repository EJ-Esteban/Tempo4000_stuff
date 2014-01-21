/*
 * ADS1114.h
 * lower level functions and constants for the ADS1114 analog to
 * digital converter (ADC). Can also work with ADS1113 and (after
 * some expansion) ADS 1115
 */

#include <msp430f5342.h>
#include "ADS1114.h"
#include "comm.h"
//#include "final_testboard_hal.h"
#include "tempo4hal.h"
#include "clocks.h"

unsigned char rxADS[4];
int ADSCommIDs[MAX_ADS];
int ADSConfigs[MAX_ADS];
int ADSCurrent = -1;
int ADSdevCount = 0; //next slot to place

static usciConfig ADSConf00 = { (UCB1_I2C + ADS_00), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxADS };
static usciConfig ADSConf01 = { (UCB1_I2C + ADS_01), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxADS };
static usciConfig ADSConf10 = { (UCB1_I2C + ADS_10), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxADS };
static usciConfig ADSConf11 = { (UCB1_I2C + ADS_11), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxADS };

/**************************************************************************//**
 * \fn 		registerADS(unsigned char I2CAddress, int config)
 * \brief	This function registers the I2C port for communication and sets up
 * 			the ADS1114. Devices are stored by IDs independent of their comm IDs
 *
 *
 * \retval	-1		too many devices
 * \retval  -2		bad address
 * \retval 	-3		comm failed to init
 * \retval (nonnegative,int)	ADS id number of device
 *****************************************************************************/
int registerADS(unsigned char I2CAddress, int config) {
	if (ADSdevCount == MAX_ADS) {
		return -1; //too many devices
	}
	int ADSCommID;
	switch (I2CAddress) {
	case (ADS_00):
		ADSCommID = registerComm(&ADSConf00);
		break;
	case (ADS_01):
		ADSCommID = registerComm(&ADSConf01);
		break;
	case (ADS_10):
		ADSCommID = registerComm(&ADSConf10);
		break;
	case (ADS_11):
		ADSCommID = registerComm(&ADSConf11);
		break;
	default:
		return -2; //baddress
	}
	MPU_IO_INIT(); //set lines for I2C communication
	if (ADSCommID == -1) {
		return -3; //failed to init at comm level
	}

	ADSCommIDs[ADSdevCount] = ADSCommID;
	ADSConfigs[ADSdevCount] = config;
	ADSdevCount++;
	return ADSdevCount - 1; //returns nonnegative ADS device ID (not comm ID) on proper init
}

int writeRegADS(char rAddr, int value) {
	i2cPacket packet; // Packet for i2c transmission
	int ret = 0; // Return value

	packet.commID = ADSCommIDs[ADSCurrent]; // Set up the commID for the packet transfer
	packet.regAddr = rAddr; // Set up the config register
	packet.len = 2; // Set the packet length to 2 bytes byte

	unsigned char cmd[2];
	cmd[0] = BytesH(&value);
	cmd[1] = BytesL(&value);

	packet.data = &cmd[0]; // Pack the commands to write into the packet

	ret = i2cB1Write(&packet); // Write the packet to USCI B1
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return

	return ret;
}

int readRegADS(char rAddr) {
	i2cPacket packet;

	packet.commID = ADSCommIDs[ADSCurrent]; // Set up the commID for the packet transfer
	packet.regAddr = rAddr; // Set up the config register
	packet.len = 4; // Set the packet length to a single byte

	packet.data = &rxADS[0]; // Aim the read pointer at the rx buffer

	if (i2cB1Read(&packet) == -1) // Attempt to read the packet from USCI B1
		return -1;
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return
	return rxADS[0] << 8 | rxADS[1];
}

int setADS(int ID) {
	if (ID >= ADSdevCount || ID < 0) {
		return -1;
	}
	if (ADSCurrent == ID) {
		return 0;
	}
	ADSCurrent = ID;
	//writeRegADS(ADS_CONFIG, ADSConfigs[ADSCurrent]);
	return 1;
}

//get the real address, to write to registers directly
int getADSCommID(int ID) {
	if (ID >= ADSdevCount || ID < 0) {
		return -1;
	}
	return ADSCommIDs[ID];

}

#ifdef ADS_USE_SINGLE_SHOT
//single shot return ADS. Make sure the config's actually, y'know, setup for single shots
int singleShotADS() {
	writeRegADS(ADS_CONFIG, ADSConfigs[ADSCurrent]);
	return readRegADS(ADS_CONV);
}

#endif

#ifdef ADS_USE_CONTINUOUS
//continuous reading ADS. Make sure the config's actually, y'know, setup for that
int startADS() {
	return writeRegADS(ADS_CONFIG, ADSConfigs[ADSCurrent]);
}

int readADS() {
	return readRegADS(ADS_CONV);
}

#endif
