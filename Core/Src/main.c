/* USER CODE BEGIN Header */

/*
Timers:
  TIM17: Triggers the interrupt to refresh the display (On SPI1)
*/

/*
Development Notes:
  The button for the encoder is temporary
  Button known as "Button_I/V" in documentation will be used for edit enable / disable. A seperate button for switching between CC/CV will be added
*/

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ADS1256.h"
#include "encoder.h"
#include "indicatorLEDs.h"
#include "7segmentDisplay_4D.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_tim.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM16_Init(void);
static void MX_TIM17_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C3_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

void flagHandler(void);
void reportStates(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/*
MACROS
*/

#define SSD_CURRENT_DISPLAY 0x00
#define SSD_VOLTAGE_DISPLAY 0x01


sevenSegmentDisplay SSDcurrent;
sevenSegmentDisplay SSDvoltage;
IndicatorLED indicator_leds;
encoder rotaryEncoder;

/*
Flags
*/

volatile uint8_t SSD_Current_numberWrittenFlag; //to verify when the entire number has been written to the 7 seg display, passed into function and updated when the entire display has been written
volatile uint8_t SSD_Current_screenUpdateFlag; //to verify when the screen needs to be updated with a new number

volatile uint8_t indicator_led_updateFlag;

volatile uint8_t encoder_flag; //for when encoder is rotated, triggered by TIM2 ARR interrupt, for encoder lib
volatile uint8_t encoder_button_flag; //for when encoder button is pressed, triggered by EXTI interrupt

volatile uint8_t encoder_rotated; //flag for software to check encoder has been rotated

volatile uint8_t mode_button_pressed_flag; // seen as button i/v on schematic

//THE BUTON I/V is temporarily being used for the edit mode button for testing. :(

uint8_t editMode; // 0x00 - No edit | 0x01 - Edit whicever mode is selected (CC or CV), press to engage, press to cycle digits
#define EDIT_MODE_EN 0x01
#define EDIT_MODE_DIS 0x00

uint8_t loadMode; //set the mode of the load (cc or cv), defaults to 0x00 (CC)
#define MODE_CV 0xFF
#define MODE_CC 0x00

/*
Variables
*/

//system information variables

uint8_t maxCurrent = 10; //max current in amps, protections trigger once this is exceded
uint8_t maxVoltage = 60; //max voltage in volts, protections trigger once this is exceded
uint8_t softwareVersion = 1; //software version

//global variables 

uint16_t SSD_Current_value_int = 1234; //may remove the requirement for int value
uint16_t SSD_Voltage_value_int = 1234;

uint8_t displayTracker = 0; //to track which display is being updated, to allow alternating the multiplexing between current and voltage
uint8_t editModeTimeoutCNT; //counter for the timeout of edit mode

float currentReading; 
float voltageReading;

//int values not needed, just there to satisfy init fucntion
uint16_t commandedCurrent; //user set value for the current, when in CC mode
uint16_t commandedVoltage; //user set value for the voltage, when in CV mode

float floatCommandedCurrent; //user set value for the current, when in CC mode
float floatCommandedVoltage; //user set value for the voltage, when in CV mode

uint8_t statusRegister; //to store the status of the device

uint8_t unitOfAdjustment; //to store the unit of adjustment for the encoder, e.g. 1mA, 10mA, 100mA, 1A e.c.t.


/*
MACROS
*/

#define UNIT_ADJUSTMENT_1mA 0x00 //defaults to 1mA then you can't accidentally adjust by loads
#define UNIT_ADJUSTMENT_10mA 0x01
#define UNIT_ADJUSTMENT_100mA 0x02
#define UNIT_ADJUSTMENT_1A 0x03

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  
  SSD_Init(&SSDcurrent, &hspi1, GPIOC, GPIO_PIN_4, &SSD_Current_value_int, &floatCommandedCurrent, 3);
  SSD_Init(&SSDvoltage, &hspi1, GPIOC, GPIO_PIN_5, &SSD_Voltage_value_int, &floatCommandedVoltage, 3);
  encoderInit(&rotaryEncoder, &htim2, ENC_Button_TEMP_GPIO_Port, ENC_Button_TEMP_Pin, &huart1);
  indicatorLEDinit(&indicator_leds, &hspi1, GPIOB, GPIO_PIN_0, &indicator_led_updateFlag);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_ADC2_Init();
  MX_I2C3_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  char uartMSG[] = "DC Load Starting\r\n";
  HAL_UART_Transmit(&huart1, (uint8_t*)uartMSG, sizeof(uartMSG), HAL_MAX_DELAY);

  HAL_TIM_Base_Start_IT(&htim17); //start display refresh timer, interrupt enabled
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); //start encoder timer
  //HAL_TIM_Base_Start_IT(&htim7); //timer for auto-exiting edit mode (CLK = 170M, PSC = 17000-1 = CLK 1Khz)
 __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE); //enable interrupt on ARR overflow for encoder
 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  currentReading = 16.87; // TEST VALUE DELETE THIS LATER -------
  voltageReading = 5.23; // TEST VALUE DELETE THIS LATER -------

  indicatorLEDtest(&indicator_leds);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    
    flagHandler();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10E1A6F2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10E1A6F2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_1LINE;
  hspi3.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.EncoderMode = TIM_ENCODERMODE_TI2;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 15;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 15;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 17000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 10000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief TIM16 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM16_Init(void)
{

  /* USER CODE BEGIN TIM16_Init 0 */

  /* USER CODE END TIM16_Init 0 */

  /* USER CODE BEGIN TIM16_Init 1 */

  /* USER CODE END TIM16_Init 1 */
  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 1700-1;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 30000;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM16_Init 2 */

  /* USER CODE END TIM16_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 170-1;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 1000;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Display_RCLK_Current_Pin|Display_RCLK_Voltage_Pin|ADS1256_RST_Pin|ADS1256_SYNC_Pin
                          |SPI3_LTC2602_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Display_RCLK_LED_Pin|SPI2_ADS1256_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : nToggle_SW_Pin */
  GPIO_InitStruct.Pin = nToggle_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(nToggle_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Button_I_V_Pin */
  GPIO_InitStruct.Pin = Button_I_V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Button_I_V_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_Button_Pin */
  GPIO_InitStruct.Pin = ENC_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENC_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Display_RCLK_Current_Pin Display_RCLK_Voltage_Pin ADS1256_RST_Pin ADS1256_SYNC_Pin
                           SPI3_LTC2602_CS_Pin */
  GPIO_InitStruct.Pin = Display_RCLK_Current_Pin|Display_RCLK_Voltage_Pin|ADS1256_RST_Pin|ADS1256_SYNC_Pin
                          |SPI3_LTC2602_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Display_RCLK_LED_Pin SPI2_ADS1256_CS_Pin */
  GPIO_InitStruct.Pin = Display_RCLK_LED_Pin|SPI2_ADS1256_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ADS1256_DRDY_Pin ENC_Button_TEMP_Pin */
  GPIO_InitStruct.Pin = ADS1256_DRDY_Pin|ENC_Button_TEMP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM17){ // check if the interrupt is from TIM17, which is for the display refresh
    SSD_Current_screenUpdateFlag = 1; // current flag starts off the process, will switch to voltage automatically after the current display has been updated
  }

  if(htim->Instance == TIM2){ //check if the interrupt is from TIM2, which is for the encoder
    encoder_flag = 1; //set flag to indicate encoder has been rotated
  }

  if(htim->Instance == TIM7){ //if interrupt is from TIM7, used for auto exit of edit mode

    uint8_t timeout = 3; // how many seconds edit mode should last
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8); //toggle pin for testing

    if (editModeTimeoutCNT < timeout){ //if delay timer hasnt been reached
      editModeTimeoutCNT ++; // increment counter
    }
    else if (editModeTimeoutCNT >= timeout){
      editModeTimeoutCNT = 0; // reset counter
      editMode = EDIT_MODE_DIS; // disable edit mode
      HAL_TIM_Base_Stop_IT(&htim7);
      flashDigit(&SSDcurrent, SSD_ALL, 0);
      flashDigit(&SSDvoltage, SSD_ALL, 0);
    }

  }
  
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin == ENC_Button_TEMP_Pin){ //check if the interrupt is from the encoder button
    encoder_button_flag = 1; //set flag to indicate encoder button has been pressed
  }

  if(GPIO_Pin == Button_I_V_Pin){ //if the edit button has been pressed
    mode_button_pressed_flag = 1; //set flag to 1
  }

}


