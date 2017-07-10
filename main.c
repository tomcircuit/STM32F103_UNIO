/* Copyright (C) 2017 by Jeroen Lanting <https://github.com/NESFreak>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */
#include <msp430.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "UNIO.h"

const uint8_t unio_teststring[] = "teststring";
uint8_t unio_testRes[sizeof(unio_teststring)];

// Setup and mainloop
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

////////////// Terminate unused ports ///////
  P2DIR = 0xFF;
  P2OUT = 0x00;
  P2SEL = 0x00;
  P2SEL2 = 0x00;

  P1DIR = 0xFF;
  P1OUT = 0x00;
  P1SEL = 0x00;
  P1SEL2 = 0x00;

// Put UNI/O eeprom attached to P1.0 to sleep by setting its dataline high
  UNIO_init();

// Set internal clock to 8mhz
  DCOCTL = CALDCO_8MHZ;                        /* DCOCTL  Calibration Data for 8MHz */
  BCSCTL1 = CALBC1_8MHZ;                       /* BCSCTL1 Calibration Data for 8MHz */


// Test UNIO
  while (1)
  {
    volatile bool result;
    result = UNIO_simple_write(UNIO_EEPROM_ADDRESS, unio_teststring, 0, sizeof(unio_teststring));
// Breakpoint here to view result
    _no_operation();
    memset(unio_testRes, 0, sizeof(unio_teststring));
    result = UNIO_read(UNIO_EEPROM_ADDRESS, unio_testRes, 0, sizeof(unio_testRes));
// Breakpoint here to view result
    _no_operation();
    
//sleep for 1s
    volatile size_t i = 1000;
    while (--i) __delay_cycles(8000);
  }
}
