#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>


//pwm version of the poll
// idea is to use pwm output
// also use a separate timer interrupt that will change the duty cycle
// basically, I have a 4us square
// a 0 is represented as a 25% duty cycle, but starts delayed...kinda tricky.
// a 1 is represented as a 75% duty cycle
// the polling signal is a  

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

void timer1_init() {
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
	OCR0A = 8; // every us	
	OCR0B = 32; // every 4 us

	// enable compare interrupt for the two counters:
	TIMSK |= ((1 << OCIE0A) | (1 << OCIE0B));

	// tick and tock keep track of interrups
	tick = 0; // 1 us
	tock = 0; // 4 us
	received_bits = 0; // enable the 128 us delay necessary to retrieve
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
			tick = 0
		}
	else {          
		if (tick == 0) {
			PORTB &= (0 << PB1);
			tick += 1;
		else if (tick < 3) {
			PORTB |= (1 << PB1);
			tick += 1;
		else {
			tick = 0;
		{
}

ISR(TIMER0_COMPB_vect) {
	if (tock < 9) {
		tock += 1;
	}
	else if (tock == 9) {
		DDRB = (1 << DDB1); // set PB1 to input
		overflows += 1;

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


