/*
 * TMP100.h
 * lower level functions and constants for the TMP100 digital
 * temperature sensor. Also compatible with TMP101
 */

#include <msp430f5342.h>
#include "TMP100.h"
#include "comm.h"
//#include "final_testboard_hal.h"
#include "tempo4hal.h"
#include "clocks.h"


#define TMP_USE_SINGLE_SHOT
#define TMP_USE_CONTINUOUS

unsigned char rxTMP[2];
int TMPCommIDs[MAX_TMP];
int TMPConfigs[MAX_TMP];
int TMPCurrent = -1;
int TMPdevCount = 0; //next slot to place

static usciConfig TMPConf00 = { (UCB1_I2C + TMP100_00), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConf0F = { (UCB1_I2C + TMP100_0F), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConf01 = { (UCB1_I2C + TMP100_01), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConf10 = { (UCB1_I2C + TMP100_10), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConf1F = { (UCB1_I2C + TMP100_1F), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConf11 = { (UCB1_I2C + TMP100_11), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConfF0 = { (UCB1_I2C + TMP100_F0), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };
static usciConfig TMPConfF1 = { (UCB1_I2C + TMP100_F1), I2C_7SM, DEF_CTLW1,
UBR_DIV(20000), rxTMP };

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
int registerTMP(unsigned char I2CAddress, int config) {
	if (TMPdevCount == MAX_TMP) {
		return -1; //too many devices
	}
	int TMPCommID;
	switch (I2CAddress) {
	case (TMP100_00):
		TMPCommID = registerComm(&TMPConf00);
		break;
	case (TMP100_0F):
		TMPCommID = registerComm(&TMPConf0F);
		break;
	case (TMP100_01):
		TMPCommID = registerComm(&TMPConf01);
		break;
	case (TMP100_10):
		TMPCommID = registerComm(&TMPConf10);
		break;
	case (TMP100_1F):
		TMPCommID = registerComm(&TMPConf1F);
		break;
	case (TMP100_11):
		TMPCommID = registerComm(&TMPConf11);
		break;
	case (TMP100_F0):
		TMPCommID = registerComm(&TMPConfF0);
		break;
	case (TMP100_F1):
		TMPCommID = registerComm(&TMPConfF1);
		break;

	default:
		return -2; //baddress
	}
	MPU_IO_INIT(); //set lines for I2C communication
	if (TMPCommID == -1) {
		return -3; //failed to init at comm level
	}

	TMPCommIDs[TMPdevCount] = TMPCommID;
	TMPConfigs[TMPdevCount] = config;
	TMPdevCount++;
	return TMPdevCount - 1; //returns nonnegative TMP device ID (not comm ID) on proper init
}

int writeLongTMP(char rAddr, int value) {
	i2cPacket packet; // Packet for i2c transmission
	int ret = 0; // Return value

	packet.commID = TMPCommIDs[TMPCurrent]; // Set up the commID for the packet transfer
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

int writeConfigTMP(int value) {
	i2cPacket packet; // Packet for i2c transmission
	int ret = 0; // Return value

	packet.commID = TMPCommIDs[TMPCurrent]; // Set up the commID for the packet transfer
	packet.regAddr = TMP_CONFIG; // Set up the config register
	packet.len = 1; // Set the packet length to 2 bytes byte
	unsigned char cmd = BytesL(&value);

	packet.data = &cmd; // Pack the commands to write into the packet

	ret = i2cB1Write(&packet); // Write the packet to USCI B1
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return

	return ret;
}

int readLongTMP(char rAddr) {
	i2cPacket packet;

	packet.commID = TMPCommIDs[TMPCurrent]; // Set up the commID for the packet transfer
	packet.regAddr = rAddr; // Set up the config register
	packet.len = 2; // Set the packet length to a single byte

	packet.data = &rxTMP[0]; // Aim the read pointer at the rx buffer

	if (i2cB1Read(&packet) == -1) // Attempt to read the packet from USCI B1
		return -1;
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return
	return rxTMP[0] << 8 | rxTMP[1];
}

int readConfigTMP() {
	i2cPacket packet;

	packet.commID = TMPCommIDs[TMPCurrent]; // Set up the commID for the packet transfer
	packet.regAddr = TMP_CONFIG; // Set up the config register
	packet.len = 1; // Set the packet length to a single byte

	packet.data = &rxTMP[0]; // Aim the read pointer at the rx buffer

	if (i2cB1Read(&packet) == -1) // Attempt to read the packet from USCI B1
		return -1;
	while (getUCB1Stat() != OPEN)
		; // Wait for the resource to open before return
	return rxTMP[0];

}

int TMPAlertStatus() {
	int a = readConfigTMP();
	if (a & TMP_ALERT) //don't forget that whether or not this is an ALERT depends on configured polarity!
		return 1;
	return 0;
}

int setTMP(int ID) {
	if (ID >= TMPdevCount || ID < 0) {
		return -1;
	}
	if (TMPCurrent == ID) {
		return 0;
	}
	TMPCurrent = ID;
	return 1;
}

//get the real address, to write to registers directly
int getTMPCommID(int ID) {
	if (ID >= TMPdevCount || ID < 0) {
		return -1;
	}
	return TMPCommIDs[ID];

}

#ifdef TMP_USE_SINGLE_SHOT
//single shot return ADS. Make sure the config's actually, y'know, setup for single shots
int singleShotTMP() {
	writeConfigTMP(TMPConfigs[TMPCurrent]);
	return readLongTMP(TMP_TEMP);
}

#endif

#ifdef TMP_USE_CONTINUOUS
//continuous reading ADS. Make sure the config's actually, y'know, setup for that
int startTMP() {
	return writeConfigTMP(TMPConfigs[TMPCurrent]);
}

int readTMP() {
	return readLongTMP(TMP_TEMP);
}

#endif
