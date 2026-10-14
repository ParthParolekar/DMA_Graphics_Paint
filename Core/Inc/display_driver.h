/*
 * display_driver.h
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

#ifndef INC_DISPLAY_DRIVER_H_
#define INC_DISPLAY_DRIVER_H_

#include <stdint.h>

#define LCD_WIDTH   240
#define LCD_HEIGHT  320

void display_init(void);   // reset + init sequence
void display_command(uint8_t cmd);
void display_data(uint8_t data);
void display_data_buffer(const uint8_t *buf, uint16_t len);
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void fill_screen(uint16_t color);

void display_wait_ready(void);
uint32_t display_dma_error_count(void);
void display_dma_init(void);
void display_data_buffer_dma(const uint8_t *buf, uint16_t len);

#endif /* INC_DISPLAY_DRIVER_H_ */
