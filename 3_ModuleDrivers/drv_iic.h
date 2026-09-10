#ifndef __DRV_IIC_H__
#define __DRV_IIC_H__

void I2CDev_Creat(void);

/* 组合读写封装(设备地址优先取I2C1_ScanAddr结果, 需100kHz, 未扫描退回0x3C) */
int I2C1_ReadReg(unsigned char regAddr, unsigned char *buf, unsigned int len);
int I2C1_WriteReg(unsigned char regAddr, unsigned char *pData, unsigned int len);

/* I2C地址探测/扫描(地址可自定义，7位地址) */
int I2C1_ProbeAddr(unsigned char devAddr);   /* 探测单个地址: 1=有应答 0=无 */
unsigned char I2C1_ScanAddr(void);           /* 扫描0x01~0x7F并打印, 返回找到数量 */
unsigned char I2C1_GetScanAddr(void);        /* 返回上次扫描找到的第1个从机地址(7位), 0=未找到 */

#endif
