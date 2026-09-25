#include "main.h"
#include "stm32g4xx_hal.h"

#define AD7705_COMM_READ_TEST_REGISTER 0x48U

typedef struct {
    
	SPI_HandleTypeDef *spiHandle;

	GPIO_TypeDef *CSport;
	uint16_t CSpin;
	GPIO_TypeDef *DRDYport;
	uint16_t DRDYpin;

	uint16_t rawData;
	uint8_t status;
} AD7705;

void AD7705_Init(AD7705 *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *CSport, uint16_t CSpin, GPIO_TypeDef *DRDYport, uint16_t DRDYpin);

uint8_t AD7705_ReadTestRegister(AD7705 *dev);

