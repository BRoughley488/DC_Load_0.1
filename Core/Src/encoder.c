#include "encoder.h"

void encoderInit(encoder *enc, TIM_HandleTypeDef *htim, GPIO_TypeDef *buttonPort, uint16_t buttonPin, UART_HandleTypeDef *huart){
    enc->timerHandle = htim;
    enc->uartHandle = huart;
    enc->buttonPort = buttonPort;
    enc->buttonPin = buttonPin;

}



void encoderTimerElapsed(encoder *enc){
    uint8_t direction = __HAL_TIM_IS_TIM_COUNTING_DOWN(enc->timerHandle); // check direction of encoder

    if(direction == 1){
      enc->encoder_down = 1;
      enc->encoder_up = 0;
    }
    else{
      enc->encoder_down = 0;
      enc->encoder_up = 1;
    }

}