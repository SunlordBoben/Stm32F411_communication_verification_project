/**
  ******************************************************************************
  * @file    app_sw2505_i2cm.h
  * @brief   SW2505/SW7203 I2C 寄存器地址宏定义
  * @note    依据《SW2505_SW7203_从机寄存器手册V1.1.0》及从机工程寄存器定义：
  *          - 16位寄存器 = 两个相邻8位地址组合：低地址=低8位，高地址=高8位
  *          - 读协议：3字节 [地址回显, 数据, ~(地址+数据) 校验和]
  *          - 主控 I2C 速率需为 100kHz
  ******************************************************************************
  */
#ifndef __APP_SW2505_I2CM_H__
#define __APP_SW2505_I2CM_H__

#include "stm32f4xx_hal.h"

/* ==================== 仅读区域(该区域后续不可更改) ==================== */
#define SW2505_REG_RESERVE0                     0x00    /* Reg0x00 保留 */
#define SW2505_REG_VERSION                      0x01    /* Reg0x01 Register Version 寄存器版本 */
#define SW2505_REG_SINK_REC_SPR_SRC_CAP         0x02    /* Reg0x02 Sink接收SPR_Src_Cap信息 */
#define SW2505_REG_SINK_REC_EPR_SRC_CAP         0x03    /* Reg0x03 Sink接收EPR_Src_Cap信息 */

/* Sink Receive PD 固定电流(uint16: 低8位在前，高8位在后) */
#define SW2505_REG_SINK_REC_PD_5V_CURR          0x04    /* uint16: Reg0x04-0x05 5V固定电流 */
#define SW2505_REG_SINK_REC_PD_5V_CURR_L        0x04    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_5V_CURR_H        0x05    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_9V_CURR          0x06    /* uint16: Reg0x06-0x07 9V固定电流 */
#define SW2505_REG_SINK_REC_PD_9V_CURR_L        0x06    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_9V_CURR_H        0x07    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_12V_CURR         0x08    /* uint16: Reg0x08-0x09 12V固定电流 */
#define SW2505_REG_SINK_REC_PD_12V_CURR_L       0x08    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_12V_CURR_H       0x09    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_15V_CURR         0x0A    /* uint16: Reg0x0A-0x0B 15V固定电流 */
#define SW2505_REG_SINK_REC_PD_15V_CURR_L       0x0A    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_15V_CURR_H       0x0B    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_20V_CURR         0x0C    /* uint16: Reg0x0C-0x0D 20V固定电流 */
#define SW2505_REG_SINK_REC_PD_20V_CURR_L       0x0C    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_20V_CURR_H       0x0D    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_28V_CURR         0x0E    /* uint16: Reg0x0E-0x0F 28V固定电流 */
#define SW2505_REG_SINK_REC_PD_28V_CURR_L       0x0E    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_28V_CURR_H       0x0F    /* 高8位 */

#define SW2505_REG_SINK_REC_PD_PPS0_MIN_VOL     0x10    /* Reg0x10 Sink Receive PD PPS0最小电压 */
#define SW2505_REG_SINK_REC_PD_PPS0_MAX_VOL     0x11    /* Reg0x11 Sink Receive PD PPS0最大电压 */
#define SW2505_REG_SINK_REC_PD_PPS0_MAX_CURR    0x12    /* Reg0x12 Sink Receive PD PPS0最大电流 */

#define SW2505_REG_SINK_REC_PD_PPS1_MIN_VOL     0x13    /* Reg0x13 Sink Receive PD PPS1最小电压 */
#define SW2505_REG_SINK_REC_PD_PPS1_MAX_VOL     0x14    /* Reg0x14 Sink Receive PD PPS1最大电压 */
#define SW2505_REG_SINK_REC_PD_PPS1_MAX_CURR    0x15    /* Reg0x15 Sink Receive PD PPS1最大电流 */

#define SW2505_REG_SINK_REC_PD_PPS2_MIN_VOL     0x16    /* Reg0x16 Sink Receive PD PPS2最小电压 */
#define SW2505_REG_SINK_REC_PD_PPS2_MAX_VOL     0x17    /* Reg0x17 Sink Receive PD PPS2最大电压 */
#define SW2505_REG_SINK_REC_PD_PPS2_MAX_CURR    0x18    /* Reg0x18 Sink Receive PD PPS2最大电流 */

