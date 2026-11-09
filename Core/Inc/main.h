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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define R_LED_Pin GPIO_PIN_13
#define R_LED_GPIO_Port GPIOC
#define SC_NRST_Pin GPIO_PIN_7
#define SC_NRST_GPIO_Port GPIOA
#define DIO1_Pin GPIO_PIN_0
#define DIO1_GPIO_Port GPIOB
#define DIO1_EXTI_IRQn EXTI0_IRQn
#define DIO3_Pin GPIO_PIN_1
#define DIO3_GPIO_Port GPIOB
#define UARTRF_TX_Pin GPIO_PIN_10
#define UARTRF_TX_GPIO_Port GPIOB
#define UARTRF_RX_Pin GPIO_PIN_11
#define UARTRF_RX_GPIO_Port GPIOB
#define RF_NRST_Pin GPIO_PIN_8
#define RF_NRST_GPIO_Port GPIOA
#define SPI_RF_CS_Pin GPIO_PIN_9
#define SPI_RF_CS_GPIO_Port GPIOA
#define BUSY_Pin GPIO_PIN_12
#define BUSY_GPIO_Port GPIOA
#define I2C_EEPROM_SCL_Pin GPIO_PIN_6
#define I2C_EEPROM_SCL_GPIO_Port GPIOB
#define I2C_EEPROM_SCK_Pin GPIO_PIN_7
#define I2C_EEPROM_SCK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
