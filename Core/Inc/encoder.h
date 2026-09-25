#include "main.h"
#include "stm32g4xx_hal.h"


typedef struct 
{
    uint16_t *currentAdjust; //for the commanded current
    uint16_t *voltageAdjust;//for commanded voltage

    TIM_HandleTypeDef *timerHandle; 
    UART_HandleTypeDef *uartHandle; //uart for debugging

    GPIO_TypeDef *buttonPort; //port and pin for the encoder
    uint16_t buttonPin;

    uint32_t *valueBuffer;

    uint8_t encoder_up; //(CW)
    uint8_t encoder_down; //(CCW)

}encoder;

void encoderInit(encoder *enc, TIM_HandleTypeDef *htim, GPIO_TypeDef *buttonPort, uint16_t buttonPin, UART_HandleTypeDef *huart);

void encoderTimerElapsed(encoder *enc);

void encoderRead(encoder *enc, uint16_t *val);
