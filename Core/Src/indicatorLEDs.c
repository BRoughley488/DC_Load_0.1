#include "indicatorLEDs.h"

void indicatorLEDinit(IndicatorLED *dev, SPI_HandleTypeDef *handle, GPIO_TypeDef *Rport, uint16_t Rpin, volatile uint8_t *flag){

    dev->spiHandle = handle;
    dev->RCLKport = Rport;
    dev->RCLKpin = Rpin;
    dev->ledBuffer = 0; //initialize buffer to all LEDs off
    dev->updateFlag = flag; //pass in pointer to flag for updating the LEDs, so that the flag can be set within the functions in this file when an update is needed
    
}

void indicatorLEDtest(IndicatorLED *dev){
    dev->ledBuffer = 0xFF;

    HAL_SPI_Transmit(dev->spiHandle, &dev->ledBuffer, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

    HAL_Delay(500);

    dev->ledBuffer = 0x00;

    HAL_SPI_Transmit(dev->spiHandle, &dev->ledBuffer, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

    HAL_Delay(500);

    dev->ledBuffer = 0b00000001;

    for (uint8_t i=0; i<8; i++){
        HAL_SPI_Transmit(dev->spiHandle, &dev->ledBuffer, 1, HAL_MAX_DELAY);

        HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
        HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

        HAL_Delay(100);

        dev->ledBuffer <<= 1; //shift the bit to the left to move to the next LED

    }

    dev->ledBuffer = 0b10000000; // start from the last LED for the reverse pass

    for (uint8_t i=0; i<8; i++){
        HAL_SPI_Transmit(dev->spiHandle, &dev->ledBuffer, 1, HAL_MAX_DELAY);

        HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
        HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

        HAL_Delay(100);

        dev->ledBuffer >>= 1; //shift the bit to the right to move to the previous LED

    }

    dev->ledBuffer = 0x00; // turn all LEDs off at the end of the test

}

void indicatorLEDon(IndicatorLED *dev, uint8_t ledNum){

    dev->ledBuffer |= ledNum; //set the bit corresponding to the LED number, 7-ledNum because the first LED is the MSB

    *dev->updateFlag = 1;
}


void indicatorLEDoff(IndicatorLED *dev, uint8_t ledNum){

    dev->ledBuffer &= ~ledNum; //clear the bit corresponding to the LED number, 7-ledNum because the first LED is the MSB

    *dev->updateFlag = 1;
}

void indicatorLEDwrite(IndicatorLED *dev){

    HAL_SPI_Transmit(dev->spiHandle, &dev->ledBuffer, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 1);
    HAL_GPIO_WritePin(dev->RCLKport, dev->RCLKpin, 0);

}