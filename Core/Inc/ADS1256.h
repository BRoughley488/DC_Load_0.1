#include "main.h"
#include "stm32g4xx_hal.h"

#ifndef INC_ADS1256_H_
#define INC_ADS1256_H_

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

} ADS1256;

void ADS1256_Init(ADS1256 *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *CSPort, uint16_t CSPin, GPIO_TypeDef *DRDYPort, uint16_t DRDYPin, GPIO_TypeDef *RSTPort, uint16_t RSTPin,GPIO_TypeDef *SYNCPort, uint16_t SYNCPin);

#endif /* INC_ADS1256_H_ */

