#include "dev_uart.h"
#include <stdio.h>

struct __FILE{
    int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f)
{
    (void)f;
    
    UARTDevice *ptdev = UARTDev_Find("DEBUG");
    if(NULL == ptdev)   return 0;
    
    if(ptdev->Write(ptdev, (uint8_t*)&ch, 1) == 1)
        return ch;
    
    return 0;
}

//extern void WaitUart1Tx();

//int fputc(int ch, FILE *f)
//{
//    (void)f;
//    
//    //UARTDevice *ptdev = UARTDev_Find("DEBUG");
//    //if(NULL == ptdev)   return 0;
//    
//    while (HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX);
//    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart1, (uint8_t*)&ch, 1);
//    if(HAL_OK != status)    return 0;
//    WaitUart1Tx();
//    return ch;

//}



