#include "msp430g2553.h"
#include<stdio.h>
/*Predefine for UART communication*/
#define TXLED BIT0
#define RXLED BIT6
#define TXD BIT2
#define RXD BIT1
const char string[] = { "Hello World\n" };

int start_flag =0,prepare_flag =0, go_flag = 0;
/*Predefine for PWM output*/
#define uint unsigned int
//#define STE 180
#define MIN 500
#define MAX 2500
#define DEGREE 180
#define PWM_P 20000
const uint stepval=(MAX-MIN)/DEGREE;
int deg[DEGREE]; // about 180 degrees;
enum {handleSeize,handleRelease,handleDown,handleUp};
int handleOption = 100;


/*LUT for map of degree to duty of pwm ouput*/
void ang_con()
{
    uint i,degnow=MIN;
    for(i = DEGREE;i > 0 ;i--)
    {
        deg[DEGREE-i] = degnow;
        degnow += stepval;
    }
}


void clk_init()
{


    DCOCTL = 0; // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_8MHZ; // Set DCO
    DCOCTL = CALDCO_8MHZ;
    BCSCTL2 = DIVS_3;

}

void uart_init()
{
	  P2DIR |= 0xFF; // All P2.x outputs
	  P2OUT &= 0x00; // All P2.x reset
	  P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	  P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
	  P1DIR |= RXLED + TXLED;
	  P1OUT   = 0x01;
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
    TA1CCR2  = deg[0];  //start from about 0 degree;
    TA1CTL   = TASSEL_2 + TACLR + MC_1;//Use SMCLK, reset/set
}

void move_motor(int motorNo, unsigned int degreeby)
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

//TODO: Find the relations between the degree and the duty of pwm.
	//FOR test of PWM output 2.1(output 2.4 should be similar)
	// Go to 0°；+duty 3.0%。

	switch(motorNo)
	{
	case 0://Motor1
		TA1CCR1 = deg[degreeby];
		// __delay_cycles(1000000); //maybe we should comment this line for
		 break;
	case 1: //Motor2
		TA1CCR2 = deg[degreeby];
		 //__delay_cycles(1000000);
		 break;
	default:
		break;

	}
	/*
	TACCR1 = deg[0];
	 __delay_cycles(1000000);
	// Go to 45°；+duty 4.8%。
	TACCR1 = deg[44];
	 __delay_cycles(1000000);
	// Go to 90°; +duty 7.4%.
	TACCR1 = deg[89];
	__delay_cycles(1000000);
	// Go to 135°; +duty 9.8%.
	TACCR1 = deg[134];
	 __delay_cycles(1000000);
	// Go to 180°; +duty 12.3%.
	TACCR1 = deg[179];
	__delay_cycles(1000000);
	// Go to 0°；+duty 3.0%。
	TACCR1 = deg[0];
	__delay_cycles(1000000);


	*/
}

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // Stop WDT

	uart_init();

	ang_con();

	pwm_init();

	while(1)
	{

		//move_test();
		switch(handleOption)
		{
		case handleSeize:
			//TODO: output pwm
			P1OUT ^= BIT6;
			move_motor(0,145);
			handleOption = -1;
			break;
		case handleRelease:
			//TODO: output pwm

			P1OUT ^= BIT0;
			move_motor(0,0);
			handleOption = -1;
			break;
		case handleDown:
			move_motor(1,90);
			handleOption = -1;
			break;
		case handleUp:
			move_motor(1,0);
			handleOption = -1;
			break;
		default:// DO nothing
			break;

		}

	}

}






/*Interrupt service for uart*/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
//  P1OUT |= RXLED;
	static int i_Codec = 0;
  //TODO : find the start code of Moving the handles

  if (UCA0RXBUF == 0xAA) // Starter code  received?
  {

    i_Codec = 0;
    start_flag = 1;
  //  UC0IE |= UCA0TXIE;  // Enable USCI_A0 TX interrupt
 //   UCA0TXBUF = string[i++];
   // i_Codec ++ ;

  }
  else if(start_flag == 1 && i_Codec == 12)// Use else if NOT just 'if'! Otherwise You will lose frames.
  {

	  if(UCA0RXBUF == 0x02)//command set should be 0x02
		  prepare_flag = 1;

  }
  else if(start_flag == 1 && prepare_flag ==1 && i_Codec == 13)
  {
	  if(UCA0RXBUF == 0x02)// Transparent data transmit command
		  go_flag = 1;
  }
  else if(start_flag == 1 && prepare_flag ==1 && go_flag ==1 && i_Codec == 14)
  {
	  switch(UCA0RXBUF)  //transmitted data;
	  {
	  case 'O': //TODO: Enable HandleOpetating, handle-grabbing 0x4f
		  handleOption = handleSeize ;// = 1;
		  break;
	  case 'S'://TODO: release the Handle 0x43
		 // handleRelease = 1;
		  handleOption = handleRelease;
		  break;
	  case 'D':// handledown operation
		  handleOption = handleDown;
		  break;
	  case 'U'://handleup operation
		  handleOption = handleUp;
		  break;
	  default:
		  break;

	  }
	  start_flag = 0;
	  prepare_flag = 0;
	  go_flag = 0;
  }


 // P1OUT &= ~RXLED;
  i_Codec ++ ;
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  //P1OUT |= TXLED;
//  UCA0TXBUF = string[i++]; // TX next character
//  if (i == sizeof(string) - 1) // TX over?
//  {
//    UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
 // }
  //P1OUT &= ~TXLED;
}
