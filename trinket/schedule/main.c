#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

#define STATE ((PINB >> PB4) & 1)

volatile uint8_t tick;
volatile uint8_t tock;
volatile uint8_t controller0;
volatile uint8_t controller1;
volatile uint8_t controller2;
volatile uint8_t controller3;
volatile char nop;
unsigned char data;
uint8_t i;
uint8_t last;
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
	GTCCR |= ((1 << COM1B0)|(1 << COM1B1));
	
	// had to enable this due to bug 27.2.3 article 4 (pg. 213)
	TCCR1 |= ((1 << COM1A0) | (1 << COM1A1));

	// enable pulse width modulator
	GTCCR |= (1 << PWM1B);
	
	// set prescale
	TCCR1 |= (1 << CS10);

	TCNT1 = 0;

	// enable super speed clock (64MHz)
	PLLCSR = (1 << PCKE);

	OCR1B = 64;   // duty cycle
	OCR1C = 255; // period of waveform

}	
void spi_init() {
	// enable data output
	USICR |= (1 << USIWM0);	
}

void spi_transfer(unsigned char data) {
	USIDR = data;
	while (!(USIDR & (1 << USISIF)));
		
}


ISR(TIMER0_COMPA_vect) {
	if (tick == 0) {
		OCR0A = 255;
		tick = 1;
		//disable PWM, clear the PB line
		GTCCR &= ~(1 << PWM1B);
		GTCCR &= ~((1 << COM1B0) | (1 << COM1B1));
		// switch to input
		DDRB &= ~(1 << DDB4);
		tock = 0;
		for (i = 0; i < 8; i ++) {
			last |= STATE & 1;
			controller0 |= (last & STATE) << i;
		}	
		for (i = 0; i < 8; i ++) {
			last = STATE;
			controller1 = (last & STATE) << i;
		}
		for (i = 0; i < 8; i++) {
			last = STATE;
			controller2 = (last & STATE ) << i;
		}	
		for (i = 0; i < 8; i ++) {
			last = STATE;
			controller3 = (last & STATE) << i;
		}	
		for (i = 0; i < 8; i++){
			PORTB = (~((controller0 >> i) &1)) << PB3;
		}
		for (i = 0; i < 8; i++){
			PORTB = ((controller1 >> i) &1) << PB3;
		}
		DDRB |= (1 << DDB4);
		PORTB = ((1 << PB4) | (1 << PB2));
	}
	//time to poll 
	else if (tick == 1) {
		controller0 = 0;
		controller1 = 0;
		controller2 = 0;
		controller3 = 0;
		// interrupt in 28 us
		OCR0A = 53;
		tick = 2;
		// starting the poll
		// set 25% duty cycle 
		OCR1B = 191;
		// switch on PWM
		GTCCR |= ((1 << COM1B0) | (1 << COM1B1));
		GTCCR |= (1 << PWM1B);
	}
	else {
		// send the one signal
		// interrupt again in 8 us
		// change the duty cycle to 75%
		OCR1B = 64;
		OCR0A = 10;
		tick = 0;
		PORTB |= (1 << PB2);
	}
}

int main(void) {
	timer0_init();
	timer1_init();
	spi_init();
	// set PB4 to output and PB1 (MISO) 
	DDRB = (1 << DDB4) | (1 << DDB1) | (1 << DDB3);
	// set PB4 high;
	while(1) {
		;
	}
}


