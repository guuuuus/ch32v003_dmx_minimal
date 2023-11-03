#include <dmx_minimal_ch32v003.h>
// #include "ltc_ch32v.h"

void dmx_irqCallbackPlaceholder();

volatile unsigned short dmx_poscoutner = 0;

volatile unsigned short dmx_addresscount = 0;

volatile unsigned short dmx_startaddress = 0;

volatile unsigned short dmx_addresslen = 0;

volatile unsigned char *dmx_dataP;

void (*dmx_callback)(void) = &dmx_irqCallbackPlaceholder;

volatile unsigned char dmx_scfilter = 0x00;
volatile unsigned char dmx_newdata = 0x00;
volatile dmx_state_t dmx_state = DMX_IDLE;

void dmx_irqCallbackPlaceholder()
{
    ;
}

unsigned short dmx_beginRX(unsigned short startaddr, volatile unsigned char *p, unsigned short len)
{
    unsigned short ret = 0;
    if ((startaddr + len) > 512)
    {
        ret = -1;
        return ret;
    }
    dmx_dataP = p;
    dmx_addresslen = len;
    dmx_startaddress = startaddr;

    GPIO_InitTypeDef dxm_gpiorx;

    USART_InitTypeDef dmx_uartinit;

    // peripheral clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOPORT, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    dxm_gpiorx.GPIO_Pin = GPIORXPIN;
    dxm_gpiorx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    dxm_gpiorx.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOPORT, &dxm_gpiorx);

    dmx_uartinit.USART_BaudRate = 250000;
    dmx_uartinit.USART_WordLength = USART_WordLength_8b;
    dmx_uartinit.USART_StopBits = USART_StopBits_1; // is specified with 2, but the second is just idle time in this case.
    dmx_uartinit.USART_Parity = USART_Parity_No;
    dmx_uartinit.USART_Mode = USART_Mode_Rx;
    dmx_uartinit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_DeInit(USART1);
    USART_Init(USART1, &dmx_uartinit);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 0xe0);


    USART_Cmd(USART1, ENABLE);

    return ret;
}

void dmx_stop()
{
    // itcs
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    // uart peri
    USART_Cmd(USART1, DISABLE);

    dmx_state = DMX_STOP;
}

void dmx_setStartcode(unsigned char sc)
{
    dmx_scfilter = sc;
}

unsigned short dmx_changeAddres(unsigned short addr)
{
    unsigned short ret = 0;
    if ((addr + dmx_addresslen) > 511)
    {
        ret = -1;
        return ret;
    }
    dmx_startaddress = addr;
    return ret;
}

unsigned char dmx_newPacket()
{
    unsigned char r = 0;
    if (dmx_newdata)
    {
        r = 0xff;
        dmx_newdata = 0x00;
    }
    return r;
}

void dmx_setCallback(void (*p)(void))
{
    dmx_callback = (*p);
}

void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART1_IRQHandler()
{
    unsigned char fe = USART_GetFlagStatus(USART1, USART_FLAG_FE);
    unsigned char data = (0xff & USART_ReceiveData(USART1));

    if (fe)
        dmx_state = DMX_BREAK;

    switch (dmx_state)
    {
    case DMX_BREAK:
        if (fe)
        {
            dmx_poscoutner = 0;
            dmx_addresscount = 0;
            dmx_state = DMX_START;
        }
        break;

    case DMX_START:
        if (data == dmx_scfilter)
            dmx_state = DMX_RUN;
        else
            dmx_state = DMX_BREAK;

        break;

    case DMX_RUN:
        if (dmx_addresscount >= dmx_startaddress)
        {
            dmx_dataP[dmx_poscoutner] = data;
            dmx_poscoutner++;
            if (dmx_poscoutner >= dmx_addresslen)
            {
                dmx_state = DMX_BREAK;
                dmx_callback();
                dmx_newdata = 0xff;
            }
        }
        dmx_addresscount++;
        if (dmx_addresscount >= 512)
        {
            dmx_state = DMX_BREAK;
        }
        break;
    default:
        break;
    }
}
