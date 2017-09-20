#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

// this NOP takes 500 ns
#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)
volatile uint8_t tick;
volatile uint8_t tock;
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
	OCR0A = 8;	

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

	OCR1A = 64;   // duty cycle
	OCR1C = 255; // period of waveform

}	

ISR(TIMER0_COMPA_vect) {
	if (tick == 0) {
		OCR0A = 8;
		PORTB ^= (1 << PB2);
		tick = 1;
	}
	// time to read
	else if (tick == 1) {
		// reading will take my full attention
		OCR0A = 140;
		PORTB ^= (1 << PB2);
		tick = 2;
	}
	//time to poll 
	else if (tick == 2) {
		PORTB ^= (1 << PB2);
		tick = 3;
		// starting the poll
		// set 25% duty cycle 
		OCR1A = 191;
		// switch on PWM
		TCCR1 |= (1 << COM1A0);
		TCCR1 |= (1 << PWM1A);
		// interrupt in 28 us
		OCR0A = 32;
	}
	else {
		PORTB ^= (1 << PB2);
		// send the one signal
		// interrupt again in 8 us
		// change the duty cycle to 75%
		OCR1A = 64;
		OCR0A = 8;
		tick = 0;
		//disable PWM, clear the PB line
		TCCR1 &= ~(1 << PWM1A);
		TCCR1 &= ~(1 << COM1A0);
		// switch to input
		DDRB &= ~(1 << DDB1);
	}
}

int main(void) {
	timer0_init();
	timer1_init();
	// set PB1 to output (red LED)
	DDRB = (1 << DDB1) | (1 << DDB2);
	// set PB1 high;
	PORTB |= (1 << PB2);
	uint8_t i = 0;
	while(1) {
		;
	}
}


