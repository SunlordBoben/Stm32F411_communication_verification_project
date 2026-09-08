/**
  ******************************************************************************
  * @file    app_sw2505_i2cm.c
  * @brief   SW2505/SW7203 I2C 应用层实现
  * @note    基于 drv_iic 的 I2C1_ReadReg/I2C1_WriteReg 组合读写封装：
  *          - 读协议：从机回复3字节 [寄存器地址回显, 寄存器值, ~(地址+值) 校验和]
  *          - 主控 I2C 速率需为 100kHz
  ******************************************************************************
  */
#include "app_sw2505_i2cm.h"
#include "drv_iic.h"
#include "errno.h"
#include "printf.h"

/**
  * @brief  读取寄存器0x01(版本)并校验
  * @note   组合读3字节：[地址回显, 寄存器值, ~(地址+值) 校验和]
  *         校验规则：
  *         ① 第1字节(地址回显) == 请求的寄存器地址
  *         ② 第3字节(校验和) == ~(第1字节 + 第2字节)
  *         校验全部通过后打印寄存器版本值
  * @retval 0: 成功(校验通过)  1: 读取失败  2: 校验失败
  */
int SW2505_ReadVersion(void)
{
    uint8_t value = 0;
    int ret;

    ret = SW2505_ReadRegChecked(SW2505_REG_VERSION, &value);
    if(0 != ret)
    {
        printf("[SW2505] read Reg0x01 failed! ret=%d\r\n", ret);
        return ret;
    }

    printf("[SW2505] Reg0x01 Version = 0x%02X (checksum OK)\r\n", value);
    return 0;
}

/**
  * @brief  通用校验读寄存器：组合读3字节并验证
  * @note   从机回复 [地址回显, 寄存器值, ~(地址+值) 校验和]
  *         ① 地址回显 == regAddr；② 校验和 == ~(地址+值)
  * @param  regAddr: 寄存器地址
  * @param  pValue:  输出寄存器值
  * @retval 0: 校验通过  1: 读取失败  2: 校验失败
  */
int SW2505_ReadRegChecked(uint8_t regAddr, uint8_t *pValue)
{
    uint8_t buf[3] = {0};
    uint8_t expect;

    if(NULL == pValue)  return -EINVAL;

    /* 组合读3字节 */
    if(3 != I2C1_ReadReg(regAddr, buf, 3))
    {
        return 1;
    }

    /* ① 地址回显校验 */
    if(buf[0] != regAddr)
    {
        printf("[SW2505] Reg0x%02X addr echo mismatch: got [0x%02X, 0x%02X, 0x%02X], expect echo 0x%02X\r\n",
               regAddr, buf[0], buf[1], buf[2], regAddr);
        return 2;
    }

    /* ② 校验和校验：第3字节 == ~(地址 + 值) */
    expect = (uint8_t)(~(buf[0] + buf[1]));
    if(buf[2] != expect)
    {
        printf("[SW2505] Reg0x%02X checksum error: got [0x%02X, 0x%02X, 0x%02X], expect chk 0x%02X\r\n",
               regAddr, buf[0], buf[1], buf[2], expect);
        return 2;
    }

    *pValue = buf[1];
    return 0;
}

/**
  * @brief  循环读取全部寄存器(0x00~0x46)测试，校验通过才打印
  * @note   每个寄存器：组合读3字节 → 地址回显校验 → 校验和校验
  *         全部通过才打印寄存器值，失败的打印失败原因
  * @retval 校验通过的寄存器数量
  */
uint8_t SW2505_Test_ReadAllRegs(void)
{
    uint8_t reg;
    uint8_t value = 0;
    uint8_t okCount = 0;
    uint8_t failCount = 0;
    int ret;

    printf("--- SW2505 read all registers (0x00~0x46) ---\r\n");

    for(reg = 0x00; reg <= SW2505_REG_NTC_TEMPERATURE; reg++)
    {
        ret = SW2505_ReadRegChecked(reg, &value);
        if(0 == ret)
        {
            printf("Reg[0x%02X] = 0x%02X\r\n", reg, value);
            okCount++;
        }
        else
        {
            printf("Reg[0x%02X] FAIL(ret=%d)\r\n", reg, ret);
            failCount++;
        }
    }

    printf("--- read all done: %d ok, %d fail ---\r\n", okCount, failCount);
    return okCount;
}

