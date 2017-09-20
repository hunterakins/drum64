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
		// send the one signal
		// interrupt again in 8 us
		// change the duty cycle to 75%
		OCR1A = 64;
		tick = 0;
		// to run out time before switching to reading
		for (nop =0; nop<6; nop++) {
			_NOP();
		}
		//disable PWM, clear the PB line
		TCCR1 &= ~(1 << PWM1A);
		TCCR1 &= ~(1 << COM1A0);
		// switch to input
		DDRB &= ~(1 << DDB1);
		// debug port
		PORTB ^= (1 << PB2);
		// i just completed the ones segment
		
		// do the reading in one go, no interrupts since its too fast I think
		OCR0A = 80;
		tick = 1;
		tock = 0;
		// time to read
		while (tock < 32) {
			tock += 1;
			_NOP();
			_NOP();
			//debug
			PORTB ^= (((PINB >> PIN1) & 1) << PB2);
		}

		// done reading...
		tick = 1;
		// switch PB1 to output
		DDRB |= (1 << DDB1);
		// leave PB1 high
		PORTB |= (1 << PB1);
		// reset tock for next read
		tock = 0;
	}
	//time to poll 
	else if (tick == 1) {
		// debug
		PORTB ^= (1 << PB2);
		OCR0A = 32;
		tick = 0;
		// starting the poll
		// set 25% duty cycle 
		OCR1A = 191;
		// switch on PWM
		TCCR1 |= (1 << COM1A0);
		TCCR1 |= (1 << PWM1A);
		// interrupt in 28 us
	}
}

int main(void) {
	timer0_init();
	timer1_init();
	// set PB1 to output (red LED)
	DDRB = (1 << DDB1) | (1 << DDB2);
	// set PB1 high;
	PORTB |= (1 << PB1);
	uint8_t i = 0;
	while(1) {
		;
	}
}


