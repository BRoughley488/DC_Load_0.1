#include "stm32g4xx_hal.h"
#include "main.h"

/*
Bitmask macros for toggling LED's, invert to turn them off when bitmasking
Use reg | (or) to turn on, reg & (and) with inverse of bitmask to turn off
*/

#define LED_1 0b10000000
#define LED_2 0b01000000
#define LED_3 0b00100000
#define LED_4 0b00010000
#define LED_5 0b00001000
#define LED_6 0b00000100
#define LED_7 0b00000010
#define LED_8 0b00000001

#define IND_LED_ERROR 0b10000000
#define IND_LED_OVP 0b01000000
#define IND_LED_OCP 0b00100000
#define IND_LED_CC 0b00010000
#define IND_LED_CV 0b00001000
#define IND_LED_CR 0b00000100
#define IND_LED_SPARE 0b00000010

typedef struct {

    SPI_HandleTypeDef *spiHandle;
	GPIO_TypeDef *RCLKport;
	uint16_t RCLKpin;

    uint8_t ledBuffer; //each byte represents an LED / shift register output, 1 is on, 0 is off

    uint8_t *updateFlag; //flag to tell functions to send the contents of the buffer to the shift register

    uint8_t prevBuffer; // to store the previous state of the LEDs, so that only the changed LEDs need to be updated when writing to the shift register

} IndicatorLED;

void indicatorLEDinit(IndicatorLED *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *Rport, uint16_t Rpin, volatile uint8_t *flag);

void indicatorLEDtest(IndicatorLED *dev); //function to test the LEDs, turns them all on and off

void indicatorLEDon(IndicatorLED *dev, uint8_t ledNum); //led num is the number of the LED to be turned on, starting from 0
void indicatorLEDoff(IndicatorLED *dev, uint8_t ledNum); //led num is the number of the LED to be turned off, starting from 0
void indicatorLEDwrite(IndicatorLED *dev); //writes the current state of the LED buffer to the shift register, to update the LEDs