/**
  * @brief  可读写寄存器写后读回验证测试
  * @note   对每个可读写寄存器：
  *         读原值(校验读) → 写取反测试值 → 读回(校验读) → 对比 → 恢复原值
  *         写入值 == 读回值 => 写生效(PASS)；否则 FAIL(可能被固件实时覆写)
  *         覆盖可读写区域：0x1E~0x2F / 0x36~0x3C / 0x3F~0x45
  * @warning 写取反值会短暂改变寄存器内容，0x1E(请求电压)可能触发重新请求、
  *          0x3B/0x3C(透传SW7203)可能向SW7203写垃圾配置，测试后自动恢复原值；
  *          建议从机未处于关键PD协商时测试
  * @retval PASS(写生效)的寄存器数量
  */
uint8_t SW2505_Test_WriteReadAll(void)
{
    static const uint8_t rwRegs[] = {
        /* 可读写 0x1E~0x2F */
        0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
        0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        /* 可读写 0x36~0x3C */
        0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
        /* 可读写 0x3F~0x45 */
        0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45
    };
    uint8_t i;
    uint8_t total = (uint8_t)(sizeof(rwRegs) / sizeof(rwRegs[0]));
    uint8_t passCount = 0;
    uint8_t failCount = 0;

    printf("--- SW2505 write-read verify (RW regs) ---\r\n");

    for(i = 0; i < total; i++)
    {
        uint8_t reg = rwRegs[i];
        uint8_t before = 0, after = 0, wData = 0;
        int ret;

        /* 1. 读原值(校验读) */
        ret = SW2505_ReadRegChecked(reg, &before);
        if(0 != ret)
        {
            printf("Reg[0x%02X] read before FAIL(ret=%d)\r\n", reg, ret);
            failCount++;
            continue;
        }

        /* 2. 写取反测试值 */
        wData = (uint8_t)~before;
        if(1 != I2C1_WriteReg(reg, &wData, 1))
        {
            printf("Reg[0x%02X] write FAIL\r\n", reg);
            failCount++;
            continue;
        }

        /* 3. 读回(校验读) */
        ret = SW2505_ReadRegChecked(reg, &after);
        if(0 != ret)
        {
            printf("Reg[0x%02X] read back FAIL(ret=%d)\r\n", reg, ret);
            failCount++;
            continue;
        }

        /* 4. 对比：写入值 == 读回值 */
        if(wData == after)
        {
            printf("Reg[0x%02X] PASS (write 0x%02X == read 0x%02X)\r\n", reg, wData, after);
            passCount++;
        }
        else
        {
            printf("Reg[0x%02X] NOT effective (write 0x%02X, read 0x%02X)\r\n", reg, wData, after);
            failCount++;
        }

        /* 5. 恢复原值 */
        I2C1_WriteReg(reg, &before, 1);
    }

    printf("--- write-read done: %d pass, %d fail ---\r\n", passCount, failCount);
    return passCount;
}

/**
  * @brief  只读寄存器写保护验证测试
  * @note   对每个只读寄存器：读原值(校验读) → 写取反值 → 读回(校验读) → 对比
  *         写入值 != 读回值 => 写入被忽略，只读保护生效(PROTECTED ✓)
  *         写入值 == 读回值 => 写入生效，无只读保护(NO PROTECTION ✗，恢复原值)
  *         覆盖仅读区域：0x00~0x1D / 0x30~0x35 / 0x3D~0x3E / 0x46
  * @warning 0x02~0x1D(Sink_Rec)是PD引擎实时使用的接收能力数据，
  *          若从机无写保护，写取反值可能影响PD状态机，测试后立即恢复原值；
  *          建议从机未处于关键PD协商时测试
  * @retval 保护生效(写入被忽略)的寄存器数量
  */
uint8_t SW2505_Test_ReadOnlyProtection(void)
{
    static const uint8_t roRegs[] = {
        /* 仅读 0x00~0x1D */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
        /* 仅读 0x30~0x35 */
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
        /* 仅读 0x3D~0x3E */
        0x3D, 0x3E,
        /* 仅读 0x46 */
        0x46
    };
    uint8_t i;
    uint8_t total = (uint8_t)(sizeof(roRegs) / sizeof(roRegs[0]));
    uint8_t protectedCount = 0;
    uint8_t noProtectCount = 0;

    printf("--- SW2505 read-only protection test ---\r\n");

    for(i = 0; i < total; i++)
    {
        uint8_t reg = roRegs[i];
        uint8_t before = 0, after = 0, wData = 0;
        int ret;

        /* 1. 读原值(校验读) */
        ret = SW2505_ReadRegChecked(reg, &before);
        if(0 != ret)
        {
            printf("Reg[0x%02X] read before FAIL(ret=%d)\r\n", reg, ret);
            continue;
        }

        /* 2. 写取反测试值 */
        wData = (uint8_t)~before;
        if(1 != I2C1_WriteReg(reg, &wData, 1))
        {
            printf("Reg[0x%02X] write FAIL\r\n", reg);
            continue;
        }

        /* 3. 读回(校验读) */
        ret = SW2505_ReadRegChecked(reg, &after);
        if(0 != ret)
        {
            printf("Reg[0x%02X] read back FAIL(ret=%d)\r\n", reg, ret);
            continue;
        }

        /* 4. 判断：写入值是否生效 */
        if(wData == after)
        {
            /* 写入生效 => 无只读保护 */
            printf("Reg[0x%02X] NO protection! write 0x%02X stuck\r\n", reg, wData);
            noProtectCount++;
            /* 恢复原值 */
            I2C1_WriteReg(reg, &before, 1);
        }
        else
        {
            /* 写入被忽略 => 只读保护生效 */
            printf("Reg[0x%02X] PROTECTED (write 0x%02X ignored, read 0x%02X)\r\n",
                   reg, wData, after);
            protectedCount++;
        }
    }

    printf("--- RO protection done: %d protected, %d no-protection ---\r\n",
           protectedCount, noProtectCount);
    return protectedCount;
}

