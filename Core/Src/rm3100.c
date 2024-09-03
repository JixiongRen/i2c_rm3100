#include "rm3100.h"
#include "stm32f1xx.h"
#include "stdio.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

// 根据实际情况添加更多地址

uint8_t RM3100ReadID(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t data = 0;
    uint8_t addr_hshake = ADDR_HSHAKE;

    HAL_StatusTypeDef status1 = HAL_I2C_Master_Transmit(&hi2c, address, &addr_hshake, 1, 1000);
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c, (address | 1), &data, 1, 1000);

    if (status != HAL_OK) {
        PrintI2CError(&hi2c);
        return 1;
    } else {
        if (data != 0x1B) {
            printf("RM3100 detected Error! %x\r\n", data);
            return 1;
        } else {
            printf("RM3100 Address = %x\r\n", data);
            return 0;
        }
    }
}

uint8_t RM3100_Init(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t CCR[6] = {0, 200, 0, 200, 0, 200};

    if (RM3100ReadID(address, hi2c)) {
        return 1;
    } else {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c, address, ADDR_CCX, I2C_MEMADD_SIZE_8BIT, CCR, 6, 1000);
        uint8_t data = POLL_XYZ;
        HAL_StatusTypeDef status1 = HAL_I2C_Mem_Write(&hi2c, address, ADDR_POLL, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);

        if (status != HAL_OK || status1 != HAL_OK) {
            printf("Error!,status=%d,status1=%d\r\n", status, status1);
            return 1;
        } else {
            return 0;
        }
    }
}

uint8_t RM3100_CheckDataReady(uint8_t address, I2C_HandleTypeDef hi2c) {
    uint8_t cResult;
    HAL_I2C_Mem_Read(&hi2c, address, ADDR_STATUS, I2C_MEMADD_SIZE_8BIT, &cResult, 1, 1000);
    cResult = cResult & 0x80;
    return cResult;
}

uint8_t RM3100_GetData(uint8_t address, short *x, short *y, short *z, I2C_HandleTypeDef hi2c) {
    static long Mag_Data[3] = {0};
    uint8_t temp[9] = {0};
    uint8_t poll_request = POLL_XYZ;

    if (RM3100_CheckDataReady(address, hi2c) == 0x80) {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c, address, ADDR_MX, I2C_MEMADD_SIZE_8BIT, temp, 9, 1000);
        Mag_Data[0] = (long)temp[0] << 16 | (long)temp[1] << 8 | temp[2];

        if (Mag_Data[0] >= 0x00800000) { Mag_Data[0] |= 0xff000000; }
        Mag_Data[1] = (long)temp[3] << 16 | (long)temp[4] << 8 | temp[5];

        if (Mag_Data[1] >= 0x00800000) { Mag_Data[1] |= 0xff000000; }
        Mag_Data[2] = (long)temp[6] << 16 | (long)temp[7] << 8 | temp[8];

        if (Mag_Data[2] >= 0x00800000) { Mag_Data[2] |= 0xff000000; }

        *y = -Mag_Data[0];
        *x = -Mag_Data[1];
        *z = -Mag_Data[2];

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
			printf("  - (DMA Parameter Error.\n");
	}
}

