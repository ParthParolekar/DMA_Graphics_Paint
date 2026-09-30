/*
 * display_driver.c
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

// display_driver.c
#include "display_driver.h"
#include "main.h"          // pin names (LCD_CS, LCD_DC, LCD_RST) from CubeMX

extern SPI_HandleTypeDef hspi1;

void display_command(uint8_t cmd) {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);  // DC low = command
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);  // CS low = select
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);    // CS high = deselect
}

void display_data(uint8_t data) {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);    // DC high = data
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    display_command(0x2A);                       // CASET (columns)
    display_data(x0 >> 8); display_data(x0 & 0xFF);
    display_data(x1 >> 8); display_data(x1 & 0xFF);

    display_command(0x2B);                       // PASET (rows)
    display_data(y0 >> 8); display_data(y0 & 0xFF);
    display_data(y1 >> 8); display_data(y1 & 0xFF);

    display_command(0x2C);                       // RAMWR
}

void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    set_window(x, y, x, y);
    display_data(color >> 8);
    display_data(color & 0xFF);
}

void draw_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	set_window(x, y, x + length - 1, y);

	uint8_t hline[length * 2];

	for(uint16_t i=0; i < length; i++){
		hline[i*2] = color >> 8;
		hline[i*2 + 1] = color & 0xFF;
	}

	display_data_buffer(hline, sizeof(hline));
}

void draw_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	set_window(x, y, x, y + length - 1);

	uint8_t vline[length * 2];

	for(uint16_t i=0; i < length; i++){
		vline[i*2] = color >> 8;
		vline[i*2 + 1] = color & 0xFF;
	}

	display_data_buffer(vline, sizeof(vline));
}

void fill_screen(uint16_t color) {
    uint8_t line[LCD_WIDTH * 2];               // one row of RGB565 = 480 bytes

    for (uint16_t i = 0; i < LCD_WIDTH; i++){
    	line[i*2] = color >> 8;					//High byte
    	line[i*2 + 1] = color & 0xFF;			//Low byte
    }

    set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    for (uint16_t row = 0; row < LCD_HEIGHT; row++) {
       display_data_buffer(line, sizeof(line));
    }
}

void display_data_buffer(const uint8_t *buf, uint16_t len) {
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);    // DC high = data
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void display_init(void) {
    // Hardware reset
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(120);

    display_command(0x01);       // SWRESET
    HAL_Delay(150);

    display_command(0x11);       // SLPOUT
    HAL_Delay(120);

    display_command(0x3A);       // COLMOD
    display_data(0x55);          // RGB565

    display_command(0x36);       // MADCTL
    display_data(0x48);          // orientation + BGR for this panel

    // No INVON (0x21): this panel must stay non-inverted

    display_command(0x13);       // NORON
    display_command(0x29);       // DISPON
    HAL_Delay(20);
}
