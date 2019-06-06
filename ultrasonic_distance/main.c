#include <msp430.h>
#include <msp430fr4133.h>
//#include <driverlib.h>



/**
 * main.c
 */

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

    P1OUT &= 0x00; // setting output to 0
    P1DIR &= 0x00;
    P1DIR |= BIT6; // sensor trig as output 0000 0000 | 0000 1000
                   // P1.6 set as output
    //P1DIR &= ~BIT4; // sensor echo as input
    P1DIR &= ~BIT1; // P1.1 is input for echo

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

    TA0EX0 = 0x0001; // Divide by 2
    TA0CTL = TASSEL_2 + MC_0 + TACLR + ID_3; // SMCLK, count up to TA0CCR0, Timer clear, Divide by 8
    TA0CCTL0 = CCIE; // TACCR0 interrupt enabled but no effect until Timer0 is running
    __bis_SR_register(GIE); // Enable all interrupts

    // Set up Timer_A1: SMCLK clock, input divider=1,
    // "continuous" mode. It counts from 0 to 65535,
    // incrementing once per clock cycle (i.e. every 1us).
    TA1CTL = TASSEL_2 + ID_0 + MC_2;
    int echo_duration; // in us
    int distance;

    while (1){ //infinite loop
        P1OUT |= BIT6; //p1.6 is high for 10 microseconds (trigger pulse)
        __delay_cycles(10);           // 20us delay
        P1OUT &= ~BIT6;
        __delay_cycles(5000);
//        while ((P1IN & BIT1) == 0x00);
//        TA1R = 0;
//        while ((P1IN & BIT2) > 0x00);
//        echo_duration = TA1R;
//        distance = echo_duration;
//        if (distance > 10) {
//            P5OUT &= 0x00; // turn off red
//                        P2OUT &= 0x00; // turn off orange
//                        P5OUT |= BIT1; // turn on green
//        } else {
//            P5OUT &= 0x00; // turn off red
//                        P2OUT &= 0x00; // turn off orange
//                        P2OUT |= BIT5; // turn on red led
//        }
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    TA0CTL |= MC_0;                             // Stop Timer0
    __bic_SR_register_on_exit(LPM0_bits);       // Wakeup from LPM0 and continue after the Delayus function
}
