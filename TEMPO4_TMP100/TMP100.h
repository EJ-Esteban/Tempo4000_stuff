#ifndef TMP100_H_
#define TMP100_H_

/*
 * TMP100.h
 * lower level functions and constants for the TMP100 digital
 * temperature sensor. Also compatible with TMP101
 */
//modes include the

typedef struct byteSplitter
{
    volatile unsigned ByteL: 8;
    volatile unsigned ByteH: 8;
} byteSplit ;

#define BytesH(x) (((byteSplit *) (x))->ByteH)				// Bit in Byte Defines
#define BytesL(x) (((byteSplit *) (x))->ByteL)

//I2C settings
#define MAX_TMP				4			//distinct configs allowed
#define BAUD_TMP			400

//I2C addresses
//TMP 100
#define	TMP100_00			0x48	//tie A1 to gnd, A0 to gnd
#define	TMP100_0F			0x49	//tie A1 to gnd, A0 floats
#define	TMP100_01			0x4A	//tie A1 to gnd, A0 to vdd
#define	TMP100_10			0x4C	//tie A1 to vdd, A0 to gnd
#define	TMP100_1F			0x4D	//tie A1 to vdd, A0 floats
#define	TMP100_11			0x4E	//tie A1 to vdd, A0 to vdd
#define	TMP100_F0			0x4B	//A1 floats, A0 to gnd
#define	TMP100_F1			0x4F	//A1 floats, A0 to vdd
//TMP 101
#define TMP101_0			0x48	//tie Address to GND
#define TMP101_F			0x49	//Address floats
#define TMP101_1			0x4A	//tie Address to vdd

//read/write macro
#define TMP_WRITE(x)		(x)|0x01
#define TMP_READ(x)			(x)&0xFE

//special ops
#define TMP_GC_RESET		0x06	//resets the device if it is the address immediately following a general call

//Register map
#define TMP_TEMP			0x00	//measured temperature register (Read only)
#define TMP_CONFIG			0x01	//Config register
#define TMP_LO_THRESH		0x02	//Low Threshold Register
#define TMP_HI_THRESH		0x03	//High Threshold Register

//Config Register 0x01
// this register is only 8 bits. The most significant bit
// is a one shot trigger when written and an alert (equal to
// the ALERT pin on the TMP101) when read
//Default 0x00 (written)
//Default 0x80 (read)
#define TMP_OS				0x80	//begin single shot conversion
#define TMP_ALERT			0x80	//ALERT bitmask
#define TMP_R1				0x40	//converter resolution bit 1
#define TMP_R0				0x20	//converter resolution bit 0
#define TMP_F1				0x10	//fault selector bit 1
#define TMP_F0				0x08	//fault selector bit 0
#define TMP_POL				0x04	//ALERT polarity mode
#define TMP_TM				0x02	//thermostat mode
#define TMP_SD				0x01	//shutdown after conversion

//with lower resolutions, least significant bits will not be filled
#define TMP_RES_9BITS		0x00			//9 bit resolution for conversion register
#define TMP_RES_10BITS		TMP_R0			//10 bit resolution for conversion register
#define TMP_RES_11BITS		TMP_R1			//11 bit resolution for conversion register
#define TMP_RES_12BITS		TMP_R1 + TMP_R0	//12 bit resolution for conversion register

//more faults crossing thresholds to trigger alerts
#define TMP_1FAULT			0x00			//assert ALERT after 1 conversion exceeds thresholds
#define TMP_2FAULT			TMP_R0			//assert ALERT after 2 conversions exceed thresholds
#define TMP_4FAULT			TMP_R1			//assert ALERT after 4 conversions exceed thresholds
#define TMP_6FAULT			TMP_R1 + TMP_R0	//assert ALERT after 6 conversions exceed thresholds

//polarity
#define TMP_POL_POS			0x00			//ALERT is active low
#define TMP_POL_NEG			TMP_POL			//ALERT is active high

//thermostat modes
#define TMP_TM_COMPARATOR	0x00			//Asserts ALERT when temp crosses thresholds
#define TMP_TM_INTERRUPT	TMP_TM			//Asserts ALERT when temp crosses thresholds, reverts ALERT after a read

//shutdown modes
#define TMP_CCONV			0x00			//continuous conversion
#define TMP_1CONV 			TMP_SD			//shut down after 1 shot conversion

//Defaults
#define TMP_DEFAULTC		TMP_CCONV
#define TMP_DEFAULT1		TMP_SD + TMP_OS

//Lo_thresh Register 0x02
//Default 0x4B00 (R/W)
//(75 deg Celcius)
//Hi_thresh Register 0x03
//Default 0x5000 (R/W)
//(80 deg Celcius)
//lowest 4 bits unused for both registers
#define TMP_LO_DEFAULT		0x4B00
#define TMP_HI_DEFULT		0x5000
#define TMP_REF_128			0x7FFF	//technically 27.9375 degrees
#define TMP_REF_100			0x6400
#define TMP_REF_80			0x5000
#define TMP_REF_75			0x4B00
#define TMP_REF_50			0x3200
#define TMP_REF_25			0x1900
#define TMP_REF_0			0x0000
//some useful macros
#define TMP_NEG(x)			~x + 1	//negates the reference by doing a one's complement thing
#define TMP_TRUNC(x)		x>>8	//shifts out the LSBs, giving integer temperature

//function prototypes
int registerTMP(unsigned char I2CAddress, int config);
int writeLongTMP(char rAddr, int value);
int writeConfigTMP(int value);
int readLongTMP(char rAddr);
int readConfigTMP();
int TMPAlertStatus();
int singleShotTMP();
int readTMP();
int setTMP(int ID);
int getTMPCommID(int ID);
int startTMP();

#endif /* TMP100_H_ */
