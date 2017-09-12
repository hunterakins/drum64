#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>


// Learned from examples by matsi (https://github.com/geomatsi)
// Link to original repo:  https://github.com/geomatsi/avr-tests/tree/master/boards/attiny85-bare-metal/apps/led-gpio
// and articles by Mayank on AVR programming, e.g. https://maxembedded.wordpress.com/2011/06/28/avr-timers-timer1/
// CREDIT SITUATION: because this file was a copy of matsi's led-gpio main.c, but I've changed almost every line 
// to follow Max's tutorial. Furthermore, I completely changed the Makefile. Nonetheless, it got me started, so that was invaluable.

// data sheet is the other resource:
// http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf
// page numbers refer to the data sheet



volatile uint8_t tick;


void timer0_init() {
	/* Note about the control registers:
	CTC functionality is distributed across the two control registers.
	For example, to enable CTC, I need WGM00 = 0, WGM01 = 1, and WGM02 = 0
	WGM02 is in register B, while WGM00 and WGM01 are in register A.
	In this case, I only need to write to register A to enable CTC.
	*/
	
	
	// timer/counter control register A, used to set CTC (see pg. 79)
	TCCR0A |= (1 << WGM01);

	
	// timer/counter control register B, prescaling the counter (see pg. 80)
	TCCR0B |= (1 << CS00); // no prescalin
		
	// initialize timer to 0
	TCNT0 = 0;
	/* output compare register
	whatever value is in TCNT0 gets compared to this value
	at 8MHz, a tick is .125 us
	8 ticks is 1 us
	TCNT0 is going 0000, 0001, 0010, 0011, 0100, 0101, 0110, 0111, 1000;
	*/
	OCR0A = 8;	

	// enable compare interrupt:
	TIMSK |= (1 << OCIE0A);
	// enable global interrupts

	// initialize tick
	tick = 0;
	sei();

}	



ISR(TIMER0_COMPA_vect) {
	PORTB ^= (1 << PB1);
}

int main(void)
{

	// set PB1 to output (red LED)
	DDRB = (1 << DDB1);
	PORTB |= (1 << PB1);

	timer0_init();

	while(1)
	{
	}
}