void flagHandler(void){

  if(SSD_Current_screenUpdateFlag == 1 && displayTracker == SSD_CURRENT_DISPLAY){
    floatUpdateScreen(&SSDcurrent, &SSD_Current_numberWrittenFlag); // flag checks if the entire number has been written to the display
    SSD_Current_screenUpdateFlag = 0; // resets flag to indicate that the screen has been updated
    displayTracker = SSD_VOLTAGE_DISPLAY; //switch to voltage display for the next update
  }
  if(SSD_Current_screenUpdateFlag == 1 && displayTracker == SSD_VOLTAGE_DISPLAY){
    floatUpdateScreen(&SSDvoltage, &SSD_Current_numberWrittenFlag); // flag checks if the entire number has been written to the display
    SSD_Current_screenUpdateFlag = 0; // resets flag to indicate that the screen has been updated
    displayTracker = SSD_CURRENT_DISPLAY; //switch to current display for the next update
  }

  if(indicator_led_updateFlag == 1){
    indicatorLEDwrite(&indicator_leds);
    indicator_led_updateFlag = 0;
  }

  if(encoder_flag == 1){
    encoderTimerElapsed(&rotaryEncoder); //call function to handle encoder rotation
    encoder_flag = 0; //reset flag
    encoder_rotated = 1; //set flag to indicate encoder has been rotated
    editModeTimeoutCNT = 0; // reset the counter to keep from exiting
  }

  if(editMode == EDIT_MODE_EN){
    if(encoder_rotated == 1){ //god I hope this is self explanatory 
      editModeTimeoutCNT = 0; // reset the counter to keep from exiting

      float ajd; //set variable for incrementing the desired value

      if (unitOfAdjustment == UNIT_ADJUSTMENT_1mA){
        ajd = 0.001f;
      }
      else if (unitOfAdjustment == UNIT_ADJUSTMENT_10mA){
        ajd = 0.01f;
      }
      else if (unitOfAdjustment == UNIT_ADJUSTMENT_100mA){
        ajd = 0.1f;
      }
      else if (unitOfAdjustment == UNIT_ADJUSTMENT_1A){
        ajd = 1.0f;
      }

      if(loadMode == MODE_CC){
        if(rotaryEncoder.encoder_up == 1){
          floatCommandedCurrent += ajd;
        }
        if(rotaryEncoder.encoder_down == 1){
          floatCommandedCurrent -= ajd;
        }
      }

      if(loadMode == MODE_CV){
        if(rotaryEncoder.encoder_up == 1){
          floatCommandedVoltage += ajd;
        }
        if(rotaryEncoder.encoder_down == 1){
          floatCommandedVoltage -= ajd;
        }
      }

      encoder_rotated = 0; //reset flag
    }
  }

  if(encoder_button_flag == 1){ //if the button on the rotary encoder has been pressed

    editModeTimeoutCNT = 0; // reset the counter to keep from exiting 

    if (editMode == EDIT_MODE_DIS){ //enable edit mode, start the TIM7 for timeout
      editMode = EDIT_MODE_EN;
      HAL_TIM_Base_Start_IT(&htim7); // start timer 7 for the timeout of edit mode
    }

    if(editMode == EDIT_MODE_EN){
      //change unit of ajustment e.g. 1mA, 10mA, 100mA e.c.t.
      if(loadMode == MODE_CC){ //cycle through the units of adjustments, and digit flashing
        if(unitOfAdjustment == UNIT_ADJUSTMENT_1mA){
          flashDigit(&SSDcurrent, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_10mA;
          flashDigit(&SSDcurrent, SSD_D3, 1); //flash the digit that is being adjusted
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_10mA){
          flashDigit(&SSDcurrent, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_100mA;
          flashDigit(&SSDcurrent, SSD_D2, 1);
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_100mA){
          flashDigit(&SSDcurrent, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_1A;
          flashDigit(&SSDcurrent, SSD_D1, 1);
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_1A){
          flashDigit(&SSDcurrent, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_1mA;
          flashDigit(&SSDcurrent, SSD_D4, 1);
        }
      }

      if(loadMode == MODE_CV){ //cycle through the units of adjustments, and digit flashing
        if(unitOfAdjustment == UNIT_ADJUSTMENT_1mA){
          flashDigit(&SSDvoltage, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_10mA;
          flashDigit(&SSDvoltage, SSD_D3, 1); //flash the digit that is being adjusted
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_10mA){
          flashDigit(&SSDvoltage, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_100mA;
          flashDigit(&SSDvoltage, SSD_D2, 1);
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_100mA){
          flashDigit(&SSDvoltage, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_1A;
          flashDigit(&SSDvoltage, SSD_D1, 1);
        }
        else if(unitOfAdjustment == UNIT_ADJUSTMENT_1A){
          flashDigit(&SSDvoltage, SSD_ALL, 0);
          unitOfAdjustment = UNIT_ADJUSTMENT_1mA;
          flashDigit(&SSDvoltage, SSD_D4, 1);
        }
      }
    }


    encoder_button_flag = 0; //reset flag
  }

  if(mode_button_pressed_flag == 1){ //if the button has been pressed

    //if the load is in CC, switch to CV and vice versa and toggle LED's
    if (loadMode == MODE_CC){
      loadMode = MODE_CV;
      indicatorLEDon(&indicator_leds, IND_LED_CV);
      indicatorLEDoff(&indicator_leds, IND_LED_CC);
      editModeTimeoutCNT = 3; //force timeout to allow digits to switch
    }
    else if (loadMode == MODE_CV){
      loadMode = MODE_CC;
      indicatorLEDon(&indicator_leds, IND_LED_CC);
      indicatorLEDoff(&indicator_leds, IND_LED_CV);
      editModeTimeoutCNT = 3;
    }

    mode_button_pressed_flag = 0; // reset the flag
  }

}

void reportStates(void){ //for reporting the status of the machine via UART to a connected PC

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  /* User can add his own implementation to report the HAL error return state */

  indicatorLEDon(&indicator_leds, IND_LED_ERROR); // Turn error LED on
  indicatorLEDwrite(&indicator_leds);
  
  __disable_irq();
  while (1)
  {

  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
