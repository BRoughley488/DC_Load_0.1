/*
 * 7segmentDisplay_4D.c
 *
 *  Created on: Feb 17, 2026
 *      Author: broug
 *
 *  All code is origional to me, AI has been used mainly for comments and the odd bit of repetitive code
 */


#include "7segmentDisplay_4D.h"

uint8_t segmentArray[] = {SSD_ZERO, SSD_ONE, SSD_TWO, SSD_THREE, SSD_FOUR, SSD_FIVE, SSD_SIX, SSD_SEVEN, SSD_EIGHT, SSD_NINE};

uint8_t counter = 0;


void SSD_Init(sevenSegmentDisplay *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *Rport, uint16_t Rpin, uint16_t *dat, float *floatDat, uint8_t dp){
	dev->spiHandle = handle; //handle for SPI peripheral, used to transmit data to the shift registers
	dev->RCLKport = Rport; //port for the RCLK pin, used to latch the data into the shift registers
	dev->RCLKpin = Rpin; //pin for the RCLK pin, used to latch the data into the shift registers
	dev->num = dat; //pointer to the integer value to be displayed, used to convert the number into its individual digits for display
	dev->floatNum = floatDat; // pointer to the float value to be displayed, used to convert the number into its individual digits for display
	dev->decPoints = dp; //number of decimal points to be displayed, used to determine where the decimal point should be placed on the display

	dev->digCounter = 0; //counter to keep track of which digit is being displayed, used to update the display one digit at a time
}

/*
 * Function to write a single digit to the 7 segment display, with optional decimal point
 * dev: pointer to the sevenSegmentDisplay struct containing the SPI handle and GPIO information
 * num: the number to be displayed (0-9)
 * dig: the digit position (0-3) where 0 is the leftmost digit and 3 is the rightmost digit
 * dp: flag to indicate if a decimal point should be displayed (1 for yes, 0 for no)
*/

void SSD_Write_number(sevenSegmentDisplay *dev, uint8_t num, uint8_t dig, uint8_t dp){

	HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

	uint8_t digit; //buffer for digit selection, to be transmitted to the digit shift register

	switch (dig){//decides which digit I want to display, and then inserts the macro for the digit selection shift register into the digit buffer
	case 0:
		digit = SSD_D1;
		break;
	case 1:
		digit = SSD_D2;
		break;
	case 2:
		digit = SSD_D3;
		break;
	case 3:
		digit = SSD_D4;
		break;
	}

	if (dev->flashCounter == flashRate){ //when counter reaches flash, reset, and toggle the flag to enable OR disable the digit at the desired freq
		dev->flashCounter = 0; //reset counter
		dev->flashFlag ^= 1; //set flag to tell program to flash the digit (XOR to toggle)
	}
	else{
		dev->flashCounter++; //increment counter
	}

	//uint8_t pData[2] = {segmentArray[num], digit};// buffer fpr the data to be transmitted. [0] is the number to be displayed, [1] is for the digit selection
	uint8_t pData[2] = {digit, segmentArray[num]};// buffer fpr the data to be transmitted. [0] is the number to be displayed, [1] is for the digit selection

	if (dp == 1){ //if decimal place is desired at the digit, enable Q7 of the shift register (connected to decimal point of the display)
		pData[1] |= 0b00000001;
	}
	else if(dp == SSD_NO_DECIMAL){ //if no decimal is required ensure that bit is turned off
		pData[1] &= ~0x01;// ~00000001 = 11111110 - make the last bit 0 to disable decimal point
	}


	if(dig == 0 && dev->flashingD1 == 1 && dev->flashFlag == 1){ //if the first digit is being displayed and the flashing flag is set, turn off the digit
		pData[0] = 0b11111111; //turn ALL digits off
	}
	else if(dig == 1 && dev->flashingD2 == 1 && dev->flashFlag == 1){ 
		pData[0] = 0b11111111; 
	}
	else if(dig == 2 && dev->flashingD3 == 1 && dev->flashFlag == 1){ 
		pData[0] = 0b11111111; 
	}
	else if(dig == 3 && dev->flashingD4 == 1 && dev->flashFlag == 1){ 
		pData[0] = 0b11111111; 
	}


	HAL_SPI_Transmit(dev->spiHandle, pData, 2, HAL_MAX_DELAY);

	HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
	HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

}

/*
* Function to convert a number into its individual digits for display on a 4-digit 7-segment display
* num: the number to be converted (0-9999)
* o1: pointer to the variable that will hold the most significant digit
* o4: pointer to the variable that will hold the least significant digit
*/

void numToSegment(uint16_t num,  uint8_t *o1,  uint8_t *o2,  uint8_t *o3,  uint8_t *o4){
	*o1 = (int)floor((num/1000) % 10);
	*o2 = (int)floor((num/100) % 10);
	*o3 = (int)floor((num/10) % 10);
	*o4 = (int)floor(num % 10);

}

