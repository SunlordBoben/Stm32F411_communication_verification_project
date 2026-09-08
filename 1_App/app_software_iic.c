/**
  ******************************************************************************
  * @file    app_software_iic.c
  * @brief   软件模拟IIC（GPIO位操作）驱动实现
  * @note    PB12 = SCL(时钟线)   PB13 = SDA(数据线)
  *          使用前需先调用 Software_IIC_GPIO_Init() 完成引脚初始化
  ******************************************************************************
  */
#include "app_software_iic.h"

/* 延时参数：根据系统主频(当前100MHz)调整，值越大IIC速率越慢 */
#define SOFT_IIC_DELAY   50

/**
 * @brief  软件IIC微秒级延时(空循环)
 */
static void Software_IIC_Delay(void)
{
    volatile uint32_t i;
    for(i = 0; i < SOFT_IIC_DELAY; i++)
    {
    }
}

/**
 * @brief  软件IIC GPIO初始化
 * @note   PB12 -> SCL(时钟线)，PB13 -> SDA(数据线)
 *         均配置为开漏输出并使能内部上拉：
 *         - 输出低电平时直接拉低引脚
 *         - 释放(写高)时靠上拉电阻恢复高电平
 *         - 开漏输出状态下可正常读取SDA引脚输入电平，无需切换模式
 */
void Software_IIC_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin   = SOFT_IIC_SCL_PIN | SOFT_IIC_SDA_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;    /* 开漏输出 */
    GPIO_InitStruct.Pull  = GPIO_PULLUP;             /* 内部上拉 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 空闲状态：SCL、SDA均为高电平 */
    SOFT_IIC_SCL_H();
    SOFT_IIC_SDA_H();
}

/**
 * @brief  IIC起始信号：SCL为高电平期间，SDA由高拉低
 */
void Software_IIC_Start(void)
{
    SOFT_IIC_SDA_H();
    SOFT_IIC_SCL_H();
    Software_IIC_Delay();

    SOFT_IIC_SDA_L();               /* SDA高->低 */
    Software_IIC_Delay();

    SOFT_IIC_SCL_L();               /* 钳住总线，准备发送数据 */
    Software_IIC_Delay();
}

/**
 * @brief  IIC停止信号：SCL为高电平期间，SDA由低拉高
 */
void Software_IIC_Stop(void)
{
    SOFT_IIC_SDA_L();
    SOFT_IIC_SCL_H();
    Software_IIC_Delay();

    SOFT_IIC_SDA_H();               /* SDA低->高 */
    Software_IIC_Delay();
}

/**
 * @brief  主机发送一个字节(高位在前)
 * @param  data: 待发送数据
 * @retval 0: 收到从机ACK   1: 从机NACK
 */
uint8_t Software_IIC_SendByte(uint8_t data)
{
    uint8_t i;

    for(i = 0; i < 8; i++)
    {
        if(data & 0x80)
            SOFT_IIC_SDA_H();
        else
            SOFT_IIC_SDA_L();
        data <<= 1;
        Software_IIC_Delay();

        SOFT_IIC_SCL_H();           /* 拉高时钟，从机采样 */
        Software_IIC_Delay();
        SOFT_IIC_SCL_L();           /* 拉低时钟，为下一位做准备 */
        Software_IIC_Delay();
    }

    return Software_IIC_WaitAck();  /* 等待从机应答 */
}

/**
 * @brief  等待从机应答
 * @retval 0: 收到ACK   1: 收到NACK
 */
uint8_t Software_IIC_WaitAck(void)
{
    uint8_t ack = 0;

    SOFT_IIC_SDA_H();               /* 释放SDA，交由从机控制 */
    Software_IIC_Delay();

    SOFT_IIC_SCL_H();
    Software_IIC_Delay();

    if(SOFT_IIC_SDA_READ())         /* SDA为高 => 从机未应答(NACK) */
        ack = 1;

    SOFT_IIC_SCL_L();
    Software_IIC_Delay();

    return ack;
}

