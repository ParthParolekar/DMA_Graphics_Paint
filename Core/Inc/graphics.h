/*
 * graphics.h
 *
 *  Created on: 28-Sept-2026
 *      Author: Lenovo
 */

#ifndef INC_GRAPHICS_H_
#define INC_GRAPHICS_H_

#include <stdint.h>

void gfx_set_pixel(uint16_t x, uint16_t y, uint16_t color);
void gfx_clear_screen(uint16_t color);
void gfx_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
void gfx_draw_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
void gfx_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t fill);
void gfx_draw_brush_stamp(uint16_t cx, uint16_t cy, uint16_t size, uint16_t color);
void gfx_paint_stamp(uint16_t cx, uint16_t cy, uint16_t size, uint16_t color);
void gfx_paint_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t size, uint16_t color);
#endif /* INC_GRAPHICS_H_ */
