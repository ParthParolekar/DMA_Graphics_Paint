/*
 * lcd_test.h
 *
 *  Created on: 24-Sept-2026
 *      Author: Parth
 */

#ifndef LCD_TEST_H
#define LCD_TEST_H

#include "main.h"   // brings in LCD_CS_Pin, LCD_DC_Pin, LCD_RST_Pin etc from CubeMX

void LCD_Init(void);
void LCD_FillScreen(uint16_t color);
void LCD_TestPattern(void);

#endif
