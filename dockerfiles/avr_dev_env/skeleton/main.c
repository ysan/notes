#include <avr/io.h>
#include <util/delay.h>

/* ATtiny404: PA3 (14pin版の ピン7) のLEDを点滅 */
int main(void)
{
  PORTA.DIRSET = PIN3_bm;
  for (;;) {
    PORTA.OUTTGL = PIN3_bm;
    _delay_ms(500);
  }
}
