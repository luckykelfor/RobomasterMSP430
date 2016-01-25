#include "msp430g2553.h"

/*Predefine for UART communication*/
#define TXLED BIT0
#define RXLED BIT6
#define TXD BIT2
#define RXD BIT1
const char string[] = { "Hello World\n" };
unsigned int i; //Counter

/*Predefine for PWM output*/
#define uint unsigned int
#define STE 180
#define MIN 500
#define MAX 2400
#define PWM_P 20000
const uint stepval=(MAX-MIN)/STE;
uint deg[200]; // about 180 degrees;

/*LUT for map of degree to duty of pwm ouput*/
void ang_con()
{
    uint i,degnow=MIN;
    for(i = 200;i > 0;i--)
    {
        deg[200-i] = degnow;
        degnow += stepval;
    }
}


void clk_init()
{


    DCOCTL = 0; // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ; // Set DCO
    DCOCTL = CALDCO_1MHZ;

}

void uart_init()
{
	  P2DIR |= 0xFF; // All P2.x outputs
	  P2OUT &= 0x00; // All P2.x reset
	  P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	  P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	  P1DIR |= RXLED + TXLED;
	  P1OUT &= 0x00;
	  UCA0CTL1 |= UCSSEL_2; // SMCLK
	  UCA0BR0 = 0x08; // 1MHz 115200
	  UCA0BR1 = 0x00; // 1MHz 115200
	  UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5
	  UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
	  UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
	  _EINT();

}

void pwm_init()
{
    P2DIR |= BIT1 + BIT4;
    P2SEL |= BIT1 + BIT4;
    TA1CCR0  = PWM_P;   // PWM frequency = PWM_PCLK_Hz / PWM_P ;
    TA1CCTL1 = OUTMOD_7;  //up mode
    TA1CCR1  = deg[0];   //start from 0 degree;
    TA1CCTL2 = OUTMOD_7;
    TA1CCR2  = deg[100];  //start from about 90 degrees;
    TA1CTL   = TASSEL_2 + TACLR + MC_1;//Use SMCLK, reset/set
}

void move_test()
{/*
    uint i;
    for(i=180;i>0;i--)
    {
        TA1CCR1 = deg[180-i];
        __delay_cycles(100000);
    }
    TA1CCR1 = deg[180-i];
    __delay_cycles(1000000);
    for(i=100;i>30;i--)
    {
        TA1CCR2 = deg[i];
        __delay_cycles(100000);
    }
    TA1CCR2 = deg[100];
    __delay_cycles(1000000);
    */
	//转动角度设置 while(1){


	//FOR test of PWM output 2.1(output 2.4 should be similar)
	// Go to 0°；+duty 3.0%。
	TACCR1 = deg[0];
	 __delay_cycles(1000000);
	// Go to 45°；+duty 4.8%。
	TACCR1 = deg[45];
	 __delay_cycles(1000000);
	// Go to 90°; +duty 7.4%.
	TACCR1 = deg[90];
	__delay_cycles(1000000);
	// Go to 135°; +duty 9.8%.
	TACCR1 = deg[146];
	 __delay_cycles(1000000);
	// Go to 180°; +duty 12.3%.
	TACCR1 = deg[190];
	__delay_cycles(1000000);
	// Go to 0°；+duty 3.0%。
	TACCR1 = deg[0];
	__delay_cycles(1000000);


}

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT

	uart_init();

	ang_con();

	pwm_init();

	while(1)
	{

		move_test();

	}

}






/*Interrupt service for uart*/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  P1OUT |= RXLED;

  //TODO : find the start code of Moving the handles

  if (UCA0RXBUF == 'a') // 'a' received?
  {
    i = 0;
    UC0IE |= UCA0TXIE;  // Enable USCI_A0 TX interrupt
    UCA0TXBUF = string[i++];
  }


  P1OUT &= ~RXLED;
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  P1OUT |= TXLED;
  UCA0TXBUF = string[i++]; // TX next character
  if (i == sizeof(string) - 1) // TX over?
  {
    UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
  }
  P1OUT &= ~TXLED;
}
