#include "dev_iic.h"
#include "errno.h"
#include "i2c.h"
#include "printf.h"

static int I2CDrvInit(struct I2CDev *ptdev);
static int I2CDrvWrite(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
static int I2CDrvRead(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);


/* ================= 原驱动实现：IT/DMA 中断方式 ================= */
void I2C1WaitTxCplt(void);
void I2C1WaitRxCplt(void);
void I2C2WaitTxCplt(void);
void I2C2WaitRxCplt(void);

//等待I2C发送 | 接收标志位变量
static volatile unsigned char gI2C1TxCpltFlag = 0;
static volatile unsigned char gI2C1RxCpltFlag = 0;

static volatile unsigned char gI2C2TxCpltFlag = 0;
static volatile unsigned char gI2C2RxCpltFlag = 0;

/* I2C1_ScanAddr 侦查结果: 第1个找到的从机地址(7位), 0=未找到 */
static unsigned char gI2C1ScanDevAddr = 0;


struct I2CDev I2C1Dev = {
    .name = "I2C1", 
    .channel = 1,   
    .own_addr = 0,  
    .slave_addr = 0x3C,  //SW6208芯片IIC地址  
    .Init = I2CDrvInit, 
    .Write = I2CDrvWrite,   
    .Read = I2CDrvRead, 
    .next   = NULL, 
};

void I2CDev_Creat()
{
    I2CDev_Insert(&I2C1Dev);
}


static int I2CDrvInit(struct I2CDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;
    switch(ptdev->channel)
    {
        case 1:
        {
            ptdev->own_addr = hi2c1.Init.OwnAddress1;
            break;
        }
        case 2:case 3:break;
        default:break;
    }
    
    return ESUCCESS;
}

static int I2CDrvWrite(struct I2CDev *ptdev, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)   return -EINVAL;
    if(0 == length)   return -EINVAL;
    
    unsigned int init_len = length;
    
    switch(ptdev->channel)
    {
        case 1:
        {
            unsigned char *pbuf = buf;
            if(length == 1)
            {
                HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_IT(&hi2c1, ptdev->slave_addr<<1, buf, length);
                if(HAL_OK != status) {
									printf("error :%d\r\n",status);
									return -EIO;
								}
                I2C1WaitTxCplt();
                break;
            }
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(&hi2c1, ptdev->slave_addr<<1, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                I2C1WaitTxCplt();
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 2:case 3:break;
        default:break;
    }
    return (int)init_len;
}

static int I2CDrvRead(struct I2CDev *ptdev, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)   return -EINVAL;
    if(0 == length)   return -EINVAL;
    
    unsigned int init_len = length;
    
    switch(ptdev->channel)
    {
        case 1:
        {
            unsigned char *pbuf = buf;
            if(length == 1)
            {
                HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(&hi2c1, ptdev->slave_addr<<1, buf, length);
                if(HAL_OK != status) {
									printf("error :%d\r\n",status);
									return -EIO;
								}
                I2C1WaitRxCplt();
                break;
            }
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(&hi2c1, ptdev->slave_addr<<1, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                I2C1WaitRxCplt();
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 2:case 3:break;
        default:break;
    }
    return (int)init_len;
}

void I2C1WaitTxCplt(void)
{
    while(gI2C1TxCpltFlag != 1);
    gI2C1TxCpltFlag = 0;
}

void I2C2WaitTxCplt(void)
{
    while(gI2C2TxCpltFlag != 1);
    gI2C2TxCpltFlag = 0;
}

/* I2C发送中断函数 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        gI2C1TxCpltFlag = 1;
    }
		else if(hi2c->Instance == I2C2)
		{
			gI2C2TxCpltFlag = 1;
		}
}

void I2C1WaitRxCplt(void)
{
    while(gI2C1RxCpltFlag != 1);
    gI2C1RxCpltFlag = 0;
}

void I2C2WaitRxCplt(void)
{
    while(gI2C2RxCpltFlag != 1);
    gI2C2RxCpltFlag = 0;
}

/* I2C接收中断函数 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C1)
    {
        gI2C1RxCpltFlag = 1;
    }
		else if(hi2c->Instance == I2C2)
		{
				gI2C2RxCpltFlag = 1;
		}
}

/* ================= 组合读写封装(设备地址固定0x3C, 100kHz) ================= */
#define SW2505_DEV_ADDR     0x3C

/**
  * @brief  组合读寄存器：写寄存器地址 + repeated start + 读len字节
  * @note   需I2C1速率100kHz；新固件返回3字节协议 [地址回显, 数据, ~(地址+数据)校验和]
  * @param  regAddr: 寄存器地址
  * @param  buf:     接收缓冲区
  * @param  len:     读取字节数
  * @retval 成功返回实际读取字节数  失败返回负errno
  */
int I2C1_ReadReg(unsigned char regAddr, unsigned char *buf, unsigned int len)
{
    if(NULL == buf || 0 == len)     return -EINVAL;

    if(HAL_OK != HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(SW2505_DEV_ADDR << 1),
                                  regAddr, I2C_MEMADD_SIZE_8BIT,
                                  buf, (uint16_t)len, 100))
    {
        printf("I2C1 ReadReg(0x%02X) err\r\n", regAddr);
        return -EIO;
    }
    return (int)len;
}

/**
  * @brief  组合写寄存器：写寄存器地址 + 写len字节数据
  * @param  regAddr: 寄存器地址
  * @param  pData:   待写入数据
  * @param  len:     写入字节数
  * @retval 成功返回实际写入字节数  失败返回负errno
  */
int I2C1_WriteReg(unsigned char regAddr, unsigned char *pData, unsigned int len)
{
    if(NULL == pData || 0 == len)   return -EINVAL;

    if(HAL_OK != HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(SW2505_DEV_ADDR << 1),
                                   regAddr, I2C_MEMADD_SIZE_8BIT,
                                   pData, (uint16_t)len, 100))
    {
        printf("I2C1 WriteReg(0x%02X) err\r\n", regAddr);
        return -EIO;
    }
    return (int)len;
}

/* ============ 原始多字节整帧收发(无寄存器前缀, 阻塞式) ============ */

/**
  * @brief  原始多字节写：整帧作为数据直接发出(不带寄存器地址)
  * @note   用于发送自定义协议帧(如 AA 01 C0 00 2B 08)；
  *         目标从机地址由 devAddr 指定(7位, 可传 I2C1_GetScanAddr() 的扫描结果)
  * @param  devAddr: 目标从机7位地址
  * @param  buf:     待发送数据
  * @param  len:     发送字节数
  * @retval 成功返回实际发送字节数  失败返回负errno
  */
int I2C1_RawWrite(unsigned char devAddr, unsigned char *buf, unsigned int len)
{
    if(0 == devAddr || NULL == buf || 0 == len)     return -EINVAL;

    if(HAL_OK != HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(devAddr << 1),
                                         buf, (uint16_t)len, 100))
    {
        printf("I2C1 RawWrite err\r\n");
        return -EIO;
    }
    return (int)len;
}

