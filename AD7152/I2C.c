
/*

 *
 *Acknowledgements to:
 *
 *Uli Kretzschmar
 *Texas Instruments Deutschland GmbH
 *November 2007
 *
 *
 * Bill Devine
 * 8 July 2013
 *version 0.0
 */
/************************************************************************************************************//**
 * \file I2C.c
 * \author Bill Devine
 * \version 1.0
 * \date  8/7/2013
 *
 * \brief Library for I2C Master communication on USCI Port B1.
 *
 * \section DESCRIPTION
 * This code takes many cues from the official TI_USCI_I2C_master libraries.
 * The only substantial differences are those related to porting the library to
 * our specific device, which only really required a change in port definitions,
 * interrupt vector definitions, and various other definitions. This is not
 * totally original work, but as I2C is an officially recognized standard, and
 * these files were available as a DEMO file (free) from TI, it made more sense
 * to port verified, USCI module specific code to our device than try to work
 * from scratch. Also, very little in their code can be considered original or
 * of ambiguous origin, because they mainly use definitions, such as in the
 * initialize functions, the TX/RX functions, and the 'isBusy' function.
 * I make no claims of ownership or original authorship of the underlying code,
 * just a claim to porting it to our specific platform.
 *
 * This functionality will be migrated to the TEMPO4K USCI libraries soon, but can be
 * utilized in this form in the interim.
 *
 *
 ***************************************************************************************************************/

#include <msp430f5342.h>
#include "I2C.h"
unsigned char portStatus;///<Status Byte
unsigned char byteCountRX; ///< Counter of bytes to receive.
unsigned char byteCountTX; ///<Counter of bytes to transmit
 unsigned char *RXBytes; ///<Pointer to store received byte
unsigned char *TXBytes;///<Pointer to byte to transmit
 int I2CcommID=0; ///<current communications ID
 I2Cdevice *deviceArray[8]; ///<Buffer of I2C devices connected to bus.


void initializeI2C(void);
int setupI2CDevice(unsigned char rAddr, unsigned int BaudDiv, unsigned char *rxPnt );
void changeI2CDevice(int commID);
void I2CRead(unsigned char countByte, unsigned char *RXpnter, unsigned char startRegister);
void I2CWrite(unsigned char countByte, unsigned char *pnter);
unsigned char I2CSlavePresent(int commID);
unsigned char isBusy(void);
unsigned char getportStatus(void);


unsigned char getportStatus(void){
	return portStatus;
}
/************************************************************************************************************//**
 * \brief Initializes USCI Port B1 for use as an I2C Master Node.
 *
 * \details Initializes the byte counters to 0x00, sets the correct Pins to be used with peripheral modules
 * and selects the correct settings for I2C single Master Mode. Also enables Nack, TX, and RX
 * interrupts. Must be called before any I2C operations can occur.
 *
 * \param void
 *
 * \return void
 *
 ***************************************************************************************************************/
void initializeI2C(void){
	portStatus=0x00;
	WDTCTL = WDTPW + WDTHOLD;              //Stop the Watchdog Timer before changing anything
	  byteCountTX=0x00;										//Initialize the TX counter to 0x00
	  byteCountRX=0x00;										//Initialize the RX counter to 0x00
	  P4SEL |= 0x06;                           						//Select P4.1 and P4.2 to be used with peripherals
	  UCB1CTL1 |= UCSWRST;                      			//Enable USCI Software reset. Necessary before changing settings.
	  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     //Set the USCI module to be a Master Node, for I2C, in synchronous mode.
	  UCB1CTL1 = UCSSEL_2 + UCSWRST;  //Keeping the Reset enabled, choose the SMCLK to time the USCI module.
	  UCB1IE|=UCNACKIE;
	  UCB1CTL1 &= ~UCSWRST;                     		//Clear SW reset, allowing the USCI module to operate normally.
	  I2CcommID=0;
	  UCB1IFG=0x00;
	  UCB1IE|=UCRXIE+UCTXIE;         //Enable pertinent interrupts.
}


/************************************************************************************************************//**
 * \brief Adds a new I2C device to the bus.
 *
 * \details Given the parameters, which represent the information and settings needed to perform I2C transactions
 * with a particular device, this function registers the new device in the device Array, then returns the
 * communications ID of the new device, or -1 if there is no more room on the bus. The limit of bus size is
 * defined by the number of elements allow
 *
 * \param rAddr		   The 7 Bit Slave Address associated with this device
 * \param BaudDiv  The Baud Rate Divisor that should be used when timing communications with this device.
 * 	\param  *rxPnt     A pointer to where the data received from this device should be stored.
 * \return  commID A value corresponding to this device's index in the Buffer. An integer between -1 and 7
 * \retval -1  The maximum number of devices have been registered.
 *
 ***************************************************************************************************************/

