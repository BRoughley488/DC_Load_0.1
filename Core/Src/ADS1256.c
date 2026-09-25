#include "ADS1256.h"

void ADS1256_Init(ADS1256 *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *CSPort, uint16_t CSPin,GPIO_TypeDef *DRDYPort, uint16_t DRDYPin, GPIO_TypeDef *RSTPort, uint16_t RSTPin, GPIO_TypeDef *SYNCPort, uint16_t SYNCPin)
{
	dev->spiHandle = handle;

	dev->CSPort = CSPort;
	dev->CSPin = CSPin;

	dev->DRDYPort = DRDYPort;
	dev->DRDYPin = DRDYPin;

	dev->RSTPort = RSTPort;
	dev->RSTPin = RSTPin;

	dev->SYNCPort = SYNCPort;
	dev->SYNCPin = SYNCPin;
}