/*
 * ADS1114.h
 * lower level functions and constants for the ADS1114 analog to
 * digital converter (ADC). Can also work with ADS1113 and (after
 * some expansion) ADS 1115
 */

#ifndef ADS1114_H_
#define ADS1114_H_

//modes include the
#define ADS_USE_SINGLE_SHOT
#define ADS_USE_CONTINUOUS

typedef struct byteSplitter
{
    volatile unsigned ByteL: 8;
    volatile unsigned ByteH: 8;
} byteSplit ;

#define BytesH(x) (((byteSplit *) (x))->ByteH)				// Bit in Byte Defines
#define BytesL(x) (((byteSplit *) (x))->ByteL)

//I2C settings
#define MAX_ADS				4			//distinct configs allowed
#define BAUD_ADS			400

//I2C addresses
#define	ADS_00				0x48	//tie ADDR to GND
#define ADS_01				0x49	//tie ADDR to VDD
#define ADS_10				0x4A	//tie ADDR to SDA
#define ADS_11				0x4B	//tie ADDR TO SCL

//read/write macro
#define ADS_WRITE(x)		(x)|0x01
#define ADS_READ(x)			(x)&0xFE

//special ops
#define ADS_GC_RESET		0x06	//resets the device if it is the address immediately following a general call

//Register map
#define ADS_CONV			0x00	//Conversions register
#define ADS_CONFIG			0x01	//Config register
#define ADS_LO_THRESH		0x02	//Low Threshold Register
#define ADS_HI_THRESH		0x03	//High Threshold Register

//Config Register 0x01
//Default 0x8583 (R/W)
#define ADS_OS				0x8000	//Operational Status: 1 = begin single conversion (in write mode), 0 = device is performing conversion (in read mode)
#define ADS_MUX2			0x4000	//Input Mux config bit
#define ADS_MUX1			0x2000	//Input Mux config bit
#define ADS_MUX0			0x1000	//Input Mux config bit
#define ADS_PGA2			0x0800	//PGA config bit
#define ADS_PGA1			0x0400	//PGA config bit
#define ADS_PGA0			0x0200	//PGA config bit
#define ADS_MODE			0x0100	//mode bit: 0 = continuous conversion, 1 = power down single-shot (default)
#define ADS_DR2				0x0080	//data rate config bit
#define ADS_DR1				0x0040	//data rate config bit
#define ADS_DR0				0x0020	//data rate config bit
#define ADS_COMP_MODE		0x0010	//comparator mode bit,  0 = traditional (deftaul), 1 = window
#define ADS_COMP_POL		0x0008	//comparator polarity, 0 = active low (default), 1 = active high
#define ADS_COMP_LAT		0x0004	//latching comparator bit (ALRT/RDY pin) 0 = non-latching (default), 1 = latching
#define ADS_COMP_QUE1		0x0002	//comparator queue config bit
#define ADS_COMP_QUE0		0x0001	//comparator queue config bit

//TODO: define mux config bits if you wanna use ADS 1115.

#define ADS_PGA_6			0x00					//Full Scale = ±6.144V (don't go over VDD + .3!)
#define ADS_PGA_4			ADS_PGA0				//Full Scale = ±4.096V (don't go over VDD + .3!)
#define ADS_PGA_2			ADS_PGA1				//Full Scale = ±2.048V (default)
#define ADS_PGA_1			ADS_PGA1 + ADS_PGA0		//Full Scale = ±1.024V
#define ADS_PGA_512			ADS_PGA2				//Full Scale = ±.512V
#define ADS_PGA_256			ADS_PGA2 + ADS_PGA_0	//Full Scale = ±.256V

#define ADS_CONV1			ADS_MODE				//single shot mode
#define ADS_CONVC			0x00					//continuous conversion

#define ADS_8Hz				0x00						//Sampling Rate 8 Hz
#define ADS_16Hz			ADS_DR0						//Sampling Rate 16 Hz
#define ADS_32Hz			ADS_DR1						//Sampling Rate 32 Hz
#define ADS_64Hz			ADS_DR1 + ADS_DR0			//Sampling Rate 64 Hz
#define ADS_128Hz			ADS_DR2						//Sampling Rate 128 Hz (default)
#define ADS_250Hz			ADS_DR2 + ADS_DR0			//Sampling Rate 250 Hz
#define ADS_475Hz			ADS_DR2 + ADS_DR1			//Sampling Rate 475 Hz
#define ADS_860Hz			ADS_DR2 + ADS_DR1 + ADS_DR0	//Sampling Rate 860 Hz

#define ADS_ALERT_1			0x00							//assert ALERT/RDY after 1 conversion exceeds thresholds
#define ADS_ALERT_2			ADS_COMP_QUE0					//assert ALERT/RDY after 2 conversions exceed thresholds
#define ADS_ALERT_4			ADS_COMP_QUE1					//assert ALERT/RDY after 4 conversions exceed thresholds
#define ADS_ALERT_NO		ADS_COMP_QUE1 + ADS_COMP_QUE0	//keep ALERT/RDY high (default)

#define ADS_DEFAULT1	0x8583		//default single shot conversion word
#define ADS_DEFAULTC	0x0483		//default continuous conversion word

//Lo_thresh Register 0x02
//Default 0x8000 (R/W)
#define ADS_RDY_LO			0x0000	//Set Lo_Thresh so ALERT/RDY asserts OS bit (also needs ADS_RDY_HI)

//Hi_thresh Register 0x03
//Default 0x7FFF (R/W)
#define ADS_RDY_HI			0x8000	//Set Hi_Thresh so ALERT/RDY asserts OS bit (also needs ADS_RDY_LO)

//function prototypes
int registerADS(unsigned char I2CAddress, int config);
int writeRegADS(char rAddr, int value);
int readRegADS(char rAddr);
int singleShotADS();
int readADS();
int setADS(int ID);
int getADSCommID(int ID);
int startADS();

#endif /* ADS1114_H_ */