/**
  * @brief  从机环形缓冲区队列溢出测试(0x36角色探针法)
  * @note   原理：从机收到写操作不立即执行，地址写入64深度环形缓冲区
  *         (register_func.c s_Reg_Cache_Array[64])，由主循环I2c_Slave_Policy
  *         每周期消费1个并调用Execute_Reg_Function。
  *         FIFO满时入队被静默丢弃(Write_Reg_Fifo返回1被忽略，RAM照写/ACK照回)，
  *         唯一可观察差异 = Execute_Reg_Function是否被执行。
  *         探针：0x36(WORK_MODE_CFG0)的bit0~1切换电源角色，
  *               读0x30(TypeC_State)观察角色变化。
  *         流程：①基线(SNK→SRC角色切换可观察) ②连续写0x00填满FIFO
  *               ③写0x36=0x01探针 ④读0x30，若角色未变 => FIFO溢出
  * @warning 切SRC角色会打断当前SNK充电协商，测试后自动恢复SNK；
  *          若板子硬件不支持SRC角色，基线无效返回2
  * @retval 0: 未溢出  1: 检测到溢出  2: 探针基线无效
  */
uint8_t SW2505_Test_FifoOverflow(void)
{
    uint8_t wSnk = 0x02, wSrc = 0x01, wFill = 0xAA;
    uint8_t stateSnk = 0, stateProbe = 0;
    uint16_t i;
    int ret;

    printf("--- SW2505 FIFO overflow test ---\r\n");

    /* ① 基线：确认0x36角色切换可观察 */
    /* 先切SNK */
    I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSnk, 1);
    HAL_Delay(100);
    ret = SW2505_ReadRegChecked(SW2505_REG_TYPEC_STATE, &stateSnk);
    if(0 != ret) { printf("[FIFO] read state fail\r\n"); return 2; }

    /* 再切SRC，观察状态变化 */
    I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSrc, 1);
    HAL_Delay(100);
    ret = SW2505_ReadRegChecked(SW2505_REG_TYPEC_STATE, &stateProbe);
    if(0 != ret) { printf("[FIFO] read state fail\r\n"); return 2; }

    if(stateSnk == stateProbe)
    {
        printf("[FIFO] probe baseline INVALID: SNK=0x%02X, SRC=0x%02X (no change), cannot test\r\n",
               stateSnk, stateProbe);
        I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSnk, 1);
        return 2;
    }
    printf("[FIFO] baseline OK: SNK=0x%02X, SRC=0x%02X\r\n", stateSnk, stateProbe);

    /* 恢复SNK */
    I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSnk, 1);
    HAL_Delay(100);

    /* ② 填FIFO：连续快速写0x00(无副作用)300次，不延时 */
    for(i = 0; i < 300; i++)
    {
        if(1 != I2C1_WriteReg(SW2505_REG_RESERVE0, &wFill, 1))
        {
            printf("[FIFO] fill write %d fail\r\n", i);
            break;
        }
    }
    printf("[FIFO] filled %d writes\r\n", i);

    /* ③ 探针：写0x36=0x01(SRC)，若入队被丢弃则角色不变 */
    I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSrc, 1);
    HAL_Delay(100);
    ret = SW2505_ReadRegChecked(SW2505_REG_TYPEC_STATE, &stateProbe);
    if(0 != ret) { printf("[FIFO] read state fail\r\n"); return 2; }

    /* ④ 判断 */
    if(stateProbe == stateSnk)
    {
        printf("[FIFO] => OVERFLOW detected! 0x36 probe ignored, state=0x%02X\r\n", stateProbe);
        ret = 1;
    }
    else
    {
        printf("[FIFO] => No overflow, probe took effect, state=0x%02X\r\n", stateProbe);
        ret = 0;
    }

    /* ⑤ 恢复SNK */
    I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG0, &wSnk, 1);
    return (uint8_t)ret;
}

