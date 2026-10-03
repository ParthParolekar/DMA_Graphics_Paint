/*
 * touch.c
 *
 *  Created on: 02-Oct-2026
 *      Author: Parth
 */

#include "touch.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

#define TOUCH_CMD_X  	0xD0
#define TOUCH_CMD_Y  	0x90

#define SAMPLES			8

// Raw readings at the edges of the panel (from corner tests)
#define RAW_X_MIN   215      // left edge
#define RAW_X_MAX  3770      // right edge
#define RAW_Y_MIN   270      // bottom edge
#define RAW_Y_MAX  3870      // top edge

#define SPREAD_MAX   100     // max allowed disagreement between samples (~7 px)
#define EDGE_MARGIN  150     // how far outside the calibrated range a real touch may read

static uint16_t touch_read_channel(uint8_t cmd){
	uint8_t tx[3] = {cmd, 0x00, 0x00};
	uint8_t rx[3] = {0};

	 HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_RESET);   // select
	 HAL_SPI_TransmitReceive(&hspi2, tx, rx, 3, HAL_MAX_DELAY);
	 HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);     // deselect

//	 return (uint16_t)(((rx[1] << 8) | (rx[2]) >>3)) & 0x0FFF;
	 return (uint16_t)(((rx[1] << 8) | rx[2]) >> 3) & 0x0FFF;
}

void touch_read_raw(uint16_t *x, uint16_t *y) {
    touch_read_channel(TOUCH_CMD_X);        // throwaway: first conversion after idle can be off
    *x = touch_read_channel(TOUCH_CMD_X);
    touch_read_channel(TOUCH_CMD_Y);        // throwaway after switching to Y
    *y = touch_read_channel(TOUCH_CMD_Y);
}

uint8_t touch_is_pressed(void){
	return HAL_GPIO_ReadPin(TOUCH_IRQ_GPIO_Port, TOUCH_IRQ_Pin) ==  GPIO_PIN_RESET;
}

uint8_t touch_read_averaged(uint16_t *x, uint16_t *y){
	if(!touch_is_pressed()) return 0;

	uint32_t sum_x = 0;
	uint32_t sum_y = 0;

	uint16_t min_x = 0xFFFF, max_x = 0;
	uint16_t min_y = 0xFFFF, max_y = 0;


	for(uint8_t i = 0; i < SAMPLES; i++){
		if(!touch_is_pressed()) return 0;

		uint16_t rx, ry;
		touch_read_raw(&rx, &ry);
		sum_x += rx;
		sum_y += ry;

		if(rx < min_x) min_x = rx;
		if(rx > max_x) max_x = rx;
		if(ry < min_y) min_y = ry;
		if(ry > max_y) max_y = ry;
	}

	if(!touch_is_pressed()) return 0;

	// Samples disagree too much: a glitch or an unsettled contact
	if((max_x - min_x) > SPREAD_MAX || (max_y - min_y) > SPREAD_MAX) return 0;

	// Readings far outside the calibrated range are "no touch", not an edge touch
	if(min_x < RAW_X_MIN - EDGE_MARGIN || max_x > RAW_X_MAX + EDGE_MARGIN) return 0;
	if(min_y < RAW_Y_MIN - EDGE_MARGIN || max_y > RAW_Y_MAX + EDGE_MARGIN) return 0;

	// Drop the single lowest and highest sample, average the rest
	*x = (sum_x - min_x - max_x) / (SAMPLES - 2);
	*y = (sum_y - min_y - max_y) / (SAMPLES - 2);
	return 1;
}

void touch_to_screen(uint16_t raw_x, uint16_t raw_y, uint16_t *sx, uint16_t *sy){
	int32_t x = ((int32_t)raw_x - RAW_X_MIN) * 239 / (RAW_X_MAX - RAW_X_MIN);
	int32_t y = ((int32_t)RAW_Y_MAX - (int32_t)raw_y) * 319 / (RAW_Y_MAX - RAW_Y_MIN);

	if(x < 0) x = 0;
	if(x > 239) x = 239;
	if(y < 0) y = 0;
	if(y > 319) y = 319;

	*sx = (uint16_t)x;
	*sy = (uint16_t)y;
}
