#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

volatile unsigned long timer0_millis = 0;
ISR(TIMER0_COMPA_vect) { timer0_millis++; }

void init_millis() {
  // 16MHz, every 250 clock cycles, rescaled by 64.
  // 16,000,000/(250 × 64) = CTC at 1000Hz
  OCR0A = 249;
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS01) | (1 << CS00); // clk/64
  TIMSK = (1 << OCIE0A);
}

unsigned long millis(void) {
  unsigned long millis_return;
  ATOMIC_BLOCK(ATOMIC_FORCEON) { millis_return = timer0_millis; }
  return millis_return;
}