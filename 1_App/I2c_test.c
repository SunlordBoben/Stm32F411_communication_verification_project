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
	I2C1_ScanAddr();
	
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

	/* 原始整帧收发测试: ①发 AA 01 C0 00 2B 08 读7字节  ②发 AA 01 C1 00 28 08 读21字节 */
	SW2505_Test_RawCmdC0();
	// SW2505_Test_RawCmdC1();

}
