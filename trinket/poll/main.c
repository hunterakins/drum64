#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>



#define _NOP() do { __asm__ __volatile__ ("nop"); } while (0)

volatile uint8_t tick;
volatile uint8_t tock;
volatile uint32_t controller_state;
volatile uint32_t received_bits; 

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
	OCR0A = 32; // every us	
	OCR0B = 128; // every 4 us

	// enable compare interrupt for the two counters:
	TIMSK |= ((1 << OCIE0A) | (1 << OCIE0B));

	tick = 0;
	tock = 0;
	controller_state = 0;
	received_bits = 0;

	// enable global interrupts
	sei();

}	


ISR(TIMER0_COMPA_vect) {
	if (tock < 7) { 
		if (tick < 3) {
			PORTB &= (0 << PB1); 
			tick += 1;
		}
		else {
			PORTB |= (1 << PB1);
			tick = 0;
			tock += 1;
		}
	}
	else {          
		if (tick == 0) {
			PORTB &= (0 << PB1);
			tick += 1;
		}
		else if (tick < 3) {
			PORTB |= (1 << PB1);
			tick += 1;
		}
		else {
			tick = 0;
			tock += 1;
		}
	}	
}

ISR(TIMER0_COMPB_vect) {
	if (tock < 9) {
		tock += 1;
	}
	else if (tock >= 9) {
		// set PB1 to input
		OCR0A = 255;
		DDRB &= (0 << DDB1); 
		if (received_bits >= 32) {
		 	received_bits = 0;
			//set PB1 to output
			DDRB = (1 << DDB1);
			tock = 0;
			OCR0A = 32;
		}
		else {
			// set received_bits bit of controller_state to the input from PB1
			controller_state |= ((PINB << PB1) << received_bits);
			received_bits += 1;
		}
	}
}

int main(void) {
	// set PB1 to output (red LED)
	DDRB = (1 << DDB1);
	PORTB |= (1 << PB1);

	//timer0_init();

	while(1) {
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
		_NOP();
		PORTB ^= (1 << PB1);
	}
}


