# UNI/O Library for STM32F103

* Author: Tom LeMense 
* Toolchain: Keil uVision/MDK
* Target: STM32F103TB (CM3, 128K Flash, 20K RAM)
* Hardware: STM32F103 Blue Pull
* Repository: [github.com/tomcircuit/STM32F103_UNIO](https://github.com/tomcircuit/STM32F103_UNIO)

## Description

The main (only?) purpose for this is to allow usage of the Microchip 11AAxxx family of UNI/O EEPROM devices with an STM32F103 device. This code is largely based upon that of [NESFreak MSP430_UNIO](https://github.com/NESFreak/MSP430_UNIO). I was able to adapt towards the STM32F103 with only a very few changes. Here, briefly, are the key differences from the MSP430 version:

* I chose to configure the UNI/O bus GPIO as Open Drain. This allows
the pin to be read/written without having to change pin direction.

* The MSP430 driver used software timed loops for bit timing. I chose
to use TIM3, one of the STM32F103's General Purpose Timers in a polled fashion. I did not bother to disable interrupts because the STM32F103 is normally running quite a bit faster than the bit timing, so any well-behaved ISR should not affect the timing very much. YMMV.

* There is a fair amount of overhead in the bit
functions. The target bit timing is 32us per bit, but in my testing
this was closer to 36us/bit with a 24MHz sysclk. Because the 11AAxxx EEPROM bit timing
accuracy is non-critical, I did not really work too hard to optimize this. From
my observations, the timing stability is sufficient.

* I changed many of the the function names, in an effort to make it more
apparent what they do. For example, the functions that transmit
to the UNI/O device have "send" in the name, and the functions that
receive from the device have "receive" in the name. I followed suit
for the "send_byte" and "send_n_bytes" functions, again for clarity.
The words "read" and "write" are reserved for the EEPROM high level
operations with the same names. I did not rename the hgher-level
level EEPROM access functions, to ease porting.

* I exposed the SETAL and ERAL commands to allow bulk set and builk
erase of the EEPROM bits. These require a separate UNIO_enable_write()
call immediately beforehand.

## Usage

UNIO.h has hardware-specific defines in it that will need to be reviewed and perhaps changed prior to use.

```
/* macros for UNIO pin maniuplation - here, B0 is assumed */
/* assumes that GPIO is configured as Open Drain output */
#define UNIO_LOW  do { GPIOB->BRR = GPIO_Pin_0; } while (0)
#define UNIO_HIGH do { GPIOB->BSRR = GPIO_Pin_0; } while (0)
#define UNIO_INP (GPIOB->IDR & GPIO_IDR_IDR0)

/* macros for GPT maninpultion - here TIM3 is assumed */
/* assumes that GPT is set for 1us/tick and One Pulse Mode */
#define GPT_START do { TIM3->CR1 |= TIM_CR1_CEN; } while (0)
#define GPT_STOP  do { TIM3->CR1 &= ~(TIM_CR1_CEN); } while (0)
#define GPT_RELOAD(val) do { TIM3->ARR = (val); } while (0)
#define GPT_UPDATE do { TIM3->EGR |= TIM_EGR_UG ; } while (0)
#define GPT_RUNNING (TIM3->CR1 & TIM_CR1_CEN)
#define GPT_DELAY do { __NOP(); } while (GPT_RUNNING)
```

The UNI/O bus frequency is also configured in UNIO.h
```
/* UNIO bus timing - all derived from UNIO_BIT_US */
/* !!THIS TIMING IS NOT ACCURATE! ADJUST IF BITRATE IS CRITICAL!! */
#define UNIO_BIT_US (32u)
#define UNIO_HALF_BIT_US ((UNIO_BIT_US / 2u) - 1u)
#define UNIO_QUARTER_BIT_US ((UNIO_BIT_US / 4u) - 1u)
```

Prior to using any of the UNIO functions, the STM32F103 GPIO and GPT peripherals must be **enabled** and **configured**. The main.c file in this repro gives an example of this, using B0 and TIM3. The underpininngs of this main.c file are the **ST Standard Peripheral Library**. 

Tested under Keil MDK using STM32F103 "Blue Pill" board and a 11AA080 EEPROM device.
  
  Tom LeMense, March 2024
  
  
