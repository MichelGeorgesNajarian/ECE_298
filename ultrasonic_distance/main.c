#include <msp430.h>
#include <msp430fr4133.h>
//#include <driverlib.h>



/**
 * main.c
 */

/*
 * USDS: echo - 8.2
 *       trigger - 8.3
 *
 * LED: red - 2.5
 *      green - 5.1
 *      yellow - 2.7
 *      orange - 5.0
 *
 * keypad: R1 - 1.7
 *         R2 - 1.1
 *         R3 - 1.6
 *         R4 - 5.3
 *         C1 - 1.3
 *         C2 - 1.4
 *         C3 - 1.5
 * */

// !!VERY IMPORTANT!! setting up golbal variable which will hold the value entered by keypad
volatile int keypad = 0;
volatile int active = 0;

void whichLED(int, int);

void Delayus(unsigned int delayamt)
{
    // Delay is 0-65535 usec; already initialized to use SMCLK, divide by 16 so timer increments once/usec; upon reaching delayamt, one interrupt is given and the delay is over
        TA0CCR0 = delayamt;                     // Interrupt when up-count to this value
        TA0CTL |= (MC_1 | TACLR);
      __bis_SR_register(LPM0_bits | GIE);       // Go to sleep. Timer0 ISR will wake us up when proper time has elapsed
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    /*
     * ------------------------------------------------------------------------------------------------------------------
     *   SETTING UP ULTRASONIC DISTANCE SENSOR
     * ------------------------------------------------------------------------------------------------------------------
     * */
    P8OUT &= 0x00; // setting output to 0
    P8DIR &= 0x00;
    P8DIR |= BIT3; // sensor trig as output 0000 0000 | 0000 1000
                   // P1.6 set as output
    //P1DIR &= ~BIT4; // sensor echo as input
    P8DIR &= ~BIT2; // P1.1 is input for echo

    TA0EX0 = 0x0001; // Divide by 2
    TA0CTL = TASSEL_2 + MC_0 + TACLR + ID_3; // SMCLK, count up to TA0CCR0, Timer clear, Divide by 8
    TA0CCTL0 = CCIE; // TACCR0 interrupt enabled but no effect until Timer0 is running
    /*
     * --------------------------------------------------------------------------------------------------------------------
     *  SETTING UP LEDS
     * --------------------------------------------------------------------------------------------------------------------
     * */
    P5DIR &= 0x00; // orange led and green led
    P5DIR |= BIT0; // set as output orange led
    P5DIR |= BIT1; // set as output green led
    P5OUT &= ~(BIT0 | BIT1); // turn off
    P2DIR &= 0x00; // yellow led and red led
    P2DIR |= BIT7; // set yellow led as output
    P2DIR |= BIT5; // set red led as output
    P2OUT &= ~(BIT5 | BIT7); // turn off

    /*
    * --------------------------------------------------------------------------------------------------------------------
    *  SETTING UP KEYPAD
    * --------------------------------------------------------------------------------------------------------------------
    * */
    P1DIR &= 0x00;
    /* COLUMNS */
    P1DIR &= ~BIT3; // set col1 as input
    P1DIR &= ~BIT4; // set col2 as input
    P1DIR &= ~BIT5; // set col3 as input

    P1IES |= BIT3; // high to low transition
    P1IES |= BIT4; // high to low transition
    P1IES |= BIT5; // high to low transition
    // enabling pull-up resistor for columns
    P1REN |= BIT3; // resistor for col1
    P1REN |= BIT4; // resistor for col2
    P1REN |= BIT5; // resistor for col3
    // setting the resistors as high in order to achieve pull-up resistor
    P1OUT |= (BIT3 | BIT4 | BIT5); //output as high 1

    // enabling interrupts for columns
    P1IE |= BIT3; // interrupt for col1
    P1IE |= BIT4; // interrupt for col2
    P1IE |= BIT5; // interrupt for col3

    P1IFG &= ~(BIT3 | BIT4 | BIT5); // clearing any interrupt flags for col1, col2, col3

    /* ROWS */
    P1DIR |= BIT7; // set r1 as output
    P1DIR |= BIT1; // set r2 as output
    P1DIR |= BIT6; // set r3 as output
    P5DIR |= BIT3; // set r4 as output

    P1OUT &= ~(BIT7 | BIT1 | BIT6); // setting r1, r2, r3 as low
    P5OUT &= ~BIT3; // setting r4 as low
    __bis_SR_register(GIE); // Enable all interrupts

    // Set up Timer_A1: SMCLK clock, input divider=1,
    // "continuous" mode. It counts from 0 to 65535,
    // incrementing once per clock cycle (i.e. every 1us).
    TA1CTL = TASSEL_2 + ID_0 + MC_2;
    int echo_duration; // in us
    int distance;

    while (1){ //infinite loop
        P8OUT |= BIT3; //p1.6 is high for 10 microseconds (trigger pulse)
        __delay_cycles(5);           // 20us delay
        P8OUT &= ~BIT3;
      while ((P8IN & BIT2) == 0x00);
        TA1R = 0;
        while ((P8IN & BIT2) > 0x00);
        echo_duration = TA1R;
        distance = echo_duration*0.017;

    }
}

