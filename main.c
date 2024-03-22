#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x.h"
#include "usart_utilities.h"
#include "unio.h"

/* Target: STM32F103TBU6 on BLUE PILL PCB */
/* 24 MHz operating frequency - see RTE_Device.h */

#define EEPROM_BITS (8192)
#define EEPROM_BYTES (EEPROM_BITS / 8)

#define PERFORM_ERASE_ALL (false)
#define PERFORM_SET_ALL (false)

/* STM32F103 peripheral initialization prototypes*/
void InitGPIO();
void InitUSART();
void InitTIM3();

/////////////

void dump(uint16_t start, uint16_t length)
{
    uint8_t buf[EEPROM_BYTES];

    if ((start + length - 1) < EEPROM_BYTES)
    {
        UNIO_read(UNIO_EEPROM_ADDRESS, buf, start, length);
        for (int i = 0; i < length; UU_PutChar(USART1, buf[start + i++]));
    }
}

int main()
{
    uint8_t eebuf[64];

    /* clear buffer before using it */
    for (uint16_t i = 0; i < 64; eebuf[i++] = 0);

    /* initialize MCU peripherals: GPIO, USART, SysTick, TIM3 */
    InitGPIO();
    InitUSART();
    InitTIM3();

    /* DEBUG output a welcome string on USART */
    UU_PutString(USART1, "UNI/O");

    // DEBUG turn off PC13 LED
    GPIOC->BSRR = GPIO_Pin_13;

    /* UNI/O EEPROM testing */
    UNIO_init();

    if (PERFORM_SET_ALL)
    {
        UU_PutString(USART1, "SETAL");
        // set the entire EEPROM to FF's
        UNIO_enable_write(UNIO_EEPROM_ADDRESS);
        UNIO_set_all(UNIO_EEPROM_ADDRESS);
        UNIO_await_write_complete(UNIO_EEPROM_ADDRESS);

        /* dump EEPROM content to USART1*/
        dump(0, 64);
    }

    if (PERFORM_ERASE_ALL)
    {
        UU_PutString(USART1, "ERAL");
        // erase the entire EEPROM to 0's
        UNIO_enable_write(UNIO_EEPROM_ADDRESS);
        UNIO_erase_all(UNIO_EEPROM_ADDRESS);
        UNIO_await_write_complete(UNIO_EEPROM_ADDRESS);

        /* dump EEPROM content to USART1*/
        dump(0, 64);
    }

    /* Write "DEADBEEF" to EEPROM at address 3-7 */
    UU_PutString(USART1, "DEADBEEF");
    eebuf[0] = 0xDE;
    eebuf[1] = 0xAD;
    eebuf[2] = 0xBE;
    eebuf[3] = 0xEF;
    UNIO_simple_write(UNIO_EEPROM_ADDRESS, eebuf, 0x3, 0x4);

    /* Write "FEEDBABE" to EEPROM at address 30-33 (unaligned with page boundary) */
    UU_PutString(USART1, "FEEDBABE");
    eebuf[0] = 0xFE;
    eebuf[1] = 0xED;
    eebuf[2] = 0xBA;
    eebuf[3] = 0xBE;
    UNIO_simple_write(UNIO_EEPROM_ADDRESS, eebuf, 0x1e, 0x4);

    /* dump EEPROM content to USART1*/
    dump(0, 64);


    /* Write increasing count from addr 0 to 50 */
    UU_PutString(USART1, "COUNT");
    for (uint16_t i = 0; i < 64; eebuf[i++] = i);

    /* DEBUG tick on to time 51-byte write */
    GPIOC->BSRR = GPIO_Pin_14;

    UNIO_simple_write(UNIO_EEPROM_ADDRESS, eebuf, 0, 51);

    /* DEBUG tick off */
    GPIOC->BRR = GPIO_Pin_14;

    /* dump EEPROM content to USART1*/
    dump(0, 64);

    /* Turn on PC13 LED when complete */
    GPIOC->BRR = GPIO_Pin_13;

    while (1)
    {
        /* DEBUG tick on */
        GPIOC->BSRR = GPIO_Pin_14;
    }
}


/* STM32F103 GPIO Initialization */
void InitGPIO()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* GPIO are all on APB2 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* enable the AFIO to allow remaps as well */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* configure C13 and C14 as GPIO output pins */
    /* Blue Pill Debug pins! LED on C13 (0 = lit) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure PB0 as OD GPIO */
    /* This is 11AA080 UNI/O signal input-output with 47K pullup */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);

    /* configure PB6/PB7 as USART1 TXD & RXD */
    /* configure PB6 as AF GPIO */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* configure PB7 as AF input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Finally, remap USART1 to PB6 and PB7 */
    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
}

/* STM32F103 USART1 initialization */
void InitUSART()
{
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* 230400 bps, 8N1, no flow control, TX only */
    USART_InitStructure.USART_BaudRate = 230400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

/* STM32F103 TIM3 Initialization */
void InitTIM3()
{
    TIM_TimeBaseInitTypeDef TimeBaseInitStructure;

    /* Enable the TIM3 clock.   */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* De-init TIM3 to get it back to default values */
    TIM_DeInit(TIM3);

    /* Init the TimeBaseInitStructure with 1us/tick values */
    TimeBaseInitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
    TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TimeBaseInitStructure.TIM_Period = 100;
    TIM_TimeBaseInit(TIM3, &TimeBaseInitStructure);
    TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);
}