/**
  * @brief  原始多字节读：直接从从机读len字节(不带寄存器地址)
  * @param  devAddr: 目标从机7位地址
  * @param  buf:     接收缓冲区
  * @param  len:     读取字节数
  * @retval 成功返回实际读取字节数  失败返回负errno
  */
int I2C1_RawRead(unsigned char devAddr, unsigned char *buf, unsigned int len)
{
    if(0 == devAddr || NULL == buf || 0 == len)     return -EINVAL;

    if(HAL_OK != HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(devAddr << 1),
                                        buf, (uint16_t)len, 100))
    {
        printf("I2C1 RawRead err\r\n");
        return -EIO;
    }
    return (int)len;
}

/* ================= I2C 地址探测/扫描(地址可自定义) ================= */

/**
  * @brief  探测指定7位地址是否有从机应答(ACK)
  * @note   仅发送地址帧检查ACK，不进行寄存器读写；
  *         使用HAL阻塞式IsDeviceReady，与上面IT/DMA收发互不干扰
  * @param  devAddr: 7位设备地址(有效范围0x01~0x7F)
  * @retval 1: 有设备应答  0: 无应答/参数非法
  */
int I2C1_ProbeAddr(unsigned char devAddr)
{
    if(0 == devAddr || devAddr > 0x7F)   return 0;

    if(HAL_OK == HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(devAddr << 1), 1, 100))
    {
        return 1;
    }
    return 0;
}

/**
  * @brief  扫描0x01~0x7F所有7位地址，打印有应答的设备
  * @note   打印格式：7位地址 / 8位写地址 / 8位读地址
  * @retval 找到的设备数量
  */
unsigned char I2C1_ScanAddr(void)
{
    unsigned char addr;
    unsigned char found = 0;

    printf("--- I2C1 scan 0x01~0x7F (7bit addr) ---\r\n");
    for(addr = 0x01; addr <= 0x7F; addr++)
    {
        if(1 == I2C1_ProbeAddr(addr))
        {
            /* 记录第1个找到的从机地址, 供后续 I2C1_GetScanAddr() 使用 */
            if(0 == gI2C1ScanDevAddr)
            {
                gI2C1ScanDevAddr = addr;
            }
            printf("Found: 7bit=0x%02X  (W=0x%02X / R=0x%02X)\r\n",
                   addr,
                   (unsigned char)(addr << 1),
                   (unsigned char)((addr << 1) | 1));
            found++;
        }
    }
    printf("--- scan done: %d device(s) found ---\r\n", found);
    return found;
}

/**
  * @brief  获取 I2C1_ScanAddr 侦查到的从机地址
  * @note   返回扫描过程中第1个有应答的7位地址；
  *         需先调用 I2C1_ScanAddr(), 未找到/未扫描返回0
  * @retval 从机7位地址  0: 未扫描或未找到设备
  */
unsigned char I2C1_GetScanAddr(void)
{
    return gI2C1ScanDevAddr;
}






