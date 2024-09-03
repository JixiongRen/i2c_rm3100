#ifndef __RM3100_H
#define __RM3100_H
#include "main.h"
#include "stm32f1xx_ll_i2c.h"


#define NUM_SENSORS 2

#define RM3100_I2C hi2c1

#define i2c1_RM3100_ADDRESS_1 0x20<<1//AD0 AD1都接地
#define i2c1_RM3100_ADDRESS_2 0x21<<1//AD0接高 AD1接地
#define i2c2_RM3100_ADDRESS_1 0x21<<1//AD0接地 AD1接高

#define RM3100_CONVERSION_INTERVAL	10000	// Microseconds, corresponds to 100 Hz (cycle count 200 on 3 axis)
#define UTESLA_TO_GAUSS			100.0f
#define RM3100_SENSITIVITY		75.0f

#define ADDR_POLL		0x00
#define ADDR_CMM		0x01
#define ADDR_CCX		0x04
#define ADDR_CCY		0x06
#define ADDR_CCZ		0x08
#define ADDR_TMRC		0x0B
#define ADDR_MX			0x24
#define ADDR_MY			0x27
#define ADDR_MZ			0x2A
#define ADDR_BIST		0x33
#define ADDR_STATUS		0x34
//#define ADDR_STATUS_READ		0xB4 //0X34|0X80
#define ADDR_HSHAKE		0x35
#define ADDR_REVID		0x36

#define CCX_DEFAULT_MSB		0x00
#define CCX_DEFAULT_LSB		0xC8
#define CCY_DEFAULT_MSB		CCX_DEFAULT_MSB
#define CCY_DEFAULT_LSB		CCX_DEFAULT_LSB
#define CCZ_DEFAULT_MSB		CCX_DEFAULT_MSB
#define CCZ_DEFAULT_LSB		CCX_DEFAULT_LSB
#define CMM_DEFAULT		0x70	// No continuous mode
#define CONTINUOUS_MODE		(1 << 0)
#define POLLING_MODE		(0 << 0)
#define TMRC_DEFAULT		0x94
#define BIST_SELFTEST		0x8F
#define BIST_DEFAULT		0x00
#define BIST_XYZ_OK		((1 << 4) | (1 << 5) | (1 << 6))
#define STATUS_DRDY		(1 << 7)
#define POLL_XYZ		0x70
#define RM3100_REVID		0x22

#define NUM_BUS_OPTIONS		(sizeof(bus_options)/sizeof(bus_options[0]))

// add
typedef struct {
    short x;
    short y;
    short z;
} MagneticData;

uint8_t RM3100ReadID(uint8_t address, I2C_HandleTypeDef hi2c);
uint8_t RM3100_Init(uint8_t address, I2C_HandleTypeDef hi2c);
uint8_t RM3100_GetData(uint8_t address, short *x, short *y, short *z, I2C_HandleTypeDef hi2c);
uint8_t RM3100_CheckDataReady(uint8_t address, I2C_HandleTypeDef hi2c);
void PrintI2CError(I2C_HandleTypeDef *hi2c);
#endif

