#include "main.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_def.h"

#ifndef INC_ADS1256_H_
#define INC_ADS1256_H_


/*
MACROS
*/

//Registers

#define ADS1256_REG_STATUS 0x00U
#define ADS1256_REG_MUX    0x01U
#define ADS1256_REG_ADCON  0x02U
#define ADS1256_REG_DRATE  0x03U
#define ADS1256_REG_IO     0x04U
#define ADS1256_REG_OFC0   0x05U
#define ADS1256_REG_OFC1   0x06U
#define ADS1256_REG_OFC2   0x07U
#define ADS1256_REG_FSC0   0x08U
#define ADS1256_REG_FSC1   0x09U
#define ADS1256_REG_FSC2   0x0AU

//Commands

#define ADS1256_CMD_WAKEUP   0x00U
#define ADS1256_CMD_RDATA    0x01U
#define ADS1256_CMD_RDATAC   0x03U
#define ADS1256_CMD_SDATAC   0x0FU
#define ADS1256_CMD_RREG     0x10U //only starts from first register
#define ADS1256_CMD_WREG     0x50U //only starts from first register (0x0001BBBB 0000DDDD, A = Command, B = Start Register, D = N of reg to read -1 )
#define ADS1256_CMD_SELFCAL  0xF0U
#define ADS1256_CMD_SELFOCAL 0xF1U
#define ADS1256_CMD_SELFGCAL 0xF2U
#define ADS1256_CMD_SYSOCAL  0xF3U
#define ADS1256_CMD_SYSGCAL  0xF4U
#define ADS1256_CMD_SYNC     0xFCU
#define ADS1256_CMD_STANDBY  0xFDU
#define ADS1256_CMD_RESET    0xFEU
#define ADS1256_CMD_NOP      0xFFU

//CS Selection

#define ADS1256_CS_EN HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_RESET);
#define ADS1256_CS_DIS HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_SET);

typedef struct
{
	SPI_HandleTypeDef *spiHandle;

	GPIO_TypeDef *CSPort;
	uint16_t CSPin;

	GPIO_TypeDef *DRDYPort;
	uint16_t DRDYPin;

	GPIO_TypeDef *RSTPort;
	uint16_t RSTPin;

	GPIO_TypeDef *SYNCPort;
	uint16_t SYNCPin;

	uint8_t registers[11]; //to contain a dump of all ADS registers

} ADS1256;

void ADS1256_Init(ADS1256 *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *CSPort, uint16_t CSPin, GPIO_TypeDef *DRDYPort, uint16_t DRDYPin, GPIO_TypeDef *RSTPort, uint16_t RSTPin,GPIO_TypeDef *SYNCPort, uint16_t SYNCPin);

uint8_t ADS1256_ReadStatusRegister(ADS1256 *dev);

void ADS1256_RegisterDump(ADS1256 *dev);

HAL_StatusTypeDef ADS1256_ReadRegisters(ADS1256 *dev, uint8_t startRegister, uint8_t *data, uint8_t count);


#endif /* INC_ADS1256_H_ */