/**
 * @brief  主机读取一个字节(高位在前)
 * @retval 读取到的数据
 */
uint8_t Software_IIC_ReadByte(void)
{
    uint8_t i, data = 0;

    SOFT_IIC_SDA_H();               /* 释放SDA，交由从机控制 */
    for(i = 0; i < 8; i++)
    {
        data <<= 1;
        SOFT_IIC_SCL_H();
        Software_IIC_Delay();

        if(SOFT_IIC_SDA_READ())     /* 采样SDA */
            data |= 0x01;

        SOFT_IIC_SCL_L();
        Software_IIC_Delay();
    }

    return data;
}

/**
 * @brief  主机发送应答ACK：在SCL高电平期间将SDA拉低
 */
void Software_IIC_Ack(void)
{
    SOFT_IIC_SDA_L();
    Software_IIC_Delay();

    SOFT_IIC_SCL_H();
    Software_IIC_Delay();
    SOFT_IIC_SCL_L();
    Software_IIC_Delay();

    SOFT_IIC_SDA_H();               /* 释放SDA */
    Software_IIC_Delay();
}

/**
 * @brief  主机发送非应答NACK：在SCL高电平期间保持SDA为高
 */
void Software_IIC_NAck(void)
{
    SOFT_IIC_SDA_H();
    Software_IIC_Delay();

    SOFT_IIC_SCL_H();
    Software_IIC_Delay();
    SOFT_IIC_SCL_L();
    Software_IIC_Delay();
}

/**
 * @brief  向从机指定寄存器写入数据
 * @param  dev_addr: 7位器件地址
 * @param  reg_addr: 寄存器地址
 * @param  pdata:    待写入数据缓冲区
 * @param  len:      数据长度
 * @retval 0: 成功   1: 失败(参数错误或从机NACK)
 */
uint8_t Software_IIC_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata, uint16_t len)
{
    uint16_t i;
    uint8_t ack;

    if(pdata == NULL || len == 0)
        return 1;

    Software_IIC_Start();
    ack = Software_IIC_SendByte((dev_addr << 1) | 0x00);    /* 写方向 */
    if(ack) { Software_IIC_Stop(); return 1; }

    ack = Software_IIC_SendByte(reg_addr);                  /* 寄存器地址 */
    if(ack) { Software_IIC_Stop(); return 1; }

    for(i = 0; i < len; i++)
    {
        ack = Software_IIC_SendByte(pdata[i]);              /* 写入数据 */
        if(ack) { Software_IIC_Stop(); return 1; }
    }

    Software_IIC_Stop();
    return 0;
}

/**
 * @brief  从从机指定寄存器读取数据
 * @param  dev_addr: 7位器件地址
 * @param  reg_addr: 寄存器地址
 * @param  pdata:    数据接收缓冲区
 * @param  len:      数据长度
 * @retval 0: 成功   1: 失败(参数错误或从机NACK)
 */
uint8_t Software_IIC_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *pdata, uint16_t len)
{
    uint16_t i;
    uint8_t ack;

    if(pdata == NULL || len == 0)
        return 1;

    Software_IIC_Start();
    ack = Software_IIC_SendByte((dev_addr << 1) | 0x00);    /* 写方向：发送寄存器地址 */
    if(ack) { Software_IIC_Stop(); return 1; }

    ack = Software_IIC_SendByte(reg_addr);
    if(ack) { Software_IIC_Stop(); return 1; }

    Software_IIC_Start();                                   /* 重复起始信号 */
    ack = Software_IIC_SendByte((dev_addr << 1) | 0x01);    /* 读方向 */
    if(ack) { Software_IIC_Stop(); return 1; }

    for(i = 0; i < len; i++)
    {
        pdata[i] = Software_IIC_ReadByte();
        if(i == (len - 1))
            Software_IIC_NAck();                            /* 最后一个字节发NACK */
        else
            Software_IIC_Ack();
    }

    Software_IIC_Stop();
    return 0;
}