#define SW2505_REG_SINK_REC_AVS_POWER           0x19    /* Reg0x19 Sink Receive PD AVS最大功率 */
#define SW2505_REG_SINK_REC_PD_AVS_MIN_VOL      0x1A    /* uint16: Reg0x1A-0x1B AVS最小电压 */
#define SW2505_REG_SINK_REC_PD_AVS_MIN_VOL_L    0x1A    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_AVS_MIN_VOL_H    0x1B    /* 高8位 */
#define SW2505_REG_SINK_REC_PD_AVS_MAX_VOL      0x1C    /* uint16: Reg0x1C-0x1D AVS最大电压 */
#define SW2505_REG_SINK_REC_PD_AVS_MAX_VOL_L    0x1C    /* 低8位 */
#define SW2505_REG_SINK_REC_PD_AVS_MAX_VOL_H    0x1D    /* 高8位 */

/* ==================== 可读写区域(0x1E~0x2F) ==================== */
#define SW2505_REG_SINK_REQ_VOLTAGE             0x1E    /* uint16: Reg0x1E-0x1F Sink设置请求电压 */
#define SW2505_REG_SINK_REQ_VOLTAGE_L           0x1E    /* 低8位 */
#define SW2505_REG_SINK_REQ_VOLTAGE_H           0x1F    /* 高8位 */
#define SW2505_REG_SINK_REQ_CURRENT             0x20    /* uint16: Reg0x20-0x21 Sink设置请求电流 */
#define SW2505_REG_SINK_REQ_CURRENT_L           0x20    /* 低8位 */
#define SW2505_REG_SINK_REQ_CURRENT_H           0x21    /* 高8位 */
#define SW2505_REG_SINK_INPUT_MAX_POWER         0x22    /* Reg0x22 Sink输入最大功率设置 */
#define SW2505_REG_SRC_PD_SPR_SHIFT_CFG         0x23    /* Reg0x23 Source PD SPR挡位使能配置 */
#define SW2505_REG_SRC_PD_EPR_SHIFT_CFG         0x24    /* Reg0x24 Source PD EPR挡位使能配置 */
#define SW2505_REG_SRC_PD_OUTPUT_MAX_POWER      0x25    /* Reg0x25 Source PD输出最大功率 */
#define SW2505_REG_SRC_PD_PPS_LIMITE_CFG        0x26    /* Reg0x26 Source PD PPS恒功率配置 */
#define SW2505_REG_SRC_PD_PPS0_MIN_VOL          0x27    /* Reg0x27 Source PD PPS0最小电压 */
#define SW2505_REG_SRC_PD_PPS0_MAX_VOL          0x28    /* Reg0x28 Source PD PPS0最大电压 */
#define SW2505_REG_SRC_PD_PPS0_MAX_CURR         0x29    /* Reg0x29 Source PD PPS0最大电流 */
#define SW2505_REG_SRC_PD_PPS1_MIN_VOL          0x2A    /* Reg0x2A Source PD PPS1最小电压 */
#define SW2505_REG_SRC_PD_PPS1_MAX_VOL          0x2B    /* Reg0x2B Source PD PPS1最大电压 */
#define SW2505_REG_SRC_PD_PPS1_MAX_CURR         0x2C    /* Reg0x2C Source PD PPS1最大电流 */
#define SW2505_REG_SRC_PD_PPS2_MIN_VOL          0x2D    /* Reg0x2D Source PD PPS2最小电压 */
#define SW2505_REG_SRC_PD_PPS2_MAX_VOL          0x2E    /* Reg0x2E Source PD PPS2最大电压 */
#define SW2505_REG_SRC_PD_PPS2_MAX_CURR         0x2F    /* Reg0x2F Source PD PPS2最大电流 */

/* ==================== 仅读区域(0x30~0x35) ==================== */
#define SW2505_REG_TYPEC_STATE                  0x30    /* Reg0x30 TypeC_State(芯片工作状态) */
#define SW2505_REG_PORT_STATE_INDICATE          0x31    /* Reg0x31 端口状态指示(Sink/Source共用) */
#define SW2505_REG_HANDSHAKE_VOL                0x32    /* uint16: Reg0x32-0x33 Sink/Source握手协议电压 */
#define SW2505_REG_HANDSHAKE_VOL_L              0x32    /* 低8位 */
#define SW2505_REG_HANDSHAKE_VOL_H              0x33    /* 高8位 */
#define SW2505_REG_HANDSHAKE_CURR               0x34    /* uint16: Reg0x34-0x35 Sink/Source握手协议电流 */
#define SW2505_REG_HANDSHAKE_CURR_L             0x34    /* 低8位 */
#define SW2505_REG_HANDSHAKE_CURR_H             0x35    /* 高8位 */

