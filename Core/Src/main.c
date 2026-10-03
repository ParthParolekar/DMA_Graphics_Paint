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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
  display_init();


  /************** Basic Function Tests **************/
  fb_render(clear_screen_scene, 0x0000);        // black background

  uint8_t  stroking = 0;          // 1 while the stylus is down and moving
  uint8_t  rejected = 0;          // consecutive reads thrown away as jumps
  uint16_t last_x = 0, last_y = 0;
  char msg[40];

//  while (1) {
//	  uint16_t ax, ay, sx, sy;
//	  if(touch_read_averaged(&ax, &ay)){
//
//		  touch_to_screen(ax, ay, &sx, &sy);
//		  gfx_paint_stamp(sx, sy, BRUSH_SIZE, 0xFFFF);
//	  }
//
//  }

  while (1) {
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
        }
    }

//  gfx_paint_stamp(60, 250, 10, 0xFFFF);    // white
//  gfx_paint_stamp(66, 252, 10, 0xF800);    // red, overlapping the white
//  gfx_paint_stamp(72, 254, 10, 0x07E0);    // green, overlapping the red
//  gfx_paint_stamp(0, 0, 12, 0x001F);       // top-left corner: should stay fully on-screen
//  gfx_paint_stamp(239, 319, 12, 0x001F);   // bottom-right corner: same

//
//  fb_set_tile(100, 150, 20, 20);
//  for (uint16_t i = 0; i < 20; i++){
//	  fb_set_pixel(100+i, 150+i, 0xFFFF);
//  }

//  fb_set_tile(40, 60, 30, 10);                 // 30 wide, 10 tall
//  fb_set_pixel(40, 60, 0xF800);                // top-left: red
//  fb_set_pixel(69, 60, 0x07E0);                // top-right: green
//  fb_set_pixel(40, 69, 0x001F);                // bottom-left: blue
//  fb_set_pixel(69, 69, 0xFFFF);                // bottom-right: white
//  fb_set_pixel(100, 100, 0xF800);              // outside the tile: should NOT appear
//  fb_flush();

//  fb_set_tile(40, 60, 30, 10);        // covers columns 40-69, rows 60-69
//  fb_set_hline(30, 65, 50, 0xF800);   // columns 30-79: overhangs both sides
//  fb_set_hline(45, 62, 10, 0x07E0);   // fully inside
//  fb_set_hline(40, 75, 20, 0x001F);   // row 75 is below the tile: should NOT appear
//  fb_flush();

//  fb_set_tile(40, 60, 30, 10);        // columns 40-69, rows 60-69
//  fb_set_vline(50, 55, 20, 0xF800);   // rows 55-74: overhangs top and bottom
//  fb_set_vline(60, 62, 4, 0x07E0);    // fully inside: rows 62-65
//  fb_set_vline(80, 60, 5, 0x001F);    // column 80 is right of the tile: should NOT appear
//  fb_flush();

//  fb_set_tile(40, 60, 30, 10);
//  fb_fill_tile(0x001F);                // blue background
//  fb_set_pixel(40, 60, 0xFFFF);        // white dot at top-left
//  fb_set_pixel(69, 69, 0xFFFF);        // white dot at bottom-right
//  fb_flush();
//
//  fb_set_tile(100, 150, 20, 5);        // move the tile, but don't fill
//  fb_set_pixel(100, 150, 0xF800);      // one red dot
//  fb_flush();

//  fb_begin_tile(40, 60, 30, 10, 0x001F);	// blue background
//  fb_set_pixel(40, 60, 0xFFFF);       	 	// white dot at top-left
//  fb_set_pixel(69, 69, 0xFFFF);        		// white dot at bottom-right
//  fb_flush();
//
//  fb_begin_tile(100, 150, 20, 5, 0x001F);
//  fb_set_pixel(100, 150, 0xF800);      		// one red dot
//  fb_flush();


//  gfx_set_pixel(0, 0, 0xFFFF);        // white dot at top-left
//  gfx_set_pixel(239, 0, 0xF800);      // red dot at top-right
//  gfx_set_pixel(0, 319, 0x07E0);      // green dot at bottom-left
//  gfx_set_pixel(239, 319, 0x001F);    // blue dot at bottom-right
//
//  gfx_draw_hline(10, 100, 150, 0xF800);
//  gfx_draw_vline(80, 25, 150, 0x07E0);
//
//
//  gfx_draw_brush_stamp(240, 0, 30, 0xF800);

  /* USER CODE END 2 */

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
