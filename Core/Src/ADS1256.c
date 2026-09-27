#include "ADS1256.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_spi.h"


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

uint8_t ADS1256_ReadStatusRegister(ADS1256 *dev)
{
	uint8_t command[] = {ADS1256_CMD_RREG, 0x00}; //Read the first register
	uint8_t status;

	HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(dev->spiHandle, command, sizeof(command), HAL_MAX_DELAY);
	HAL_SPI_Receive(dev->spiHandle, &status, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_SET);

	return status;
}

void ADS1256_RegisterDump(ADS1256 *dev){
	
	uint8_t command[] = {ADS1256_CMD_RREG, 0x0F}; // read from first register, 0f - 1 
	uint8_t registers[11];

	HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(dev->spiHandle, command, sizeof(command), HAL_MAX_DELAY);
	HAL_SPI_Receive(dev->spiHandle, registers, sizeof(registers), HAL_MAX_DELAY);
	HAL_GPIO_WritePin(dev->CSPort, dev->CSPin, GPIO_PIN_SET);

	for (uint8_t i = 0; i < sizeof(registers); i++) {
		dev->registers[i] = registers[i];
	}
}

HAL_StatusTypeDef ADS1256_ReadRegisters(ADS1256 *dev, uint8_t startRegister, uint8_t *data, uint8_t count){

	//ref datasheet page 36 for RREG command usage
	//check if any inputs are invalid
	if(dev == NULL || startRegister > ADS1256_REG_FSC2 || data == NULL || count > (ADS1256_REG_FSC2 - startRegister + 1)){
		return HAL_ERROR;
	}

	uint8_t command[] = {ADS1256_CMD_RREG | startRegister, count - 1};

	ADS1256_CS_EN;

	HAL_StatusTypeDef status = HAL_SPI_Transmit(dev->spiHandle, command, sizeof(command), HAL_MAX_DELAY);

	if (status == HAL_OK) {
		status = HAL_SPI_Receive(dev->spiHandle, data, count, HAL_MAX_DELAY);
	}

	ADS1256_CS_DIS;

	return status;
}