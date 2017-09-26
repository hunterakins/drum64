#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

volatile uint8_t tick;
volatile uint8_t tock;
volatile uint8_t i;
volatile uint32_t controller;
volatile char nop;

void timer0_init() {
	
	// timer/counter control register A, used to set CTC (see pg. 79)
	TCCR0A |= (1 << WGM01);

	// set prescale 
	TCCR0B |= (1 << CS01);	
		
	// initialize timer to 0
	TCNT0 = 0;

	// output compare register
	//  1 us per tick
	OCR0A = 222;	

	// enable compare interrupt:
	TIMSK |= (1 << OCIE0A);

	// initialize tick
	tick = 0;

	// enable global interrupts
	sei();

}	


void timer1_init() {

	// enable compare output mode	
	TCCR1 |= (1 << COM1A0) | (1 << COM1A1);

	// enable pulse width modulator
	TCCR1 |= (1 << PWM1A);
	
	// set prescale
	TCCR1 |= (1 << CS10);

	TCNT1 = 0;

	// enable super speed clock (64MHz)
	PLLCSR = (1 << PCKE);

	OCR1A = 195;   // duty cycle
	OCR1C = 255; // period of waveform

}	

ISR(TIMER0_COMPA_vect) {
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
	PORTB = (((PINB >> PINB3) & 1) << PB2);
}

int main(void) {
	timer0_init();
	timer1_init();
	// set PB1 to output (red LED)
	DDRB = (1 << DDB1) | (1 << DDB2);
	// set PB1 high;
	PORTB |= (1 << PB1);
	while(1) {
		;
	}
}


