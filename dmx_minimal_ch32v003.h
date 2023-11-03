/*
Guus 2023. channel vals are 0-511 (NOT 1-512)
tested on ch32v003.
This library does not set the rs485 driver in the right direction. Do so before starting or permanent by hardware.
Receiver only, doesn't save a full frame so a doesn't require much memory.
make shure to declare a global volatile arr or malloc a global volatile arr before sending it to dmx_beginRX().
The callbak is running in the itc, so maybe make short and snappy.
*/

#ifndef dmx_minimal_ch32v_h
#define dmx_minimal_ch32v_h
#define DMX_UART1 // defines the uart to use

#include <ch32v00x.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_rcc.h>
#include <ch32v00x_usart.h>
#define GPIOPORT GPIOD
#define RCC_APB2Periph_GPIOPORT RCC_APB2Periph_GPIOD
#define GPIORXPIN GPIO_Pin_6

typedef enum
{
    DMX_STOP = 0x00,
    DMX_IDLE = 0x01,
    DMX_BREAK = 0x02,
    DMX_START = 0x03,
    DMX_RUN = 0x04
} dmx_state_t;

// starts the receiver
unsigned short dmx_beginRX(unsigned short startaddr, volatile unsigned char *p, unsigned short len);
// stop
void dmx_stop();

// callback thats fired if all requested addresses are updated
void dmx_setCallback(void (*p)(void));

unsigned short dmx_changeAddres(unsigned short addr);

// set the startcode to filter at
void dmx_setStartcode(unsigned char sc);

// returns 0xff if new frame is avail since last check
unsigned char dmx_newPacket();

extern void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#endif