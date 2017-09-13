#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

volatile uint8_t tick;
volatile uint8_t tock;
volatile uint32_t controller;


void timer0_init() {
	
	// timer/counter control register A, used to set CTC (see pg. 79)
	TCCR0A |= (1 << WGM01);

	// set prescale 
	TCCR0B |= (1 << CS01);	
		
	// initialize timer to 0
	TCNT0 = 0;

	// output compare register
	//  1 us per tick
	OCR0A = 28;	

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
		// i just completed the ones segment
		//disable PWM, clear the PB line
		TCCR1 &= ~(1 << PWM1A);
		TCCR1 &= ~(1 << COM1A0);
		// switch to input
		DDRB &= ~(1 << DDB1);
		controller |= (((PINB >> PINB1) & 1) << tock);	
		// interrupt every 4 us for the next tick
		OCR0A = 4; 
		tick = 1;
		tock += 1;
	}
	// time to read
	else if (tick == 1) {
		if (tock < 32) {
			controller |= (((PINB >> PINB1) & 1) << tock);
			tock += 1;
		}
		else {
			// done reading...
			tick = 2;
			// take a little break
			OCR0A = 100;
			// switch PB1 to output
			DDRB |= (1 << DDB1);
			// leave PB1 high
			PORTB |= (1 << PB1);
		}
	}
	//time to poll 
	else if (tick == 2) {
		tick = 3;
		// starting the poll
		// set 25% duty cycle 
		OCR1A = 191;
		// switch on PWM
		TCCR1 |= (1 << COM1A0);
		TCCR1 |= (1 << PWM1A);
		// interrupt in 28 us
		OCR0A = 32;
		tock = 0;
	}
	else {
		// send the one signal
		// interrupt again in 8 us
		OCR0A = 8;
		// change the duty cycle to 75%
		OCR1A = 64;
		tick = 0;
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
		for (i = 0; i < 32; i++) {
			PORTB |= (controller >> i);
			_delay_ms(10);
		}
	}
}


