/*
 * AD7152.h
 * lower level functions and constants for the AD7152 capacitance to
 * digital converter (CDC)
 */

#ifndef AD7152_H_
#define AD7152_H_

//function prototypes

int registerCDC(int config);
int writeRegCDC(char rAddr, char value);
int readRegCDC(char rAddr, int length);
int singleShotCDC();
int startCDC();
int readCDC();
int getCDCCommID();
int setCDC(int ID);
void resetCDC();

//some
#define MAX_CDC		16
#define BAUD_CDC	20

#define CDC_USE_CONTINUOUS
#define CDC_USE_SINGLE_SHOT

//I2C address, will be bit shifted left once when adding R/W bit to 0x90 or 0x91
#define CDC_ADDR		0x48

//special ops
#define CDC_RESET		0xBF
#define CDC_GC_RESET	0x06	//resets the device if it is the address immediately following a general call

//register map
#define CDC_STATUS	0x00
#define CDC_D1MSB	0x01
#define CDC_D1LSB	0x02
#define CDC_D2MSB	0x03
#define CDC_D2LSB	0x04
#define CDC_O1MSB	0x05
#define CDC_O1LSB	0x06
#define CDC_O2MSB	0x07
#define CDC_O2LSB	0x08
#define CDC_G1MSB	0x09
#define CDC_G1LSB	0x0A
#define CDC_SETUP1	0x0B
#define CDC_G2MSB	0x0C
#define CDC_G2LSB	0x0D
#define CDC_SETUP2	0x0E
#define CDC_CONFIG	0x0F
#define CDC_POS		0x11
#define CDC_NEG		0x12
#define CDC_CONFIG2	0x1A

//status register 0x00
//Default 0x03 (R)
#define CDC_PWDN		0x80	//PWDN = 0 is default
#define CDC_C1C2		0x04	//indicates last op, C1C2 = 0 for Ch. 1, C1C2 = 1 for Ch.
#define CDC_RDY2		0x02	//0 if 2 has new sample ready
#define CDC_RDY1		0x01	//0 if 1 has new sample ready

//setup registers 0x0B (Ch 1) , 0x0E (Ch 2)
//Default 0x00 (R/W)
#define CDC_RANGE1		0x80
#define CDC_RANGE0		0x40
#define CDC_DIFFMODE	0x20

#define CDC_DIFF1pF		CDC_DIFFMODE							//1 pF differential mode
#define CDC_DIFF_25pF	CDC_DIFFMODE + CDC_RANGE0				//.25pF differential mode
#define CDC_DIFF_5pF		CDC_DIFFMODE + CDC_RANGE1			//.5pF differential mode
#define CDC_DIFF2pF		CDC_DIFFMODE + CDC_RANGE1 + CDC_RANGE0	//2 pF differential mode
#define CDC_SE2pF		0x00									//2 pF single ended mode
#define CDC_SE_5pF		CDC_RANGE0								//.5pF single ended mode
#define CDC_SE_25pF		CDC_RANGE1								//.25pF single ended mode
#define CDC_SE4pF		CDC_RANGE1 + CDC_RANGE0					//4 pF single ended mode

//config register 0x0F
//Default 0x00 (R/@)
#define CDC_CHEN1		0x10	//Set Channel Enables to 1 to enable channel sampling
#define CDC_CHEN2		0x08
#define CDC_MD2			0x04
#define CDC_MD1			0x02
#define CDC_MD0			0x01

#define CDC_IDLE			0x00				//idle mode
#define CDC_SAMPLEC		CDC_MD0				//Continuous conversion mode
#define CDC_SAMPLE1		CDC_MD1				//single conversion mode
#define CDC_SLEEP		CDC_MD0 + CDC_MD1	//power down mode
#define CDC_OFFSETCALIB	CDC_MD2 + CDC_MD0	//offset calib mode
#define CDC_GAINCALIB	CDC_MD2 + CDC_MD1	//gain calib mode

//config2 register 0x1A
//Defaul value 0x00 (R/W)
#define CDC_OSR1		0x20
#define CDC_OSR0		0x10

#define CDC_200Hz	0x00						//Sampling Rate 200 Hz
#define CDC_50Hz		CDC_OSR0					//Sampling Rate 50 Hz
#define CDC_20Hz		CDC_OSR1					//Sampling Rate 20 Hz
#define CDC_17Hz		CDC_OSR1 + CDC_OSR0	//Sampling Rate 16.7 Hz

//CDC pos register 0x11
//Default 0x00 (R/W)
#define CDC_DACPEN		0x80	//connects DAC POS to positive cap input

//CDC neg register 0x12
//Default 0x00 (R/W)
#define CDC_DACNEG		0x80	//connects DAC NEG to positive cap input

#endif /* AD7152_H_ */
