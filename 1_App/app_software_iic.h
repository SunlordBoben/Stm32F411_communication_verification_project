/**
  ******************************************************************************
  * @file    app_software_iic.h
  * @brief   软件模拟IIC（GPIO位操作）头文件
  * @note    PB12 = SCL(时钟线)   PB13 = SDA(数据线)
  ******************************************************************************
  */
#ifndef __APP_SOFTWARE_IIC_H__
#define __APP_SOFTWARE_IIC_H__

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* 软件IIC引脚定义 */
#define SOFT_IIC_SCL_PORT       GPIOB
#define SOFT_IIC_SCL_PIN        GPIO_PIN_12
#define SOFT_IIC_SDA_PORT       GPIOB
#define SOFT_IIC_SDA_PIN        GPIO_PIN_13

/* SCL 电平控制宏 */
#define SOFT_IIC_SCL_H()        HAL_GPIO_WritePin(SOFT_IIC_SCL_PORT, SOFT_IIC_SCL_PIN, GPIO_PIN_SET)
#define SOFT_IIC_SCL_L()        HAL_GPIO_WritePin(SOFT_IIC_SCL_PORT, SOFT_IIC_SCL_PIN, GPIO_PIN_RESET)

/* SDA 电平控制/读取宏 */
#define SOFT_IIC_SDA_H()        HAL_GPIO_WritePin(SOFT_IIC_SDA_PORT, SOFT_IIC_SDA_PIN, GPIO_PIN_SET)
#define SOFT_IIC_SDA_L()        HAL_GPIO_WritePin(SOFT_IIC_SDA_PORT, SOFT_IIC_SDA_PIN, GPIO_PIN_RESET)
#define SOFT_IIC_SDA_READ()     HAL_GPIO_ReadPin(SOFT_IIC_SDA_PORT, SOFT_IIC_SDA_PIN)

/* 软件IIC GPIO初始化：PB12/PB13 配置为开漏输出 + 内部上拉 */
void Software_IIC_GPIO_Init(void);

/* IIC 时序基础函数 */
void    Software_IIC_Start(void);                      /* 起始信号 */
void    Software_IIC_Stop(void);                       /* 停止信号 */
uint8_t Software_IIC_SendByte(uint8_t data);           /* 发送一个字节，返回应答状态(0=ACK) */
uint8_t Software_IIC_ReadByte(void);                   /* 读取一个字节 */
uint8_t Software_IIC_WaitAck(void);                    /* 等待从机应答(0=ACK，1=NACK) */
void    Software_IIC_Ack(void);                        /* 主机发送应答ACK */
void    Software_IIC_NAck(void);                       /* 主机发送非应答NACK */

/* IIC 寄存器读写接口(dev_addr为7位器件地址) */
uint8_t Software_IIC_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata, uint16_t len);
uint8_t Software_IIC_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata, uint16_t len);

#endif /* __APP_SOFTWARE_IIC_H__ */
