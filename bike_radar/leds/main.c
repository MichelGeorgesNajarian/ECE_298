#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	P5OUT &= 0x00;
	P5DIR &= 0x00;
	P5DIR |= BIT0;
	P5OUT |= BIT0;
	for(;;){

	}
	
	return 0;
}
