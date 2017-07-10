After getting frustrated with microchips own [AN1186](https://www.microchip.com/wwwAppNotes/AppNotes.aspx?appnote=en535411),
and having had some succes with https://github.com/sde1000/NanodeUNIO on the arduino, I've decided to port that library to the msp430.

Pins and clockspeed need to be configured in the header `UNIO.h`.
Specifically, the following defines will probably need to be updated:
```
#define UNIO_EEPROM_ADDRESS (0xa0)
#define UNIO_PXIN     (P1IN)
#define UNIO_PXOUT    (P1OUT)
#define UNIO_PXDIR    (P1DIR)
#define UNIO_PXSEL    (P1SEL)
#define UNIO_PXSEL2   (P1SEL2)
#define UNIO_PIN      (0)
/* The number of clocks a us takes, as timings are minima,
   round this number up to the nearest integer.*/
#define UNIO_USCLKS   (8)
```

Tested with both the ti compiler (`v17.3.0`, `c99`), and gcc (`6.2`, `-std=gnu99`) on both the MSP430AFE253, and the MSP430F2013.