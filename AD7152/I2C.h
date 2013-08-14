/*
 * I2C.h
 *
 *  Created on: Jul 8, 2013
 *      Author: Bill Devine
 */

#ifndef I2C_H_
#define I2C_H_

#define OPEN 0
#define TX 1
#define RX 2

#define MAXDEV=8;

typedef struct i2cdev{

	unsigned int rAddr;
	unsigned int BaudDiv;
	unsigned int ControlW0;
	unsigned char *rxPnt;

} I2Cdevice;


#define statTX  0xFF
#define statRX  0x0F
#define statOpen  0x00



void initializeI2C(void);
int setupI2CDevice(unsigned char rAddr, unsigned int BaudDiv, unsigned char *rxPnt );
void changeI2CDevice(int commID);
void I2CRead(unsigned char countByte, unsigned char *RXpnter, unsigned char startRegister);
void I2CWrite(unsigned char countByte, unsigned char *pnter);
unsigned char I2CSlavePresent(int commID);
unsigned char isBusy(void);
unsigned char getportStatus(void);


#endif /* I2C_H_ */
