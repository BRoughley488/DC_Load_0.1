#include "AD7705.h"

void AD7705_Init(AD7705 *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *CSport, uint16_t CSpin, GPIO_TypeDef *DRDYport, uint16_t DRDYpin)
{
	dev->spiHandle = handle;
	dev->CSport = CSport;
	dev->CSpin = CSpin;
	dev->DRDYport = DRDYport;
	dev->DRDYpin = DRDYpin;
	dev->rawData = 0;
	dev->status = 0;
}

uint8_t AD7705_ReadTestRegister(AD7705 *dev)
{
	uint8_t testRegister = 0;
	uint8_t command = AD7705_COMM_READ_TEST_REGISTER;

	HAL_GPIO_WritePin(dev->CSport, dev->CSpin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(dev->spiHandle, &command, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(dev->spiHandle, &testRegister, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(dev->CSport, dev->CSpin, GPIO_PIN_SET);

	return testRegister;
}