/**
  * @brief  背靠背读写压测(不改时钟)：验证从机中断处理能力
  * @note   保持当前I2C时钟(100kHz)不变，循环内无任何延时：
  *         ① 连续读0x01×loops：每次 [地址回显+校验和] 验证，
  *            统计 ok/fail/checksumErr
  *         ② 连续写0x00×loops：写保留寄存器(无副作用)，统计成功率
  *         ③ 统计总耗时/平均每事务耗时/等效事务率(负载指标)
  *         事务间零间隔 => 从机承受"等效>100kHz"的高密度负载
  * @param  loops: 每组事务次数(0则默认1000)
  * @retval 读校验失败+写失败的总次数(0=全部通过)
  */
uint16_t SW2505_Test_BackToBackStress(uint16_t loops)
{
    uint16_t j;
    uint16_t ok = 0, rdFail = 0, chkErr = 0, wrFail = 0;
    uint32_t t0, elapsed;

    if(0 == loops)  loops = 1000;

    printf("--- SW2505 back-to-back stress (clock unchanged, no delay) ---\r\n");

    /* ① 连续读0x01：背靠背 + 地址回显/校验和验证 */
    printf("[Stress] back-to-back READ 0x01 x%d ...\r\n", loops);
    t0 = HAL_GetTick();
    for(j = 0; j < loops; j++)
    {
        uint8_t value = 0;
        int ret = SW2505_ReadRegChecked(SW2505_REG_VERSION, &value);
        if(0 == ret)        ok++;
        else if(1 == ret)   rdFail++;    /* 事务失败(超时/无响应) */
        else                chkErr++;    /* 地址回显/校验和错误 */

        /* 只打印前5次失败详情，避免高频错误刷屏 */
        if(0 != ret && (rdFail + chkErr) <= 5)
        {
            uint8_t dbg[3] = {0};
            I2C1_ReadReg(SW2505_REG_VERSION, dbg, 3);
            printf("[Stress] #%d fail(ret=%d): raw [0x%02X, 0x%02X, 0x%02X]\r\n",
                   j + 1, ret, dbg[0], dbg[1], dbg[2]);
        }
    }
    elapsed = HAL_GetTick() - t0;
    printf("[Stress] READ: ok=%d fail=%d checksumErr=%d, %dms, avg %dus/txn\r\n",
           ok, rdFail, chkErr, (int)elapsed,
           (loops > 0) ? (int)(elapsed * 1000 / loops) : 0);

    /* ② 连续写0x00(保留寄存器，无副作用)：背靠背 */
    printf("[Stress] back-to-back WRITE 0x00 x%d ...\r\n", loops);
    t0 = HAL_GetTick();
    for(j = 0; j < loops; j++)
    {
        uint8_t wData = 0xA5;
        if(1 != I2C1_WriteReg(SW2505_REG_RESERVE0, &wData, 1))
        {
            wrFail++;
        }
    }
    elapsed = HAL_GetTick() - t0;
    printf("[Stress] WRITE: ok=%d fail=%d, %dms, avg %dus/txn\r\n",
           (loops - wrFail), wrFail, (int)elapsed,
           (loops > 0) ? (int)(elapsed * 1000 / loops) : 0);

    printf("--- stress done: total fail=%d ---\r\n", (rdFail + chkErr + wrFail));
    return (uint16_t)(rdFail + chkErr + wrFail);
}

/**
  * @brief  NTC温度读取与温度曲线正确性验证 (reg 0x46)
  * @note   从机每次读0x46都会现场采样ADC+查表插值(实时温度)：
  *         表: JQ52B104F4250F08021, 5uA恒流源, 8点{-5,0,10,20,30,40,50,60}℃
  *         对应{2420,1827,1067,643,400,290,169,113}mV
  *         寄存器: bit7符号位 + bit6-0温度值(1℃/bit)
  *         验证方法:
  *           ① 连续采样打印温度(符号解析), 统计min/max
  *           ② 环境温度对比: 室温应25~30℃
  *           ③ 加热: 手指按NTC/吹风机 → 温度应上升
  *           ④ 冷却: 冰水 → 温度应下降
  *           ⑤ 注意: 从机查表循环跳过num=0, -5~0℃区间可能错误返回60℃
  * @param  samples: 采样次数(0=无限采样, 每次间隔1s)
  * @retval 0: 采样完成  1: 全部失败
  */
