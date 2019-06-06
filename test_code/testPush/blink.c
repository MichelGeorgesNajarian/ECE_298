//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#include <msp430.h>

volatile int active = 1;

void whichLED(int, int);

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    /*------------------------------------------- Setting up onboard LED and push button -------------------------------------*/
    P1DIR &= 0x00;
    P1DIR |= BIT0;                          // Set P1.0 to output direction
    P1IES |= BIT2;                           //high to low transition
    P4DIR |= BIT0;
    P1REN |= BIT2; // Resistor for P1.2
    P1OUT = BIT2; // Resistor pulls up
    P4OUT = BIT0;
    P1IE |= BIT2;                           // Enabling interrupts for P1.2
    P1IFG &= ~BIT2;                         // Clearing interrupt flag for P1.2

    /*--------------------------------------------- Setting up breadboard LEDs ------------------------------------------------*/
    P5DIR &= 0x00; // orange led
    P5DIR |= BIT0; // set as output
    P5DIR |= BIT1;
    P5OUT |= BIT0; //turn off
    P5OUT |= BIT1;
    P2DIR &= 0x00;
    P2DIR |= BIT7;
    P2DIR |= BIT5; // set as output
    P2OUT |= BIT7;
    P2OUT |= BIT5; //turn off

    /*--------------------------------------------- Setting up Keypad ----------------------------------------------------------*/
    P1REN |= (BIT7); // Resistors for P8.0...3
    P1OUT |= (BIT7);
    P1IE |= BIT7; // enabling only the P8.3 interrupt once in interrupt, figure out which column is one as well
    P1IES |= (BIT7); // high to low transition
    P1IFG &= ~(BIT7+BIT2); // clearing up the interrupt flag

//-----------------------------------------LEDS------------------------------------
    //P5.1 GREEN
    //P5.0 ORANGE
    //P2.7 YELLOW
    //P2.5 RED

 //---------------------------------------USDS---------------------------------------
    //P1.4 Trigger
    //P1.1 Echo

//---------------------------------------Keypad---------------------------------------

    //P1.3 COL 1
    //P1.4 COL 2
    //P1.5 COL 3
    //P1.7 ROW 1

    __bis_SR_register(GIE);          // Enter LPM0 w/ interrupt

    int j = 0;
    volatile int active = 1;

    for(;;) {
        volatile unsigned int i;            // volatile to prevent optimization

        i = 10000;                          // SW Delay
        j %= 4;
        whichLED(j, active);
        j++;
        do i--;
        while(i != 0);
    }



}

#pragma vector=PORT1_VECTOR
__interrupt void port1_ISR(void) {
  /*while(1) {
    switch(_even_in_range(P1IV,BIT2)) { // check for highest pending interrupt and clear its IFG bit
      case 0: return; // no more pending interrupts
      case BIT0: // P1IFG.3
        P1OUT ^= BIT0;
        break;
      case BIT2: //P1IFG.0
        P4OUT ^= BIT0;
        break;
    }
  }*/
    if(P1IFG & BIT2) {
        P1OUT ^= BIT0;
        P1IFG &= ~BIT2;
        active ^= 1;
    }
    if(P1IFG & BIT7) {
        P4OUT ^= BIT0;
        P1IFG &= ~BIT7;
    }
    __bic_SR_register_on_exit(LPM0_bits);
}
//    __bic_SR_register_on_exit(LPM0_bits);
//}

    void whichLED(int j, int active){
        if (j == 0 && active) { // turn on green led
            P5OUT &= 0x00; // turn off red
            P2OUT &= 0x00; // turn off orange
            P5OUT |= BIT1; // turn on green
        } else if (j == 1 && active) {
            P5OUT &= 0x00; // turn off orange and green
            P2OUT &= 0x00; // turn off red and yellow
            P2OUT |= BIT7; // turn on yellow led
//        } else if (j == 3 && active) {
//            P5OUT &= 0x00; // turn off red
//            P2OUT &= 0x00; // turn off orange
//            P5OUT |= BIT0; // turn on orange led
        } else if (active) {
            P5OUT &= 0x00; // turn off red
            P2OUT &= 0x00; // turn off orange
            P2OUT |= BIT5; // turn on red led
        }
    }
