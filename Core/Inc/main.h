/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define nToggle_SW_Pin GPIO_PIN_13
#define nToggle_SW_GPIO_Port GPIOC
#define Button_I_V_Pin GPIO_PIN_3
#define Button_I_V_GPIO_Port GPIOC
#define Button_I_V_EXTI_IRQn EXTI3_IRQn
#define ENC_TIM2_CH1_Pin GPIO_PIN_0
#define ENC_TIM2_CH1_GPIO_Port GPIOA
#define ENC_TIM2_CH2_Pin GPIO_PIN_1
#define ENC_TIM2_CH2_GPIO_Port GPIOA
#define ENC_Button_Pin GPIO_PIN_2
#define ENC_Button_GPIO_Port GPIOA
#define SPI1_Display_SCK_Pin GPIO_PIN_5
#define SPI1_Display_SCK_GPIO_Port GPIOA
#define SPI1_Display_MOSI_Pin GPIO_PIN_7
#define SPI1_Display_MOSI_GPIO_Port GPIOA
#define Display_RCLK_Current_Pin GPIO_PIN_4
#define Display_RCLK_Current_GPIO_Port GPIOC
#define Display_RCLK_Voltage_Pin GPIO_PIN_5
#define Display_RCLK_Voltage_GPIO_Port GPIOC
#define Display_RCLK_LED_Pin GPIO_PIN_0
#define Display_RCLK_LED_GPIO_Port GPIOB
#define ADS1256_DRDY_Pin GPIO_PIN_11
#define ADS1256_DRDY_GPIO_Port GPIOB
#define SPI2_ADS1256_CS_Pin GPIO_PIN_12
#define SPI2_ADS1256_CS_GPIO_Port GPIOB
#define SPI2_ADS1256_SCK_Pin GPIO_PIN_13
#define SPI2_ADS1256_SCK_GPIO_Port GPIOB
#define SPI2_ADS1256_MISO_Pin GPIO_PIN_14
#define SPI2_ADS1256_MISO_GPIO_Port GPIOB
#define SPI2_ADS1256_MOSI_Pin GPIO_PIN_15
#define SPI2_ADS1256_MOSI_GPIO_Port GPIOB
#define ADS1256_RST_Pin GPIO_PIN_6
#define ADS1256_RST_GPIO_Port GPIOC
#define ADS1256_SYNC_Pin GPIO_PIN_7
#define ADS1256_SYNC_GPIO_Port GPIOC
#define SPI3_LTC2602_SCK_Pin GPIO_PIN_10
#define SPI3_LTC2602_SCK_GPIO_Port GPIOC
#define SPI3_LTC2602_CS_Pin GPIO_PIN_11
#define SPI3_LTC2602_CS_GPIO_Port GPIOC
#define SPI3_LTC2602_MOSI_Pin GPIO_PIN_12
#define SPI3_LTC2602_MOSI_GPIO_Port GPIOC
#define ENC_Button_TEMP_Pin GPIO_PIN_4
#define ENC_Button_TEMP_GPIO_Port GPIOB
#define ENC_Button_TEMP_EXTI_IRQn EXTI4_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
