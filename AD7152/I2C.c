/*
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
 *Acknowledgements to:
 *
 *Uli Kretzschmar
 *Texas Instruments Deutschland GmbH
 *November 2007
 *
 *
 * Bill Devine
 * 8 July 2013
 *
 */


#include <msp430f5342.h>
#include "I2C.h"







unsigned char byteCountRX, byteCountTX;
 unsigned char *RXBytes;
unsigned char *TXBytes;
 int I2CcommID=0;
 I2Cdevice *deviceArray[8];






void initializeB0I2C(void);
int setupI2CDevice(unsigned char rAddr, unsigned int BaudDiv, unsigned char *rxPnt );
void changeI2CDevice(int commID);
void UCB0I2CRead(unsigned char countByte, unsigned char *RXpnter, unsigned char startRegister);
void UCB0I2CWrite(unsigned char countByte, unsigned char *pnter);
unsigned char UCB0I2CSlavePresent(int commID);
unsigned char isBusy(void);


void initializeB0I2C(void){
	  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	  byteCountTX=0x00;
	  byteCountRX=0x00;
	  P4SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	  UCB1CTL1 |= UCSWRST;                      // Enable SW reset
	  UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
	  UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
	  UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	  I2CcommID=0;
	  UCB1IE=UCNACKIE+UCRXIE+UCTXIE;                 //interrupts


}

int setupI2CDevice(unsigned char rAddr, unsigned int BaudDiv, unsigned char *rxPnt){


	if ( I2CcommID > 7 ) {
		return -1;
	}else{
		I2Cdevice newDevice;
		newDevice.BaudDiv=BaudDiv;
		newDevice.rAddr=rAddr;
		newDevice.rxPnt=rxPnt;
		deviceArray[I2CcommID]=&newDevice;
		I2CcommID++;
	}

	return (I2CcommID-1);
}

void changeI2CDevice(int commID){

    __disable_interrupt();

    unsigned char IER=UCB1IE;

	UCB1CTL1|=UCSWRST;

	UCB1BRW=deviceArray[commID]->BaudDiv;

	RXBytes=deviceArray[commID]->rxPnt;

	UCB1I2CSA=(deviceArray[commID]->rAddr);

	UCB1CTL1 &= ~UCSWRST;

	__enable_interrupt();
	UCB1IE=IER;
}



void UCB0I2CRead(unsigned char countByte, unsigned char *RXpnter, unsigned char startRegister){
	RXBytes=RXpnter; //assign the local pointer (pnter) to the global pointer for recieving data, instantiated above
	byteCountRX=countByte;
	unsigned char IER=UCB1IE;

	_disable_interrupt();
	//First we have to write the start Register and the Slave Address
	UCB1CTL1|=UCTR+UCTXSTT;                          // to the Bus. This section does so without transmitting a STOP
	              //condition, preparing us for a repeated START condition
	UCB1TXBUF=startRegister;
	while(UCB1CTL1 & UCTXSTT);
	UCB1IFG&=~UCTXIFG;
	UCB1CTL1&=~(UCTR+UCTXSTT);
	__enable_interrupt();


	/* This section below triggers the RX ISR	 */


		UCB1CTL1|=UCTXSTT; //set start condition if it does not already exist, ISR handles the rest


}

void UCB0I2CWrite(unsigned char countByte, unsigned char *pnter){
	//ensure pnter initially points  to a value corresponding to the RegAdd that you want to start writing.

	TXBytes=pnter; //Assign the array of bytes from pnter to the global pointer instantiated above
	byteCountTX=countByte; //Assign the number of bytes to be transmitted to the global byte counter, instantiated above

	UCB1CTL1|=UCTR+UCTXSTT; //Send Start condition, ISR takes care of the rest
	return;
}



 unsigned char UCB0I2CSlavePresent(int commID){
	unsigned char oldIEReg, oldSlaveAdd, returnVal;
	oldIEReg=UCB1IE; //Store Interrupt Enable Settings
	oldSlaveAdd=UCB1I2CSA; //Store current Slave Address
	UCB1IE &= ~UCNACKIE; //Disable Nack Interrupt;
	UCB1I2CSA=(deviceArray[commID]->rAddr); //Put the new Slave Address
	UCB1IE &=~(UCTXIE+UCRXIE); //Remove the TX and RX interrupts
	_disable_interrupts(); //generally disable interrupts
	UCB1CTL1 |= UCTR + UCTXSTT + UCTXSTP; //Start I2C Transmit
	while(UCB1CTL1 & UCTXSTP); //Wait for Stop
	returnVal=!(UCB1STAT & UCNACKIFG); //Is there a Nack? if so, it DNE
	_enable_interrupts();  //Restore interrupts
	UCB1IE=oldIEReg; //Restore old Interrupt Settings
	UCB1I2CSA=oldSlaveAdd; //Restore old Slave Address
	return returnVal;
	//1 ->found
	//0 -> DNE
}



unsigned char isBusy(){
	return (UCB1STAT &UCBBUSY);

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
		  				}
		  }else{
		  			if(byteCountRX==0x00){
			  	  	 UCB1TXBUF=*TXBytes; //Load TX buffer
		  			while(UCB1CTL1 & UCTXSTT);

		  		    TXBytes++; //increment TX pointer
		  		    byteCountTX--; //decrement global byte counter
		  		    UCB1IFG&=~UCTXIFG;
		  			}else{
		  				UCB1IFG&=~UCTXIFG;
		  			}
		  }

		  break;

	  default: break;

	  }
}