int setupI2CDevice(unsigned char rAddr, unsigned int BaudDiv, unsigned char *rxPnt){

	int commID;  //Return Value
	if ( I2CcommID > 7 ) {   //Check to see if there are still spots left on the bus, defined by how many elements are allowed in the device Array
		return -1;   //Return -1 if no more spots are left on the bus.
	}else{
		I2Cdevice newDevice; //Initialize the new device on the bus if there are still spots.
		newDevice.BaudDiv=BaudDiv; //Save the baud rate divisor.
		newDevice.rAddr=rAddr; //Save the Slave Address
		newDevice.rxPnt=rxPnt; //Assign the device's RX data pointer.
		deviceArray[I2CcommID]=&newDevice; //Put this new device in the device Array
		commID=I2CcommID;  //Record the device ID to return.
		I2CcommID++;  //Increment I2CcommID
		return (commID); //return the communications ID of the device that was just registered.
	}
}

/************************************************************************************************************//**
 * \brief Configures the USCI module to connect to a particular device in the device array.
 *
 * Given an ID number, this function assigns the slave address to the I2CSA register, sets the Baud Divisor,
 * and assigns the RX Pointer for this device to the global RX Pointer.
 *
 * \param commID	   An integer, from 0 to 7 inclusive, that corresponds to the index of the selected I2C device in the device array
 * \return void
 *
 ***************************************************************************************************************/
void changeI2CDevice(int commID){

    unsigned char IER=UCB1IE; //Save the current USCI interrupt enable register

    UCB1IE=0x00; //Disable USCI interrupts

	UCB1CTL1|=UCSWRST; //Enable the USCI SW Reset

	UCB1BRW=deviceArray[commID]->BaudDiv; //Set the Baud Divisor to the value specified for this device

	RXBytes=deviceArray[commID]->rxPnt; //Assign the RX Pointer for this device to the global RX Pointer

	UCB1I2CSA=(deviceArray[commID]->rAddr); //Set the I2CSA register to the slave address specified for this device.

	UCB1CTL1 &= ~UCSWRST; //Disable the SW Reset

	UCB1IE=IER; //Return the interrupt enable register to its previous state
}


/************************************************************************************************************//**
 * \brief Initiates an I2C Read operation.
 *
 * Given a number of bytes to receive, a pointer to a place to store received bytes, and a register address at which to start the read,
 * this function initiates a read operation. It first writes the start register to the bus, then generates a repeated start. Then it triggers an RX interrupt.
 * The ISR handles the rest.
 *
 * \param countByte  An unsigned character that corresponds to the number of bytes in this transmission.
 * \param *RXpnter   An pointer to the location to store the received bytes.
 * \param startRegister   The address of the register that contains the first byte.
 * \return void
 * \retval void
 *
 ***************************************************************************************************************/
void I2CRead(unsigned char countByte, unsigned char *RXpnter, unsigned char startRegister){
	RXBytes=RXpnter; //Assign the device specific RX Pointer to the global RX Pointer.
	byteCountRX=countByte; //Assign the local byte counter value to the global RX byte counter

	unsigned char IER=UCB1IE; //Save the current USCI Interrupt Enable register
	UCB1IE=0x00; //Disable USCI Interrupts

	UCB1CTL1|=UCTR+UCTXSTT;  // Send a Start Command and set the UCTR bit high, corresponding to an I2C Write
	UCB1TXBUF=startRegister;  //Broadcast the start Register onto the bus.
	while(UCB1CTL1 & UCTXSTT); //Wait until the transmission is complete
	UCB1IFG&=~UCTXIFG; //Clear the TX interrupt flag
	UCB1CTL1&=~(UCTR+UCTXSTT); //Remove the UCTR bit to prepare for a read operation and remove the UCTXSTT Bit if it is still there.

	UCB1IE=IER; //Restore the Interrupt Enable register to its previous state.

	UCB1CTL1|=UCTXSTT; // Send a start command, initiating the read and triggering the ISR.
}


/************************************************************************************************************//**
 * \brief Initiates an I2C Write operation.
 *
 * Given a number of bytes to transmit and a pointer to the first element in an array of bytes to be sent, this
 * initiates an I2C Write. When trying to use this, the easiest and quickest way to get it going is to just have an
 * array of bytes to send. The first element of this array MUST be the address of the first register to write on the
 * slave node. This function does little more than assign the parameters to their global counterparts, then trigger
 * the ISR, which handles all of the transmission.
 *
 * \param countByte  An unsigned character that corresponds to the number of bytes in this transmission.
 * \param *pnter   An pointer to the first element of an array of characters that contains the data to be transmitted.
 * This pointer's value must be the address of the register to be written on the slave device.
 * \return void
 *
 ***************************************************************************************************************/