/* ==================== 可读写区域(0x36~0x3C) ==================== */
#define SW2505_REG_WORK_MODE_CFG0               0x36    /* Reg0x36 SW2505工作模式0 */
#define SW2505_REG_WORK_MODE_CFG1               0x37    /* Reg0x37 SW2505工作模式1 */
#define SW2505_REG_GAUGE_PARAM_CTRL             0x38    /* Reg0x38 SW2505电量计参数读/写控制 */
#define SW2505_REG_GAUGE_PARAM_DATA             0x39    /* uint16: Reg0x39-0x3A SW2505电量计参数读/写数据 */
#define SW2505_REG_GAUGE_PARAM_DATA_L           0x39    /* 低8位 */
#define SW2505_REG_GAUGE_PARAM_DATA_H           0x3A    /* 高8位 */
#define SW2505_REG_TRANS_SW7203_REG_ADDR        0x3B    /* Reg0x3B SW2505透传SW7203寄存器地址 */
#define SW2505_REG_TRANS_SW7203_REG_DATA        0x3C    /* Reg0x3C SW2505透传SW7203寄存器数据 */

/* ==================== 仅读区域(0x3D~0x3E) ==================== */
#define SW2505_REG_ABN_STATE0                   0x3D    /* Reg0x3D SW2505异常状态指示0 */
#define SW2505_REG_ABN_STATE1                   0x3E    /* Reg0x3E SW2505异常状态指示1 */

/* ==================== 可读写区域(0x3F~0x45) ==================== */
#define SW2505_REG_SW7203_ABN_IRQ_CFG0          0x3F    /* Reg0x3F SW7203异常事件中断使能0 */
#define SW2505_REG_SW7203_ABN_IRQ_CFG1          0x40    /* Reg0x40 SW7203异常事件中断使能1 */
#define SW2505_REG_SW7203_ABN_STATE0            0x41    /* Reg0x41 SW7203异常状态指示0 */
#define SW2505_REG_SW7203_ABN_STATE1            0x42    /* Reg0x42 SW7203异常状态指示1 */
#define SW2505_REG_SYSTEM_DATA_READ_TYPE_CTRL   0x43    /* Reg0x43 SYSTEM数据读取类别控制 */
#define SW2505_REG_SYSTEM_DATA                 0x44    /* uint16: Reg0x44-0x45 SYSTEM数据 */
#define SW2505_REG_SYSTEM_DATA_L               0x44    /* 低8位 */
#define SW2505_REG_SYSTEM_DATA_H               0x45    /* 高8位 */

/* ==================== 仅读区域 ==================== */
#define SW2505_REG_NTC_TEMPERATURE              0x46    /* Reg0x46 SW2505 NTC温度 */

/**
  * @brief  读取寄存器0x01(版本)并校验
  * @note   组合读3字节 [地址回显, 寄存器值, ~(地址+值) 校验和]，校验通过后打印
  * @retval 0: 成功  1: 读取失败  2: 校验失败
  */
int SW2505_ReadVersion(void);

/**
  * @brief  通用校验读寄存器：组合读3字节并验证
  * @note   ① 地址回显==regAddr ② 校验和==~(地址+值)，通过后输出寄存器值
  * @param  regAddr: 寄存器地址
  * @param  pValue:  输出寄存器值
  * @retval 0: 校验通过  1: 读取失败  2: 校验失败
  */
int SW2505_ReadRegChecked(uint8_t regAddr, uint8_t *pValue);

/**
  * @brief  循环读取全部寄存器(0x00~0x46)测试，校验通过才打印
  * @retval 校验通过的寄存器数量
  */
uint8_t SW2505_Test_ReadAllRegs(void);

/**
  * @brief  可读写寄存器写后读回验证测试
  * @note   读原值→写取反→读回→对比→恢复原值，覆盖 0x1E~0x2F/0x36~0x3C/0x3F~0x45
  * @retval PASS(写生效)的寄存器数量
  */
uint8_t SW2505_Test_WriteReadAll(void);

/**
  * @brief  只读寄存器写保护验证测试
  * @note   读原值→写取反→读回→对比，覆盖 0x00~0x1D/0x30~0x35/0x3D~0x3E/0x46
  *         写入被忽略=>PROTECTED，写入生效=>NO protection(恢复原值)
  * @retval 保护生效(写入被忽略)的寄存器数量
  */
uint8_t SW2505_Test_ReadOnlyProtection(void);