void largeNumToSegment(uint32_t num,  uint8_t *o1,  uint8_t *o2,  uint8_t *o3,  uint8_t *o4){

	num /= 10; //drop the last digit as it is not needed 

	*o1 = (int)floor((num/1000) % 10); //gets thousands digit, applies modulo 10 to extract the digit, and then casts to an int to remove any decimal places
	*o2 = (int)floor((num/100) % 10); //gets hundreds digit, applies modulo 10 to extract the digit, and then casts to an int to remove any decimal places
	*o3 = (int)floor((num/10) % 10); //gets tens digit, applies modulo 10 to extract the digit, and then casts to an int to remove any decimal places
	*o4 = (int)floor(num % 10); //just extracts the lsd using modulo 10

}

void updateScreen(sevenSegmentDisplay *dev, uint8_t dig){

	uint8_t buff[4];

	numToSegment(*(dev->num), &buff[0], &buff[1], &buff[2], &buff[3]);
	//numToSegment(1489, &skibidiNumber[0], &skibidiNumber[1], &skibidiNumber[2], &skibidiNumber[3]);

	SSD_Write_number(dev, buff[dig], dig, SSD_NO_DECIMAL);

//	for (int i = 0; i < 4; i++){
//		SSD_Write_number(dev, buff[i], i);
//
//	}

}



void floatUpdateScreen(sevenSegmentDisplay *dev, volatile uint8_t *flag){ //takes approx 13.84 microseconds according to scope

	float num = *(dev->floatNum);

	//probably not needed (below)

	// if(num == 0){
	// 	*flag = 1; 
	// 	return;
	// }

	uint8_t buff[4]; //buffer for the 4 digits to be displayed, passed into the number to segment conversion function

	uint32_t intValue = (uint32_t)(num * 1000.0f + 0.5f);// preserve 3 decimal places of the float ( max the display can do anyway), and then add .5 for rounding as the dp gets chopped off when trunacated.

	if(intValue >= 1000000){ //100000 is equivelant to 100.000 before conversion on previous line
		Error_Handler(); // I may or may not sort out error handling :)
	}

	if(intValue < 10000){

		numToSegment(intValue, &buff[0], &buff[1], &buff[2], &buff[3]);

	}
	else if(intValue <100000){

		largeNumToSegment(intValue, &buff[0], &buff[1], &buff[2], &buff[3]);

	}
	else {
		numToSegment((uint16_t)(intValue/100), &buff[0], &buff[1], &buff[2], &buff[3]);
	}



	uint8_t dec = SSD_NO_DECIMAL; //default to no decimal place

	if(intValue < 10000 && dev->digCounter == 0){//unless the value is less than 10.000 and you are at the correct location for the decimal place (first digit)
		dec = 1; //set decimal place to current digit (first)
	}
	else if(((intValue < 100000) && (intValue > 10000)) && dev->digCounter == 1){//unless the value is in between 10.000 and 100.000
		dec = 1;//set decimal place to current digit (second)
	}
	else if (intValue >= 100000 && dev->digCounter == 2){ //unless the value is in between 100.000 and 999.999
		dec = 1; //set decimal place to current digit (third)
	}


	SSD_Write_number(dev, buff[dev->digCounter], dev->digCounter, dec); //buff[dev>digCounter] uses the digit couner to select the array location to transmit the single digit

	dev->digCounter ++; //increment counter to keep track of digit

	if(dev->digCounter == 4){ //if last digit has been reached

			*flag = 1; //flag for telling program the entire digit has been written

			dev->digCounter = 0; // reset counter
		}


}

void testDisplay(sevenSegmentDisplay *dev){

	uint8_t pData[2] = {0b00000000, 0b11111111};// buffer fpr the data to be transmitted. [0] is the number to be displayed, [1] is for the digit selection

	HAL_SPI_Transmit(dev->spiHandle, pData, 2, HAL_MAX_DELAY);

	HAL_Delay(1);

	HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);

	HAL_Delay(1);

	HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

}

void flashDigit(sevenSegmentDisplay *dev, uint8_t dig, uint8_t flash){
	
	switch (dig){//decides which digit I want to flash, and then sets the corresponding flag in the struct to 1 (flash) or 0 (no flash)
	case SSD_D1:
		dev->flashingD1 = flash;
		break;
	case SSD_D2:
		dev->flashingD2 = flash;
		break;
	case SSD_D3:
		dev->flashingD3 = flash;
		break;
	case SSD_D4:
		dev->flashingD4 = flash;
		break;
	case SSD_ALL:
		dev->flashingD1 = flash;
		dev->flashingD2 = flash;
		dev->flashingD3 = flash;
		dev->flashingD4 = flash;
		break;
	default:
		break;
	}

}

