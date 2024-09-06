#include "rm3100.h"
#include "stm32f1xx.h"
#include "stdio.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;


/**
 * @brief 读取 RM3100 设备的 ID
 *
 * 该函数通过 I2C 接口读取指定地址的 RM3100 设备的 ID。
 *
 * @param address 设备的 I2C 地址
 * @param hi2c I2C 句柄
 * @return uint8_t 返回 0 表示成功，返回 1 表示失败
 */
uint8_t RM3100ReadID(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t data = 0; // 用于存储读取到的数据
    uint8_t addr_hshake = ADDR_HSHAKE; // 握手地址

    // 发送握手地址到指定的 I2C 设备
    HAL_StatusTypeDef status1 = HAL_I2C_Master_Transmit(&hi2c, address, &addr_hshake, 1, 1000);
    // 从指定的 I2C 设备读取一个字节的数据
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c, (address | 1), &data, 1, 1000);

    // 检查读取操作是否成功
    if (status != HAL_OK) {
        // 打印 I2C 错误信息
        PrintI2CError(&hi2c);
        return 1; // 1 - 失败
    } else {
        // 检查读取到的数据是否为预期的 ID 值
        if (data != 0x1B) {
            printf("RM3100 detected Error! %x\r\n", data);
            return 1;
        } else {
            printf("RM3100 Address = %x\r\n", data);
            return 0; // 0 - 成功
        }
    }
}


/**
 * @brief 初始化 RM3100 设备
 *
 * 该函数通过 I2C 接口初始化指定地址的 RM3100 设备。
 *
 * @param address 设备的 I2C 地址
 * @param hi2c I2C 句柄
 * @return uint8_t 返回 0 表示成功，返回 1 表示失败
 */
uint8_t RM3100_Init(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t CCR[6] = {0, 200, 0, 200, 0, 200}; // 配置寄存器值

    // 读取设备 ID，如果失败则返回 1
    if (RM3100ReadID(address, hi2c)) {
        return 1;
    } else {
        // 写入配置寄存器值到设备
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c, address, ADDR_CCX, I2C_MEMADD_SIZE_8BIT, CCR, 6, 1000);
        uint8_t data = POLL_XYZ; // 轮询请求数据
        HAL_StatusTypeDef status1 = HAL_I2C_Mem_Write(&hi2c, address, ADDR_POLL, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);

        // 检查写入操作是否成功
        if (status != HAL_OK || status1 != HAL_OK) {
            printf("Error!,status=%d,status1=%d\r\n", status, status1);
            return 1;
        } else {
            return 0;
        }
    }
}


/**
 * @brief 检查 RM3100 设备的数据是否准备好
 *
 * 该函数通过 I2C 接口读取 RM3100 设备的状态寄存器，并检查数据是否准备好。
 *
 * @param address 设备的 I2C 地址
 * @param hi2c I2C 句柄
 * @return uint8_t 返回 0x80 表示数据准备好，返回 0 表示数据未准备好
 */
uint8_t RM3100_CheckDataReady(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t cResult;
    HAL_I2C_Mem_Read(&hi2c, address, ADDR_STATUS, I2C_MEMADD_SIZE_8BIT, &cResult, 1, 1000);
    cResult = cResult & 0x80;
    return cResult;
}


/**
 * @brief 获取 RM3100 设备的磁场数据
 *
 * 该函数通过 I2C 接口从指定地址的 RM3100 设备读取磁场数据。
 *
 * @param address 设备的 I2C 地址
 * @param x 指向存储 X 轴数据的指针
 * @param y 指向存储 Y 轴数据的指针
 * @param z 指向存储 Z 轴数据的指针
 * @param hi2c I2C 句柄
 * @return uint8_t 返回 0 表示成功，返回 1 表示失败
 */
uint8_t RM3100_GetData(uint8_t address, short *x, short *y, short *z, I2C_HandleTypeDef hi2c) {
    static long Mag_Data[3] = {0}; // 用于存储磁场数据
    uint8_t temp[9] = {0}; // 临时缓冲区，用于存储从设备读取的数据
    uint8_t poll_request = POLL_XYZ; // 轮询请求数据

    // 检查数据是否准备好
    if (RM3100_CheckDataReady(address, hi2c) == 0x80) {
        // 从设备读取磁场数据
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c, address, ADDR_MX, I2C_MEMADD_SIZE_8BIT, temp, 9, 1000);
        Mag_Data[0] = (long)temp[0] << 16 | (long)temp[1] << 8 | temp[2];

        if (Mag_Data[0] >= 0x00800000) { Mag_Data[0] |= 0xff000000; }
        Mag_Data[1] = (long)temp[3] << 16 | (long)temp[4] << 8 | temp[5];

        if (Mag_Data[1] >= 0x00800000) { Mag_Data[1] |= 0xff000000; }
        Mag_Data[2] = (long)temp[6] << 16 | (long)temp[7] << 8 | temp[8];

        if (Mag_Data[2] >= 0x00800000) { Mag_Data[2] |= 0xff000000; }

        // 将读取到的磁场数据存储到对应的变量中
        *y = - Mag_Data[0] * 13 / 1000;
        *x = - Mag_Data[1] * 13 / 1000;
        *z = - Mag_Data[2] * 13 / 1000;

        // 发送轮询请求数据到设备
        HAL_StatusTypeDef status1 = HAL_I2C_Mem_Write(&hi2c, address, ADDR_POLL, I2C_MEMADD_SIZE_8BIT, &poll_request, 1, 1000);
        if (status != HAL_OK || status1 != HAL_OK) {
            printf("Error!,status=%d,status1=%d\r\n", status, status1);
            return 1;
        } else {
            return 0;
        }
    } else {
        printf("Data Not Ready!\r\n");
        return 1;
    }
}


/**
 * @brief 打印 I2C 错误信息
 *
 * 该函数根据 I2C 句柄中的错误码打印相应的错误信息。
 *
 * @param hi2c 指向 I2C 句柄的指针
 */
void PrintI2CError(I2C_HandleTypeDef *hi2c)
{
    uint32_t error_code = hi2c->ErrorCode;

    // 检查是否有多个错误标志被设置
    if (error_code == HAL_I2C_ERROR_NONE)
    {
        printf("No error.\n");
    }
    else
    {
        printf("I2C Error: 0x%08X\n", error_code);

        if (error_code & HAL_I2C_ERROR_DMA)
            printf("  - DMA Error.\n");
        if (error_code & HAL_I2C_ERROR_OVR)
            printf("  - Overrun/Underrun (OVR).\n");
        if (error_code & HAL_I2C_ERROR_AF)
            printf("  - Addressing Failure.\n");
        if (error_code & HAL_I2C_ERROR_ARLO)
            printf("  - Bus Arbitration Lost.\n");
        if (error_code & HAL_I2C_ERROR_BERR)
            printf("  - Bus Error.\n");
        if (error_code & HAL_I2C_ERROR_NONE)
            printf("  - None Error.\n");
        if (error_code & HAL_I2C_ERROR_SIZE)
            printf("  - Size Error.\n");
        if (error_code & HAL_I2C_ERROR_TIMEOUT)
            printf("  - Timeout.\n");
        if (error_code & HAL_I2C_ERROR_DMA_PARAM)
            printf("  - DMA Parameter Error.\n");
    }
}

