#ifndef __DRV_IIC_H__
#define __DRV_IIC_H__

void I2CDev_Creat(void);

/* 组合读写封装(设备地址固定0x3C，需100kHz，新固件3字节读协议) */
int I2C1_ReadReg(unsigned char regAddr, unsigned char *buf, unsigned int len);
int I2C1_WriteReg(unsigned char regAddr, unsigned char *pData, unsigned int len);

/* 原始多字节整帧收发(无寄存器地址前缀, 目标地址由调用者传入, 阻塞式) */
int I2C1_RawWrite(unsigned char devAddr, unsigned char *buf, unsigned int len);
int I2C1_RawRead(unsigned char devAddr, unsigned char *buf, unsigned int len);

/* I2C地址探测/扫描(地址可自定义，7位地址) */
int I2C1_ProbeAddr(unsigned char devAddr);   /* 探测单个地址: 1=有应答 0=无 */
unsigned char I2C1_ScanAddr(void);           /* 扫描0x01~0x7F并打印, 返回找到数量 */
unsigned char I2C1_GetScanAddr(void);        /* 返回上次扫描找到的第1个从机地址(7位), 0=未找到 */

#endif