void whichLED(int distance, int max){
    if(active == 0x00) {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        return;
    }
    if (distance >= 3*max/4) { // turn on green led
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P5OUT |= BIT1; // turn on green
    } else if (distance >= max/2) {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P2OUT |= BIT7; // turn on yellow led
    } else if (distance >= max/4) {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P5OUT |= BIT0; // turn on orange led
    } else {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P2OUT |= BIT5; // turn on red led
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    TA0CTL |= MC_0;                             // Stop Timer0
    __bic_SR_register_on_exit(LPM0_bits);       // Wakeup from LPM0 and continue after the Delayus function
}

//       P1OUT &= ~(BIT7 | BIT1 | BIT6); //r1, r2, r3
//       P5OUT &= ~BIT3; // r4
#pragma vector=PORT1_VECTOR
__interrupt void port1_ISR(void) {
    int column = 0;
    int columnBIT;
    if (P1IFG & BIT3) {
        column = 1;
        columnBIT = BIT3;
    } else if (P1IFG & BIT4) {
        column = 2;
        columnBIT = BIT4;
    } else if (P1FG & BIT5) {
        column = 3;
        columnBIT = BIT5;
    }
    int row = 0;
    //setting up r1 as 0 and everything else as 1
    P1OUT |= (BIT7 | BIT1 | BIT6);
    P5OUT |= BIT3;
    P1OUT &= ~BIT7; // setting r1 as low
    if ((P1IN & BIT7) == 0x00) {
        row = 1;
    }
    P1OUT &= ~(BIT7 | BIT1 | BIT6); // re-setting everything as low again
    P5OUT &= ~BIT3;
    if (!row) {
        P1OUT |= (BIT7 | BIT1 | BIT6);
        P5OUT |= BIT3;
        P1OUT &= ~BIT1; // setting r1 as low
        if ((P1IN & BIT1) == 0x00) {
            row = 2;
        }
        P1OUT &= ~(BIT7 | BIT1 | BIT6); // re-setting everything as low again
        P5OUT &= ~BIT3;
        if (!row) {
            P1OUT |= (BIT7 | BIT1 | BIT6);
            P5OUT |= BIT3;
            P1OUT &= ~BIT1; // setting r1 as low
            if ((P1IN & BIT6) == 0x00) {
                row = 3;
            }
            P1OUT &= ~(BIT7 | BIT1 | BIT6); // re-setting everything as low again
            P5OUT &= ~BIT3;
            if (!row) {
                P1OUT |= (BIT7 | BIT1 | BIT6);
                P5OUT |= BIT3;
                P1OUT &= ~BIT1; // setting r1 as low
                if ((P5IN & BIT3) == 0x00) {
                    row = 4;
                }
                P1OUT &= ~(BIT7 | BIT1 | BIT6); // re-setting everything as low again
                P5OUT &= ~BIT3;
            }
        }
    }

    switch(1)
    {
        case(column == 1 && row == 1):
                    keypad = keypad*10+1;
        case(column == 1 && row == 2):
                    keypad = keypad*10+4;
        case(column == 1 && row == 3):
                    keypad = keypad*10+7;
        case(column == 1 && row == 4):
                    keypad = 0;
                    active = 0;
        case(column == 2 && row == 1):
                    keypad = keypad*10+2;
        case(column == 2 && row == 2):
                    keypad = keypad*10+5;
        case(column == 2 && row == 3):
                    keypad = keypad*10+8;
        case(column == 2 && row == 4):
                    keypad = keypad*10;
        case(column == 3 && row == 1):
                    keypad = keypad*10+3;
        case(column == 3 && row == 2):
                    keypad = keypad*10+6;
        case(column == 3 && row == 3):
                    keypad = keypad*10+9;
        case(column == 3 && row == 4):
                    active = 1;
    }

    keypad = (keypad > 400) ? 400 : keypad; // if max value more than 4m, update max range to 4m
    active = (keypad >= 2 && active == 1) ? 1 : 0; // if value entered is less than 2cm, then do NOT active leds yet.
    P1IFG &= ~columnBIT;
    __bic_SR_register_on_exit(LPM0_bits);
}
