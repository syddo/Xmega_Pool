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
volatile uint8_t time_pC;
volatile uint8_t time_pB;

static void time_feedback();

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
	//port->OUT |= ~(0x00); // inverted for STK600
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
    time_feedback();
}

static void time_feedback()
{
    time_pC |= t.second & 0x3F;
    time_pC |= (t.minute &0x0F)<<6;
    time_pB |= t.minute & 0x3C; 
    time_pB |= (t.hour&0xF0 )<<4;
}

//static void display_time_compressed()
//{
//    PORTC.OUT = time_pC;
//    PORTB.OUT = time_pB;
//}

/**
 * \brief        enables the xosc 32kHz at TOSC pins
 * 
 * 
 * \return void
 */
void xosc_enable_32kHz()
{
    OSC.XOSCCTRL |= OSC_XOSCSEL_32KHz_gc;
    OSC.CTRL |= OSC_XOSCEN_bm;
    while ( !(OSC.STATUS & OSC_XOSCRDY_bm) ); // wait for xosc to stabilize
}

void usart_init_tx()
{
    // set TxD H, XCK - L
    PORTC.OUTSET |= PIN2_bm;
    // TxD pin as output
    PORTC.DIR |= 0xFF;
    // Set Baud rate and frame format
    USARTC0.BAUDCTRLA = 12;
    // set mode of operation
    USARTC0.CTRLA |= 0; //no interupts
    USARTC0.CTRLB |= 0x08; //Tx only
    USARTC0.CTRLC |= 0x03;  // select asynchronous USART, disable parity, 1 stop bit, 8 data bits.
     
}

void usart_send_char( char c)
{
    while( !(USARTC0_STATUS & USART_DREIF_bm)); //wait until data buffer is empty
    USARTC0_DATA = c;
}

void usart_send_string( char *text)
{
    while(*text != '\0')
    {
        usart_send_char(*text++);
    }
}

/**
    Send a unsigned 8bit integer on USART as dec

    @param[in]  number      8 bit number to format
*/
void usart_print_dec(uint8_t number)
{
    char tmp[11];
    uint8_t i = sizeof(tmp) - 1;

    tmp[i] = '\0';
    do {
        tmp[--i] = (number % 10) + '0';
        number /= 10;
    } while (number != 0);

    usart_send_string(tmp + i);
}

void usart_display_time()
{
    usart_send_string("Running Time:");
    usart_send_string("\nSeconds: ");
    usart_print_dec(t.second);
    usart_send_string("\nMinutes: ");
    usart_print_dec(t.minute);
    usart_send_string("\nHours: ");
    usart_print_dec(t.hour);
    usart_send_string("\n");
}

int main(void)
{
	// allow device to run on default 2MHz internal RC as system clock (default)
	
	cli(); //clear global interrupts
	
	//configure_port( &PORTC );
	//configure_port( &PORTB );
    usart_init_tx();
    
    xosc_enable_32kHz();
    
    just_enable_interrupts();
    
    sei();
	
	//Rtc_Init(OSC_RC32KEN_bm, RTC_PRESCALER_DIV1_gc, 0x400);
	Rtc_Init(CLK_RTCSRC_TOSC_gc, RTC_PRESCALER_DIV1_gc, 0x400);
    
    
	
	while(1)
	{
		if ( tick )
		{
			update_time();
            usart_display_time();
		}
		//display_time_compressed();
        

	}
	
}