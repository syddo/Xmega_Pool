/*
 * Clock_HourMinSec.c
 *
 * Created: 2/3/2017 3:22:59 PM
 * Author : Syd Pao
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 2000000

#define true  1
#define false 0

typedef struct{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
}time;

typedef uint8_t bool;

volatile time t;
volatile static bool tick = false;

/*
 *	ISR to update the time - tick per second.
 */
ISR( RTC_OVF_vect )
{
	tick = true;
}

/**
 * \brief       sets PORTx as output
 * 
 * \param port  reference to the port
 * 
 * \return void
 */
void configure_port(volatile PORT_t *port)
{
	port->OUT |= ~(0x00); // inverted for STK600
	port->DIR |= 0xFF;
}

/**
 * \brief        just enable all interrupts
 * 
 * \param        NA
 * 
 * \return       void
 */
void just_enable_interrupts(void)
{
	PMIC.CTRL |= PMIC_HILVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_LOLVLEX_bm;
}

/**
 * \brief                initialize the RTC module to count time in seconds
 * 
 * \param RTC_CLK_source limited only to XOSC32kHz, RC 32kHz and ULP 32kHz
 * \param RTC_CLK_Presc  option to prescale clock to 1.024Hz
 * \param RTC_Period     period of the counter
 * 
 * \return void
 */
void Rtc_Init(CLK_RTCSRC_t RTC_CLK_source, RTC_PRESCALER_t RTC_CLK_Presc, uint16_t RTC_Period)
{
	OSC.CTRL |= RTC_CLK_source;               // select RTC Clock source
	while( !(OSC.STATUS & RTC_CLK_source) );  // wait for osc to stabilize
	
	PR.PRGEN &= ~PR_RTC_bm;                   // enable power to RTC module
	CLK.RTCCTRL = CLK_RTCEN_bm;
	RTC.CTRL = 0;                             // disable RTC to start configuration
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
	
	RTC.CNT = 0;
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.PER  = RTC_Period;
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.INTCTRL  = RTC_OVFINTLVL_MED_gc;      // Enable overflow interrupt
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.INTFLAGS = RTC_OVFIF_bm;
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
	RTC.CTRL     = ( RTC.CTRL & ~RTC_PRESCALER_gm) | RTC_CLK_Presc;
	while(RTC.STATUS & RTC_SYNCBUSY_bm);
}

/**
 * \brief        keeps track of time elapsed in seconds, min, hours
 *               timer resets after 24 hours.
 * 
 * \return void
 */
void update_time()
{
	t.second +=1;
	
	if( t.second >= 60 ){
		t.second -= 60;
		if(++t.minute == 60){
			t.minute = 0;
			if(++t.hour == 24){
				t.hour =0; //reset counter after 24 hrs
			}
		}
	}
	
	tick = false;
}

/**
 * \brief        displays the elapsed time in the following ports
 *               PORTA - Hours
 *               PORTB - Minutes
 *               PORTC - Seconds
 * 
 * 
 * \return void
 */
void display_time()
{
	PORTC.OUT = ~(t.second); //invert output for STK600 LED
	PORTB.OUT = ~(t.minute);
	PORTA.OUT = ~(t.hour);
}

int main(void)
{
	// allow device to run on default 2MHz internal RC as system clock (default)
	
	cli(); //clear global interrupts
	
	configure_port( &PORTC );
	configure_port( &PORTB );
	configure_port( &PORTA );
	
	just_enable_interrupts();
	
	//Rtc_Init(OSC_RC32KEN_bm, RTC_PRESCALER_DIV1_gc, 1);
	Rtc_Init(OSC_RC32KEN_bm, RTC_PRESCALER_DIV1024_gc, 1);
	
	sei();
	
	while(1)
	{
		if ( tick )
		{
			update_time();
		}
		display_time();
	}
	
}