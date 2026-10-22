/* USER CODE BEGIN Header */
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
#include "cmsis_os.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include "display_driver.h"
#include "framebuffer.h"
#include "graphics.h"
#include "touch.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BRUSH_SIZE		4
#define BRUSH_COLOR  0xFFFF
#define MAX_JUMP     30
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile UBaseType_t paint_hwm;		// To measure paint task stack usage
static TaskHandle_t paint_handle;

static void test_scene(void) {
    gfx_draw_rectangle(20, 10, 200, 60, 0xF800, 0);      // red outline, crosses strips 0-3
    gfx_draw_rectangle(40, 100, 80, 50, 0x07E0, 1);      // filled green, crosses strips 5-7
    gfx_draw_vline(200, 0, 320, 0x001F);                 // blue line, full screen height
    gfx_draw_brush_stamp(120, 20, 10, 0xFFFF);           // white stamp on the strip 0/1 boundary
    gfx_draw_brush_stamp(120, 200, 30, 0xFFE0);          // yellow stamp, mid-screen
}

static void clear_screen_scene(void) {
    gfx_draw_rectangle(0, 0, 240, 320, 0x0000, 1);      // Clear screen

}

static void test_task(void *arg)
{
    for (;;) {
    	paint_hwm = uxTaskGetStackHighWaterMark(paint_handle);
        HAL_UART_Transmit(&huart2, (uint8_t *)"tick\r\n", 6, HAL_MAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void paint_task(void *arg){

	uint8_t  stroking = 0;          // 1 while the stylus is down and moving
	uint8_t  rejected = 0;          // consecutive reads thrown away as jumps
	uint16_t last_x = 0, last_y = 0;

	for(;;){
	    uint16_t ax, ay, sx, sy;

	    if (touch_read_averaged(&ax, &ay)) {
	        touch_to_screen(ax, ay, &sx, &sy);

	        if (stroking) {
	            if (abs((int)sx - last_x) > MAX_JUMP || abs((int)sy - last_y) > MAX_JUMP) {
	                if (++rejected < 3) continue;     // ignore this one
	                stroking = 0;                      // three in a row: really moved, start a new stroke
	            }
	        }
	        rejected = 0;

	        if (stroking) {
	            gfx_paint_line(last_x, last_y, sx, sy, BRUSH_SIZE, BRUSH_COLOR);
	        } else {
	            gfx_paint_stamp(sx, sy, BRUSH_SIZE, BRUSH_COLOR);    // first point of a stroke
	            stroking = 1;
	        }
	        last_x = sx;
	        last_y = sy;
	    } else {
	        stroking = 0;           // stylus lifted: next touch starts a new stroke
	        rejected = 0;
	        vTaskDelay(pdMS_TO_TICKS(5));
	    }
	}

}
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  display_dma_init();
  display_init();


  /************** Basic Function Tests **************/

  fb_render(clear_screen_scene, 0x0000);        // black background

  xTaskCreate(test_task, "test", 128, NULL, 1, NULL);
  xTaskCreate(paint_task, "paint", 192, NULL, 2, &paint_handle);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
