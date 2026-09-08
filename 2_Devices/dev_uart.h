#ifndef __DEV_UART_H_
#define __DEV_UART_H_

#include "stm32f4xx_hal.h"

typedef struct UARTDev{
    
    char* name;
    UART_HandleTypeDef* UART;
    unsigned char channel;
    int(*Init)(struct UARTDev* dev);
    int(*Write)(struct UARTDev* dev, const uint8_t *pData, uint16_t Size);
    int(*Read)(struct UARTDev* dev, uint8_t *pData, uint16_t Size);
    void (*TxIRQHandler)(void *arg);
    void (*RxIRQHandler)(void *arg);
    struct UARTDev* next;
    
}UARTDevice;

void AllUARTDev_Regis(void);
struct UARTDev* UARTDev_Find(char* name);
int UARTDev_Insert(struct UARTDev* Dev);



#endif /* __DEV_UART_H_ */