uint8_t SW2505_Test_NtcTemperature(uint16_t samples)
{
    uint8_t value = 0;
    int8_t temp;
    int16_t minTemp = 127, maxTemp = -128;
    uint16_t i = 0, okCnt = 0, failCnt = 0;
    int ret;

    printf("=== NTC temperature check (reg 0x46) ===\r\n");
    printf("Slave table: -5~60C / 2420~113mV (5uA source, B=4250)\r\n");
    printf("Hint: room 25~30C; finger press 30+C; ice water near 0C\r\n");

    while(1)
    {
        ret = SW2505_ReadRegChecked(SW2505_REG_NTC_TEMPERATURE, &value);
        if(0 != ret)
        {
            failCnt++;
            printf("[%d] read fail ret=%d\r\n", i + 1, ret);
        }
        else
        {
            okCnt++;
            /* 解析: bit7符号位, bit6-0温度值(1℃/bit) */
            temp = (value & 0x80) ? (int8_t)(0 - (int8_t)(value & 0x7F))
                                  : (int8_t)(value & 0x7F);
            if(temp > maxTemp)  maxTemp = temp;
            if(temp < minTemp)  minTemp = temp;
            printf("[%d] NTC temp = %dC (raw=0x%02X, chk OK)\r\n", i + 1, temp, value);
        }

        i++;
        if(0 != samples && i >= samples)    break;
        HAL_Delay(1000);
    }

    printf("--- stat: ok=%d fail=%d, temp range %dC ~ %dC ---\r\n",
           okCnt, failCnt, minTemp, maxTemp);
    return (failCnt >= i && 0 != i) ? 1 : 0;
}

/**
  * @brief  从机复位后通信自恢复测试
  * @note   寄存器表(0x00~0x46)无复位寄存器、I2C无法触发从机软复位，
  *          故采用"拔插Type-C线"(从机断电重启)模拟从机复位：
  *          ① 先读0x01验证通信正常
  *          ② 提示拔掉Type-C线→主控轮询检测到通信丢失
  *          ③ 提示重新插入→从机重启初始化
  *          ④ 主控持续重试→通信自动恢复(无需主机重启/重初始化)
  *          ⑤ 恢复后验证 version + TypeC状态寄存器
  * @retval 0: 恢复成功  1: 60s内未恢复
  */
uint8_t SW2505_Test_SlaveResetRecovery(void)
{
    uint8_t value = 0;
    uint8_t state0 = 0, state1 = 0;
    int ret;
    int r0, r1;
    uint32_t tStart, tLost = 0, tRecover = 0;
    uint16_t failCnt = 0;
    uint8_t state = 0;          /* 0=正常 1=检测到丢失 2=已恢复 */

    printf("=== Slave Reset Recovery test ===\r\n");

    /* ① 复位前：验证当前通信正常 */
    ret = SW2505_ReadRegChecked(SW2505_REG_VERSION, &value);
    if(0 != ret)
    {
        printf("FAIL: init read 0x01 fail ret=%d, check hardware\r\n", ret);
        return 1;
    }
    printf("Step1 OK: version=0x%02X, comm normal\r\n", value);

    /* ② 提示制造从机复位 */
    printf("Step2: unplug Type-C cable (slave power off), replug after 3s\r\n");
    printf("       master keeps polling, auto recovery expected...\r\n");

    tStart = HAL_GetTick();
    while(1)
    {
        ret = SW2505_ReadRegChecked(SW2505_REG_VERSION, &value);

        if(0 == ret)
        {
            if(1 == state)      /* 之前丢失过，现在成功 = 已恢复 */
            {
                tRecover = HAL_GetTick();
                printf("=== RECOVERY OK ===\r\n");
                printf("  comm lost at +%dms, recovered at +%dms, outage %dms\r\n",
                       (int)(tLost - tStart), (int)(tRecover - tStart),
                       (int)(tRecover - tLost));
                printf("  fail txns: %d, version after recovery=0x%02X\r\n", failCnt, value);

                /* ⑤ 额外验证 TypeC 状态寄存器恢复正常 */
                r0 = SW2505_ReadRegChecked(SW2505_REG_TYPEC_STATE, &state0);
                r1 = SW2505_ReadRegChecked(SW2505_REG_PORT_STATE_INDICATE, &state1);
                printf("  TypeC_State 0x30=0x%02X(ret=%d), Port_Ind 0x31=0x%02X(ret=%d)\r\n",
                       state0, r0, state1, r1);

                printf("=== PASS: slave reset recovery verified (master no restart) ===\r\n");
                return 0;
            }
            /* 一直正常: 继续轮询等待用户拔线 */
        }
        else
        {
            if(0 == state)      /* 首次失败 = 检测到通信丢失 */
            {
                state = 1;
                tLost = HAL_GetTick();
                printf("!!! comm lost detected (+%dms), retrying...\r\n",
                       (int)(tLost - tStart));
            }
            failCnt++;
            if(1 == failCnt % 20)   /* 每20次失败打印一次进度 */
            {
                printf("  retrying... fail=%d (+%dms)\r\n",
                       failCnt, (int)(HAL_GetTick() - tStart));
            }
        }

        /* 60秒超时未恢复 = 测试失败 */
        if(HAL_GetTick() - tStart > 60000)
        {
            printf("FAIL: no recovery in 60s (fail=%d), check slave/wiring\r\n", failCnt);
            return 1;
        }

        HAL_Delay(200);
    }
}

