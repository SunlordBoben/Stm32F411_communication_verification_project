#include "dev_iic.h"
#include "dev_uart.h"
#include "printf.h"
#include "errno.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "stdint.h"


#include "app_software_iic.h"
#include "app_sw2505_i2cm.h"
#include "drv_iic.h"

/* ============ PB13 EXTI(跳变沿)测试: 中断计数+读电平, 主循环打印 ============ */
volatile uint32_t      gPb13ExtiCount  = 0;              /* 进入中断次数 */
volatile GPIO_PinState gPb13ExtiLevel  = GPIO_PIN_RESET; /* 中断里读取的电平 */
volatile uint8_t       gPb13ExtiFlag   = 0;              /* 有事件待打印 */

/**
  * @brief  PB13 EXTI 中断回调(HAL_GPIO_EXTI_IRQHandler 自动调用)
  * @note   中断上下文: 只做记录, 不能直接 printf(串口是中断发送会卡死/重入)
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_13)
    {
        gPb13ExtiCount++;                                   /* 记录次数 */
        gPb13ExtiLevel = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13); /* 读取当前电平 */
        gPb13ExtiFlag  = 1;                                 /* 置待打印标志 */
    }
}

/**
  * @brief  主循环轮询: 有待打印事件时打印次数+电平
  * @note   printf 依赖串口中断发送, 只能在主循环/任务里调用
  */
void PB13_Exti_PollPrint(void)
{
    if(0 != gPb13ExtiFlag)
    {
        gPb13ExtiFlag = 0;
        printf("[EXTI] PB13 IRQ #%d, level = %s\r\n",
               (unsigned int)gPb13ExtiCount,
               (GPIO_PIN_SET == gPb13ExtiLevel) ? "HIGH" : "LOW");
    }
}

/**
  * @brief  PB13 EXTI 测试入口: 使能NVIC后死循环等边沿并打印
  * @note   PB13 已在 gpio.c 配为 GPIO_MODE_IT_RISING(上升沿);
  *         想测双边沿请改成 GPIO_MODE_IT_RISING_FALLING
  */
void PB13_Exti_Test(void)
{

    printf("--- PB13 EXTI test: waiting rising edges... ---\r\n");
    while(1)
    {
        PB13_Exti_PollPrint();
    }
}

void I2c_test()
{
	//注册所有外设设备
	AllI2CDev_Regis();
	AllUARTDev_Regis();
	
	I2CDevice *pI2C = I2CDev_Find("I2C1");
	if(NULL != pI2C){
		printf("pI2C Ready\r\n");
	}
	
	if(ESUCCESS == pI2C->Init(pI2C)){
		printf("pI2C Init success!\r\n");
		pI2C->slave_addr = 0x3C;
	}
	
	/* ===== 试探/扫描总线上所有I2C设备地址(0x01~0x7F) ===== */
	//I2C1_ScanAddr();
	
	/* ===== 新驱动测试：pI2C->Write记录地址 + pI2C->Read组合读3字节 ===== */
	/* 从机复位后通信自恢复测试(拔插Type-C线模拟从机复位) */
	//SW2505_Test_SlaveResetRecovery();

	/* NTC温度表验证: 读0x46实时温度, 环境对比+加热/冷却验证曲线 */
	// SW2505_ReadVersion();
	//SW2505_Test_NtcTemperature(30);
	
	/* 对比测试: 不等待bit6就绪, 直接读0x39/0x3A */
	//SW2505_Test_ReadFuelParamsNoWait();

	/* 电量计参数读取测试(0x38读流程: bit7=1→轮询bit6→读0x39/0x3A) */
	//SW2505_Test_ReadFuelParams();

	/* 电量计参数写入测试(0x38写流程: 先写0x39/0x3A→写0x38=type(bit7=0)→轮询bit6) */
	//SW2505_Test_WriteFuelParams();
	//HAL_Delay(5000);
	/* 电量计参数读取测试(0x38读流程: bit7=1→轮询bit6→读0x39/0x3A) */
	//SW2505_Test_ReadFuelParams();

	


//	

	
	//SW2505_Test_ReadAllRegs();
	
	//SW2505_Test_WriteReadAll();
	
	//SW2505_Test_ReadOnlyProtection();
	
	//SW2505_Test_FifoOverflow();
	
	//SW2505_Test_BackToBackStress(1000000);

	/* 原始整帧收发测试: ①发 AA 01 C0 00 2B 08 读7字节  ②发 AA 01 C1 00 28 08 读22字节 */
	//SW2505_Test_RawCmdC0();
	//SW2505_Test_WriteRegAA();
	//HAL_Delay(100);
	//SW2505_Test_RawCmdC1();

	/* 往寄存器0xAA写13字节: 55 FF FF 06 00 55 AA 55 AA 5A A5 34 62 */
	//SW2505_Test_WriteRegAA();

	/* PB13 EXTI 跳变沿测试: 中断内计数+读电平, 主循环打印(阻塞, 不返回) */
	PB13_Exti_Test();

}