/**
  * @brief  从机环形缓冲区队列溢出测试(0x36角色探针法)
  * @note   基线验证→连续写0x00填满64深度FIFO→写0x36探针→读0x30判断
  * @retval 0: 未溢出  1: 检测到溢出  2: 探针基线无效
  */
uint8_t SW2505_Test_FifoOverflow(void);

/**
  * @brief  背靠背读写压测(不改时钟)：验证从机中断处理能力
  * @note   保持100kHz，循环无延时连续读0x01(带校验)和写0x00各loops次，
  *          统计ok/fail/checksumErr+耗时(等效负载指标)
  * @param  loops: 每组事务次数(0则默认1000)
  * @retval 失败总次数(0=全部通过)
  */
uint16_t SW2505_Test_BackToBackStress(uint16_t loops);

/**
  * @brief  从机复位后通信自恢复测试
  * @note   寄存器表无复位寄存器、I2C无法触发从机软复位，
  *          采用"拔插Type-C线"(从机断电重启)模拟从机复位：
  *          读0x01验证正常→拔线检测到丢失→重插→自动恢复→
  *          验证version+TypeC状态(无需主机重启)
  * @retval 0: 恢复成功  1: 60s内未恢复
  */
uint8_t SW2505_Test_SlaveResetRecovery(void);

/**
  * @brief  NTC温度读取与温度曲线正确性验证 (reg 0x46)
  * @note   从机每次读0x46实时采样: bit7符号位+bit6-0温度(1℃/bit)
  *          表: JQ52B104F4250F08021, -5~60℃/2420~113mV
  *          环境对比+加热/冷却验证; 注意-5~0℃区间疑似从机bug(可能返60℃)
  * @param  samples: 采样次数(0=无限, 每1s一次)
  * @retval 0: 采样完成  1: 全部失败
  */
uint8_t SW2505_Test_NtcTemperature(uint16_t samples);

/**
  * @brief  电量计参数读取测试：逐条读13个参数 (0x38/0x39/0x3A)
  * @note   读流程: 写0x38=type|0x80(bit7=1读,bit6=0执行中)
  *         → 轮询bit6=1(从机数据就绪) → 读0x39/0x3A
  *         注意: 从机固件枚举与手册编号存在错位, 读回值语义不符时以固件为准
  * @retval 失败的类型数量(0=全部读成功)
  */
uint8_t SW2505_Test_ReadFuelParams(void);

/**
  * @brief  电量计参数写入测试：逐条写13个参数 (0x38/0x39/0x3A)
  * @note   写流程: 先写0x39/0x3A(低/高8位) → 写0x38=type(bit7=0写!)
  *         → 轮询bit6=1 → 读回0x39/0x3A确认; 非法type14应被拒绝
  * @retval 失败的类型数量(0=全部写成功)
  */
uint8_t SW2505_Test_WriteFuelParams(void);

/**
  * @brief  SW356x I2C OTA命令测试①：WriteReg(0xAA,{01,C0,00,2B,08}) 复位命令，读7字节ACK并校验
  * @note   0xAA=寄存器地址/SOF; 读须 I2C1_ReadReg(0xAA,...) 组合读
  *         正确ACK = AA 03 00 FF FF 3D 8C (CRC16=0x8C3D)
  * @warning 读回ACK后从机自动复位(进bootloader)
  * @retval 0: 成功(ACK正确)  1: 发送失败/未找到设备  2: 接收失败  3: ACK内容校验失败
  */
uint8_t SW2505_Test_RawCmdC0(void);

/**
  * @brief  SW356x I2C OTA命令测试②：WriteReg(0xAA,{01,C1,00,28,08})，读22字节并打印
  * @note   0xAA=寄存器地址/SOF; 读须 I2C1_ReadReg(0xAA,...) 组合读; C1 需从机固件支持
  * @retval 0: 成功  1: 发送失败/未找到设备  2: 接收失败
  */
uint8_t SW2505_Test_RawCmdC1(void);

/**
  * @brief  往寄存器0xAA写入13字节内容，读回并校验ACK
  * @note   数据: 55 FF FF 06 00 55 AA 55 AA 5A A5 34 62;
  *         正确ACK = AA 03 00 FF FF 3D 8C (CRC16=0x8C3D)
  * @retval 0: 成功(ACK正确)  1: 发送失败/未找到设备  2: 接收失败  3: ACK内容校验失败
  */
uint8_t SW2505_Test_WriteRegAA(void);

#endif /* __APP_SW2505_I2CM_H__ */