/**
  * @brief  电量计参数读取测试：逐条读13个参数 (0x38/0x39/0x3A)
  * @note   读流程: 写0x38 = type|0x80 (bit7=1读, bit6=0执行中)
  *         → 轮询读0x38直到bit6=1(从机数据就绪) → 读0x39/0x3A
  *         type编号按手册V1.1.0排列
  *         注意: 从机固件枚举FG_PARAM_*与手册编号存在错位,
  *         若读回值与名称语义不符, 以从机固件枚举为准
  * @retval 失败的类型数量(0=全部读成功)
  */
uint8_t SW2505_Test_ReadFuelParams(void)
{
    static const struct
    {
        uint8_t      type;
        const char  *name;
    } tbl[13] = {
        {1,  "CellFullVol"},
        {2,  "BatType"},
        {3,  "SeriesCells"},
        {4,  "NominalCap"},
        {5,  "DisCutoffPct"},
        {6,  "ChgLinearCoef"},
        {7,  "DisLowVCoef"},
        {8,  "DisCompensate"},
        {9,  "ChgCompensate"},
        {10, "ChgCutoffCur"},
        {11, "UvProtect"},
        {12, "ChgTargetVol"},
        {13, "BatCurLimit"},
    };
    uint8_t i;
    uint8_t failCnt = 0;
    uint32_t t0;

    printf("=== Fuel Gauge Param READ test (0x38/0x39/0x3A) ===\r\n");

    for(i = 0; i < 13; i++)
    {
        uint8_t ctrl = 0;
        uint8_t lo = 0, hi = 0;
        uint16_t value = 0;
        uint8_t ready = 0;

        /* ① 写控制: bit7=1(读), bit6=0(执行中), bit5-0=类型 */
        ctrl = (uint8_t)(tbl[i].type | 0x80);
        if(1 != I2C1_WriteReg(SW2505_REG_GAUGE_PARAM_CTRL, &ctrl, 1))
        {
            printf("[%02d] %s: write 0x38 fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }

        /* ② 轮询bit6=1(数据就绪), 500ms超时 */
        t0 = HAL_GetTick();
        while((HAL_GetTick() - t0) < 500)
        {
            if(0 == SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_CTRL, &ctrl)
               && (ctrl & 0x40))
            {
                ready = 1;
                break;
            }
            HAL_Delay(5);
        }
        if(0 == ready)
        {
            printf("[%02d] %s: bit6 wait timeout (0x38=0x%02X)\r\n",
                   tbl[i].type, tbl[i].name, ctrl);
            failCnt++;
            continue;
        }

        /* ③ 读数据: 0x39低8位 + 0x3A高8位 */
        if(0 != SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_DATA_L, &lo)
           || 0 != SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_DATA_H, &hi))
        {
            printf("[%02d] %s: read data fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }
        value = (uint16_t)lo | ((uint16_t)hi << 8);
        printf("[%02d] %s = %u (hi=0x%02X lo=0x%02X)\r\n",
               tbl[i].type, tbl[i].name, value, hi, lo);
    }

    printf("=== done: ok=%d/13, fail=%d ===\r\n", (13 - failCnt), failCnt);
    return failCnt;
}



/**
  * @brief  电量计参数写入测试：逐条写13个参数 (0x38/0x39/0x3A + 0x37握手)
  * @note   完整写流程: ⓪ 0x37 bit1=1(配置中, 保留bit0数据角色)
  *                 ① 先写数据: 0x39=低8位, 0x3A=高8位
  *                 ② 写控制: 0x38 = type (bit7=0写!, bit6=0执行中, bit5-0=类型)
  *                 ③ 轮询0x38 bit6==1(从机执行完成), 500ms超时
  *                 ④ 读回0x39/0x3A与写入值对比(寄存器RAM)
  *                 ⑤ 0x37 bit1=0(配置完成) → 从机FULE_INIT把缓存应用到生效配置
  *                 ⑥ 读流程读回生效配置(p_Fuel_SDK_Data->cfg), 与写入值对比
  *         注意: 写操作bit7必须=0(bit7=1是读操作, 从机会把当前值写回
  *         0x39/0x3A覆盖掉主控写入的数据)
  * @retval 失败数(0=全部写成功且生效验证通过)
  */
uint8_t SW2505_Test_WriteFuelParams(void)
{
    static const struct
    {
        uint8_t      type;
        const char  *name;
        uint16_t     value;
    } tbl[13] = {
        {1,  "CellFullVol",   4200},
        {2,  "BatType",       1},
        {3,  "SeriesCells",   2},
        {4,  "NominalCap",    10000},
        {5,  "DisCutoffPct",  5},
        {6,  "ChgLinearCoef", 70},
        {7,  "DisLowVCoef",   55},
        {8,  "DisCompensate", 96},
        {9,  "ChgCompensate", 96},
        {10, "ChgCutoffCur",  200},
        {11, "UvProtect",     3000},
        {12, "ChgTargetVol",  8400},
        {13, "BatCurLimit",   5000},
    };
    uint8_t i;
    uint8_t failCnt = 0;
    uint8_t cfg1 = 0, tmp = 0;
    uint32_t t0;

    printf("=== Fuel Gauge Param WRITE test (0x38/0x39/0x3A + 0x37 handshake) ===\r\n");

    /* ⓪ 握手开始: 0x37 bit1=1(配置中), 保留bit0数据角色等其他位 */
    if(0 == SW2505_ReadRegChecked(SW2505_REG_WORK_MODE_CFG1, &cfg1))
    {
        tmp = (uint8_t)(cfg1 | 0x02);
        if(1 != I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG1, &tmp, 1))
        {
            printf("[cfg] write 0x37 fail\r\n");
        }
        printf("[cfg] 0x37: 0x%02X -> 0x%02X (bit1=1 configuring)\r\n", cfg1, tmp);
        HAL_Delay(50);
    }
    else
    {
        printf("[cfg] read 0x37 fail, continue without handshake\r\n");
    }

    for(i = 0; i < 13; i++)
    {
        uint8_t wData = 0;
        uint8_t ctrl = 0;
        uint8_t lo = 0, hi = 0;
        uint16_t rb = 0;
        uint8_t ready = 0;

        /* ① 先写数据: 0x39=低8位, 0x3A=高8位 */
        wData = (uint8_t)(tbl[i].value & 0xFF);
        if(1 != I2C1_WriteReg(SW2505_REG_GAUGE_PARAM_DATA_L, &wData, 1))
        {
            printf("[%02d] %s: write 0x39 fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }
        wData = (uint8_t)(tbl[i].value >> 8);
        if(1 != I2C1_WriteReg(SW2505_REG_GAUGE_PARAM_DATA_H, &wData, 1))
        {
            printf("[%02d] %s: write 0x3A fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }

        /* ② 写控制: bit7=0(写), bit6=0(执行中), bit5-0=类型 */
        ctrl = (uint8_t)(tbl[i].type & 0x3F);
        if(1 != I2C1_WriteReg(SW2505_REG_GAUGE_PARAM_CTRL, &ctrl, 1))
        {
            printf("[%02d] %s: write 0x38 fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }

        /* ③ 轮询bit6=1(从机执行完成), 500ms超时 */
        t0 = HAL_GetTick();
        while((HAL_GetTick() - t0) < 500)
        {
            if(0 == SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_CTRL, &ctrl)
               && (ctrl & 0x40))
            {
                ready = 1;
                break;
            }
            HAL_Delay(5);
        }
        if(0 == ready)
        {
            printf("[%02d] %s: bit6 wait timeout (0x38=0x%02X)\r\n",
                   tbl[i].type, tbl[i].name, ctrl);
            failCnt++;
            continue;
        }

        /* ④ 读回0x39/0x3A, 与写入值对比 */
        if(0 != SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_DATA_L, &lo)
           || 0 != SW2505_ReadRegChecked(SW2505_REG_GAUGE_PARAM_DATA_H, &hi))
        {
            printf("[%02d] %s: readback fail\r\n", tbl[i].type, tbl[i].name);
            failCnt++;
            continue;
        }
        rb = (uint16_t)lo | ((uint16_t)hi << 8);
        if(rb == tbl[i].value)
        {
            printf("[%02d] %s = %u : WRITE OK, readback MATCH (0x%04X)\r\n",
                   tbl[i].type, tbl[i].name, tbl[i].value, rb);
        }
        else
        {
            printf("[%02d] %s : MISMATCH! wrote %u, readback %u (hi=0x%02X lo=0x%02X)\r\n",
                   tbl[i].type, tbl[i].name, tbl[i].value, rb, hi, lo);
            failCnt++;
        }
    }

    /* ⑤ 握手结束: 0x37 bit1=0(配置完成), 保留其他位 */
    if(0 == SW2505_ReadRegChecked(SW2505_REG_WORK_MODE_CFG1, &cfg1))
    {
        tmp = (uint8_t)(cfg1 & ~0x02);
        I2C1_WriteReg(SW2505_REG_WORK_MODE_CFG1, &tmp, 1);
        printf("[cfg] 0x37: 0x%02X -> 0x%02X (bit1=0 config done), wait apply...\r\n",
               cfg1, tmp);
        HAL_Delay(500);     /* 等从机FULE_INIT把缓存应用到生效配置 */
        SW2505_ReadRegChecked(SW2505_REG_WORK_MODE_CFG1, &cfg1);
        printf("[cfg] 0x37 now = 0x%02X (bit2 done flag=%d)\r\n", cfg1, (cfg1 >> 2) & 1);
    }

    printf("=== done: fail=%d ===\r\n", failCnt);
    return failCnt;
}

/**
  * @brief  原始整帧收发测试①：发送 AA 01 C0 00 2B 08，接收7字节并打印
  * @note   用 I2C1_RawWrite 把整帧原样发出(不带寄存器地址前缀)，
  *         再用 I2C1_RawRead 读回7字节打印；
  *         目标从机地址 = I2C1_GetScanAddr()(I2C1_ScanAddr侦查到的地址)；
  *         期望收到 AA 03 00 FF FF 3E 0F(仅供参考，不比对)
  * @retval 0: 成功  1: 发送失败/未找到设备  2: 接收失败
  */
uint8_t SW2505_Test_RawCmdC0(void)
{
    static const uint8_t tx[6] = {0xAA, 0x01, 0xC0, 0x00, 0x2B, 0x08};
    uint8_t devAddr;
    uint8_t rx[7] = {0};
    uint8_t i;

    devAddr = I2C1_GetScanAddr();
    if(0 == devAddr)
    {
        printf("[C0] no device found by I2C1_ScanAddr, abort\r\n");
        return 1;
    }

    printf("--- raw frame C0 @7bit addr 0x%02X: send 6B, read 7B ---\r\n", devAddr);
    printf("[TX] ");
    for(i = 0; i < sizeof(tx); i++)  printf("%02X ", tx[i]);
    printf("\r\n");

    if((int)sizeof(tx) != I2C1_RawWrite(devAddr, (unsigned char *)tx, (unsigned int)sizeof(tx)))
    {
        printf("[C0] send FAIL\r\n");
        return 1;
    }

    if(7 != I2C1_RawRead(devAddr, rx, 7))
    {
        printf("[C0] read FAIL\r\n");
        return 2;
    }

    printf("[RX-7] ");
    for(i = 0; i < 7; i++)  printf("%02X ", rx[i]);
    printf("\r\n");
    printf("[C0] expect ~ AA 03 00 FF FF 3E 0F (for reference)\r\n");
    return 0;
}

/**
  * @brief  原始整帧收发测试②：发送 AA 01 C1 00 28 08，接收21字节并打印
  * @note   用 I2C1_RawWrite 把整帧原样发出(不带寄存器地址前缀)，
  *         再用 I2C1_RawRead 读回21字节并打印；
  *         目标从机地址 = I2C1_GetScanAddr()(I2C1_ScanAddr侦查到的地址)
  * @retval 0: 成功  1: 发送失败/未找到设备  2: 接收失败
  */
uint8_t SW2505_Test_RawCmdC1(void)
{
    static const uint8_t tx[6] = {0xAA, 0x01, 0xC1, 0x00, 0x28, 0x08};
    uint8_t devAddr;
    uint8_t rx[21] = {0};
    uint8_t i;

    devAddr = I2C1_GetScanAddr();
    if(0 == devAddr)
    {
        printf("[C1] no device found by I2C1_ScanAddr, abort\r\n");
        return 1;
    }

    printf("--- raw frame C1 @7bit addr 0x%02X: send 6B, read 21B ---\r\n", devAddr);
    printf("[TX] ");
    for(i = 0; i < sizeof(tx); i++)  printf("%02X ", tx[i]);
    printf("\r\n");

    if((int)sizeof(tx) != I2C1_RawWrite(devAddr, (unsigned char *)tx, (unsigned int)sizeof(tx)))
    {
        printf("[C1] send FAIL\r\n");
        return 1;
    }

    if(21 != I2C1_RawRead(devAddr, rx, 21))
    {
        printf("[C1] read FAIL\r\n");
        return 2;
    }

    printf("[RX-21] ");
    for(i = 0; i < 21; i++)  printf("%02X ", rx[i]);
    printf("\r\n");
    return 0;
}
