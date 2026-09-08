#include "dev_uart.h"
#include "errno.h"
#include "usart.h"

static int UARTInit(struct UARTDev* dev);
static int UARTWrite(struct UARTDev* dev, const uint8_t *pData, uint16_t Size);
static int UARTRead(struct UARTDev* dev, uint8_t *pData, uint16_t Size);
void WaitUart1Tx(void);
void WaitUart3Tx(void);
static void (*gTxIRQHandler[6])(void *arg);
static void (*gRxIRQHandler[6])(void *arg);

volatile static int guart1_waitTX = 0;//等待串口1发送中断标志位
volatile static int guart3_waitTX = 0;//等待串口3发送中断标志位

uint8_t gUsart3RxData = 0;//串口接收数据

struct UARTDev DEBUG = {
    .name = "DEBUG",
    .UART = &huart1,
    .channel = 1,
    .Init = UARTInit,
    .Write = UARTWrite,
    .Read = UARTRead,
    .TxIRQHandler = NULL,
    .RxIRQHandler = NULL,
    .next = NULL
};

//struct UARTDev WIFIUART = {
//    .name = "WIFIUART",
//    .UART = &huart3,
//    .channel = 3,
//    .Init = UARTInit,
//    .Write = UARTWrite,
//    .Read = UARTRead,
//    .TxIRQHandler = NULL,
//    .RxIRQHandler = NULL,
//    .next = NULL
//};

/**
 * @brief 串口设备插入函数
 * 
 */
void UARTDev_Creat()
{
    UARTDev_Insert(&DEBUG);
//    UARTDev_Insert(&WIFIUART);
}



static int UARTInit(struct UARTDev* dev)
{
    if(dev == NULL)  return -ENAVAIL;
    
    switch(dev->channel)
    {
        case 1:
        {
            break;
        }
        case 3:
        {
//            HAL_StatusTypeDef status = HAL_UART_Receive_IT(&huart3, (unsigned char*)&gUsart3RxData, 1);
//            if(HAL_OK != status)        return -EIO;
            break;
        }
    }
    gTxIRQHandler[dev->channel - 1] = dev->TxIRQHandler;
    gRxIRQHandler[dev->channel - 1] = dev->RxIRQHandler;
    
    return ESUCCESS;
}

/**
 * @brief 串口发送函数
 * 
 * @param 
 */
static int UARTWrite(struct UARTDev* dev, const uint8_t *pData, uint16_t Size)
{
    if(dev == NULL || pData == NULL || Size == 0)  return -ENAVAIL;
    
    switch(dev->channel)
    {
        /* debug串口 */
        case 1:
        {
            // 检查串口状态
            while(HAL_UART_STATE_BUSY_TX == HAL_UART_GetState(&huart1));
            HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart1, pData, Size);
            if(HAL_OK != status)    return -EIO;
            WaitUart1Tx();//死等发送完成
            
            break;
        }
        case 3:/* WIFI串口 */
        {
            //while(HAL_UART_STATE_BUSY_TX == HAL_UART_GetState(&huart3));
//            HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart3, pData, Size);
//            if(HAL_OK != status)    return -EIO;
//            WaitUart3Tx();//死等发送完成
            
            break;
        }
        
    }
    
    return Size;
}

/**
 * @brief 串口接收函数
 * 
 * @param 
 */
static int UARTRead(struct UARTDev* dev, uint8_t *pData, uint16_t Size)
{
    if(dev == NULL || pData == NULL || Size == 0)  return -ENAVAIL;
    
    switch(dev->channel)
    {
        /* debug串口——只需发送功能 */
        case 1:
        {
            break;
        }
        case 3: break;
    }
    
    return Size;
}

/**
 * @brief 等待串口1发送完成
 * 
 */
void WaitUart1Tx()
{
    while(guart1_waitTX != 1);
    guart1_waitTX = 0;
}

/**
 * @brief 等待串口3发送完成
 * 
 */
void WaitUart3Tx(void)
{
    while(guart3_waitTX != 1);
    guart3_waitTX = 0;
}

/* 串口发送完成中断 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    void (*pIRQHandler)(void *arg);
    pIRQHandler = NULL;
    if(huart->Instance == USART1)
    {
        guart1_waitTX = 1;
        pIRQHandler = gTxIRQHandler[0];
    }
//    else if(huart->Instance == USART3)
//    {
//        guart3_waitTX = 1;
//        pIRQHandler = gTxIRQHandler[2];
//    }
    
    if(NULL != pIRQHandler)
        pIRQHandler(NULL);
    
}

/* 串口接收完成中断 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    void (*pIRQHandler)(void *arg);
    pIRQHandler = NULL;
    if(huart->Instance == USART1)
    {
//        pIRQHandler = gRxIRQHandler[0];
//        if(NULL != pIRQHandler)
//            pIRQHandler((unsigned char*)&gUsart1RxData);
    }
}

