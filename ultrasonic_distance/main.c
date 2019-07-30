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
 *         R2 - 1.0
 *         R3 - 1.6
 *         R4 - 5.3
 *         C1 - 1.3
 *         C2 - 1.4
 *         C3 - 1.5
 * */

// !!VERY IMPORTANT!! setting up golbal variable which will hold the value entered by keypad
volatile int keypad = 0;
volatile int active = 0;

void whichLED(float *, float *, float *, int);

void Delayus(unsigned int delayamt)
{
    // Delay is 0-65535 usec; already initialized to use SMCLK, divide by 16 so timer increments once/usec; upon reaching delayamt, one interrupt is given and the delay is over
    TA0CCR0 = delayamt;                 // Interrupt when up-count to this value
    TA0CTL |= (MC_1 | TACLR);
    __bis_SR_register(LPM0_bits | GIE); // Go to sleep. Timer0 ISR will wake us up when proper time has elapsed
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
    /*
     * ------------------------------------------------------------------------------------------------------------------
     *   SETTING UP ULTRASONIC DISTANCE SENSOR
     * ------------------------------------------------------------------------------------------------------------------
     * */
    //P8OUT &= 0x00; // setting output to 0
    P8DIR &= 0x00;
    P8DIR |= BIT3; // sensor trig as output 0000 0000 | 0000 1000
    P8OUT &= ~BIT3;
                   // P1.6 set as output
    //P1DIR &= ~BIT4; // sensor echo as input
    P8DIR &= ~BIT2; // P1.1 is input for echo
    P8OUT |= BIT2;
    P8REN |= BIT2;

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
    P1IN = 0x00;
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
    //P1OUT |= (BIT3 | BIT4 | BIT5); //output as high 1

    // enabling interrupts for columns
    P1IE |= BIT3; // interrupt for col1
    P1IE |= BIT4; // interrupt for col2
    P1IE |= BIT5; // interrupt for col3

    P1IFG = 0x00; // clearing any interrupt flags for col1, col2, col3

    /* ROWS */
    P1DIR |= BIT7; // set r1 as output
    P1DIR |= BIT0; // set r2 as output
    P1DIR |= BIT6; // set r3 as output
    P5DIR |= BIT3; // set r4 as output

    P1OUT &= ~(BIT7 | BIT0 | BIT6); // setting r1, r2, r3 as low
    P5OUT &= ~BIT3; // setting r4 as low
    __bis_SR_register(GIE); // Enable all interrupts

    // Set up Timer_A1: SMCLK clock, input divider=1,
    // "continuous" mode. It counts from 0 to 65535,
    // incrementing once per clock cycle (i.e. every 1us).
    TA1CTL = TASSEL_2 + ID_0 + MC_2;
    int echo_duration; // in usfloat distance = 0;
    float distance = 0.0;
    float distance_old_1 = 0.0;
    float distance_old_2 = 0.0;

    while (1)
    { //infinite loop
        P8OUT |= BIT3; //p1.6 is high for 10 microseconds (trigger pulse)
        __delay_cycles(5);           // 10us delay
        P8OUT &= ~BIT3;
        //__delay_cycles(20);
        while (!(P8IN & BIT2));
        TA1R = 0;
        while ((P8IN & BIT2) > 0x00);
        echo_duration = TA1R;
        distance = echo_duration * 0.017;
        //distance = (distance+1 >600) ? 0 : distance+1;
        //__delay_cycles(50000);
        whichLED(&distance, &distance_old_1, &distance_old_2, keypad);
        __delay_cycles(250);
    }
}

void whichLED(float *distance, float * distance_old_1, float *distance_old_2, int max)
{
    if (active == 0x00)
    {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        return;
    }
    if (*distance_old_1 != 0 && *distance_old_2 == 0) {
        *distance = (*distance + *distance_old_1)/2;
        *distance_old_2 = *distance_old_1;
        *distance_old_1 = *distance;
    } else if (*distance_old_1 != 0 && *distance_old_2 != 0) {
        *distance = (((*distance - *distance_old_2)/6)*(*distance_old_2 + 4* (*distance_old_1) + *distance));
    }
    if (*distance >= 3 * max / 4)
    { // turn on green led
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P5OUT |= BIT1; // turn on green
    }
    else if (*distance >= max / 2)
    {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P2OUT |= BIT7; // turn on yellow led
    }
    else if (*distance >= max / 4)
    {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P5OUT |= BIT0; // turn on orange led
    }
    else
    {
        P5OUT &= ~(BIT1 | BIT0); // turn off green and orange
        P2OUT &= 0x00; // turn off yellow and red
        P2OUT |= BIT5; // turn on red led
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    TA0CTL |= MC_0;                             // Stop Timer0
    __bic_SR_register_on_exit(LPM0_bits); // Wakeup from LPM0 and continue after the Delayus function
}

//       P1OUT &= ~(BIT7 | BIT1 | BIT6); //r1, r2, r3
//       P5OUT &= ~BIT3; // r4
#pragma vector=PORT1_VECTOR
__interrupt void port1_ISR(void)
{
    P1OUT |= BIT7; // setting row 1 as high
    P1OUT |= BIT1; // setting row 2 as high
    P1OUT |= BIT6; // setting row 3 as high
    P5OUT |= BIT3; // setting row 4 as high

    P1OUT &= ~BIT7; //setting row 1 as low
    if ((P1IN & BIT3) == 0x00)
    {
        keypad = keypad * 10 + 1;
    }
    else if ((P1IN & BIT4) == 0x00)
    {
        keypad = keypad * 10 + 2;
    }
    else if ((P1IN & BIT5) == 0x00)
    {
        keypad = keypad * 10 + 3;
    }
    P1OUT |= BIT7; // setting row 1 as high
    P1OUT &= ~BIT0; // settigng row 2 as low

    if ((P1IN & BIT3) == 0x00)
    {
        keypad = (keypad - 1) + 4;
    }
    else if ((P1IN & BIT4) == 0x00)
    {
        keypad = keypad - 2 + 5;
    }
    else if ((P1IN & BIT5) == 0x00)
    {
        keypad = keypad - 3 + 6;
    }

    P1OUT |= BIT0; // setting row 2 as high
    P1OUT &= ~BIT6; // setting row 3 as low

    if ((P1IN & BIT3) == 0x00)
    {
        keypad = keypad * 10 + 7;
    }
    else if ((P1IN & BIT4) == 0x00)
    {
        keypad = keypad * 10 + 8;
    }
    else if ((P1IN & BIT5) == 0x00)
    {
        keypad = keypad * 10 + 9;
    }

    P1OUT |= BIT6; // setting row 3 as high
    P5OUT &= ~BIT3; // setting row 4 as low

    if ((P1IN & BIT3) == 0x00)
    {
        keypad = 0;
        active = 0;
    }
    else if ((P1IN & BIT4) == 0x00)
    {
        keypad = keypad * 10;
    }
    else if ((P1IN & BIT5) == 0x00)
    {
        active = 1;
    }

    P1OUT &= ~(BIT7 | BIT0 | BIT6); // setting row 1,2,3 as low

    keypad = ((keypad > 400 && active == 1) || keypad < 0 ) ? 400 : keypad; // if max value more than 4m, update max range to 4m
    active = (keypad >= 2 && active == 1) ? 1 : 0; // if value entered is less than 2cm, then do NOT active leds yet.
    P1IFG = 0x00;
    __bic_SR_register_on_exit(LPM0_bits);
}
