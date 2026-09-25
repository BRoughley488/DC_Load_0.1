/*
 * 7segmentDisplay_4D.h
 *
 *  Created on: Feb 17, 2026
 *      Author: broug
 *
 *      README
 *      SPI must be LSB first
 *      Daisy chain number 74HC595 -> digit 74HC595
 *
 */

#ifndef INC_7SEGMENTDISPLAY_4D_H_
#define INC_7SEGMENTDISPLAY_4D_H_

#include "stm32g4xx_hal.h"
#include<math.h>
#include "main.h" // moved this from c to h


/*
* USER OPTIONS
*/

#define digitUpdateFrequency 250 //frequency in Hz to update each digit, 250Hz is a good value for flicker free display
#define flashRate 250 //arbitrary for now, used SSD_Write_number
/*
* MACROS
*/

//numbers (active HIGH - common cathode)

#define SSD_ZERO 0b11111100
#define SSD_ONE 0b01100000
#define SSD_TWO 0b11011010
#define SSD_THREE 0b11110010
#define SSD_FOUR 0b01100110
#define SSD_FIVE 0b10110110
#define SSD_SIX 0b10111110
#define SSD_SEVEN 0b11100000
#define SSD_EIGHT 0b11111110
#define SSD_NINE 0b11110110

//digit selection (active LOW - common cathode)

#define SSD_D1 0b01111111
#define SSD_D2 0b10111111
#define SSD_D3 0b11011111
#define SSD_D4 0b11101111

#define SSD_ALL 0b00000000 

//if using common anode, invert selection

//decimal point
#define SSD_NO_DECIMAL 0xff

//segments array

//uint8_t segmentArray[] = {SSD_ZERO, SSD_ONE, SSD_TWO, SSD_THREE, SSD_FOUR, SSD_FIVE, SSD_SIX, SSD_SEVEN, SSD_EIGHT, SSD_NINE}; //this does not work here ?!?!?!?!


typedef struct {

	SPI_HandleTypeDef *spiHandle;
	GPIO_TypeDef *RCLKport;
	uint16_t RCLKpin;

	uint16_t *num;
	float *floatNum;

	uint8_t decPoints;

	uint8_t digCounter;

	uint8_t flashingD1; //flag for flashing each digit, 1 = flash, 0 = no flash :)
	uint8_t flashingD2;
	uint8_t flashingD3;
	uint8_t flashingD4;

	uint16_t flashCounter;
	uint8_t flashFlag; //to tell the write number func to flash
}sevenSegmentDisplay;

typedef struct {

}screen;

void SSD_Init(sevenSegmentDisplay *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *Rport, uint16_t Rpin, uint16_t *dat, float *floatDat, uint8_t dp); //dat is a pointer to the data that is to be displayed, dp is decimal points (3 max)

void SSD_Write_number(sevenSegmentDisplay *dev, uint8_t num, uint8_t dig, uint8_t dp);

void numToSegment(uint16_t num,  uint8_t *o1,  uint8_t *o2,  uint8_t *o3,  uint8_t *o4);

void largeNumToSegment(uint32_t num,  uint8_t *o1,  uint8_t *o2,  uint8_t *o3,  uint8_t *o4);

void updateScreen(sevenSegmentDisplay *dev, uint8_t dig);

void floatUpdateScreen(sevenSegmentDisplay *dev, volatile uint8_t *flag);

void testDisplay(sevenSegmentDisplay *dev);

void flashDigit(sevenSegmentDisplay *dev, uint8_t dig, uint8_t flash); //revert to digit selection macros for dig, 0 = no flash, 1 = flash

#endif /* INC_7SEGMENTDISPLAY_4D_H_ */