void I2CWrite(unsigned char countByte, unsigned char *pnter){
	TXBytes=pnter; //Assigns the pointer parameter to its global counterpart.
	byteCountTX=countByte; //Assign the number of bytes to be transmitted to the global TX byte counter.
	portStatus=statTX;
	UCB1CTL1|=UCTR+UCTXSTT; //Send Start condition, ISR takes care of the rest.
	while(UCB1CTL1 & UCTXSTT);
}


/************************************************************************************************************//**
 * \brief Check if a device with a particular communications ID is on the bus.
 *
 * Given a communications ID, this function polls the device to check if it is on the bus. It disables interrupts, then
 * sends a start and stop command simultaneously. If a NACK interrupt flag is thrown, the device is not on the bus.
 * This functionality has not been tested, but TI claims this procedure will work.
 *
 * \param commID  The Communications ID number of the device to poll.
 * \return returnVal  An unsigned character that communicates whether the device responded.
 * \retval 0x00  Device not on bus.
 *
 ***************************************************************************************************************/
 unsigned char I2CSlavePresent(int commID){
	unsigned char oldIEReg;
	unsigned char oldSlaveAdd;
	unsigned char returnVal;
	oldIEReg=UCB1IE; //Store Interrupt Enable Settings.
	oldSlaveAdd=UCB1I2CSA; //Store current Slave Address.
	UCB1IE &= 0x00; //Disable USCI Interrupts.
	UCB1I2CSA=(deviceArray[commID]->rAddr); //Put the new Slave Address in UCB1I2CSA.
	UCB1CTL1 |= UCTR + UCTXSTT + UCTXSTP; //Send a special command with a simultaneous Start and Stop. Not sure why. TI said so.
	while(UCB1CTL1 & UCTXSTP); //Wait for transaction to complete.
	if(UCB1STAT & UCNACKIFG){  //Check to see if a NACK occurred.
		returnVal=0x00; //Prepare to return 0x00 if device does not respond.
	}else{
		returnVal=0xFF; //Prepare to return 0xFF if device responds with ACK.
	}

	UCB1IE=oldIEReg; //Restore old Interrupt Enable state
	UCB1I2CSA=oldSlaveAdd; //Restore old Slave Address
	return returnVal;
}


 /************************************************************************************************************//**
  * \brief Check if the bus is currently in use.
  *
  * This function polls the USCI status register, and in particular the BUSY bit, to determine if the bus is currently
  * busy. Returns a 0x00 if USCI port B1 is not being used.
  *
  * \param void
  * \retval 0x00   USCI port is available for use.
  ***************************************************************************************************************/
unsigned char isBusy(){
	return (UCB1STAT & UCBBUSY);

	//0=not busy
	//1=busy
}

#pragma vector =USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void){

	 switch(__even_in_range(UCB1IV,12))
	  {
	  case  0: break;                           // Vector  0: No interrupts

	  case  2: break;                           // Vector  2: ALIFG

	  case  4:                                         // Vector  4: NACKIFG

		  UCB1CTL1 |= UCTXSTP; //Stop
		  UCB1STAT &= ~UCNACKIFG; //Clear Nack flag

		  break;

	  case  6: break;                           // Vector  6: STTIFG

	  case  8: break;                           // Vector  8: STPIFG

	  case 10:                            // Vector 10: RXIFG

		  if(byteCountRX==0x00){
		  			UCB1CTL1 |= UCTXSTP; //Stop
		  			*RXBytes=UCB1RXBUF; //Clear buffer
		  			RXBytes++; //increment RXData pointer
		  	}else{
		  			*RXBytes=UCB1RXBUF; //Clear buffer
		  			RXBytes++; //increment RXData Pointer
		  			byteCountRX--; //decrement global byte counter
		  }

		  	break;

	  case 12:                                   // Vector 12: TXIFG
		  if(byteCountTX==0x00){
		  				if(byteCountRX>0x00){
		  					UCB1IFG&=~UCTXIFG;
		  				}else{
			  	  	  	 UCB1CTL1|=UCTXSTP; //Stop
			  	  	  	 UCB1IFG &= ~UCTXIFG; //Clear TX flag
		  				portStatus=statOpen;
		  				}
		  }else{
		  			if(byteCountRX==0x00){
			  	  	 UCB1TXBUF=*TXBytes; //Load TX buffer


		  		    TXBytes++; //increment TX pointer
		  		    byteCountTX--; //decrement global byte counter

		  			}
		  }

		  break;

	  default: break;

	  }
}
