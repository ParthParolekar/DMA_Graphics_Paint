/*
 * framebuffer.h
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

#ifndef INC_FRAMEBUFFER_H_
#define INC_FRAMEBUFFER_H_

#include <stdint.h>

void fb_set_pixel(uint16_t x, uint16_t y, uint16_t color);
uint8_t fb_set_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void fb_begin_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bg_color);	//To make sure no stale data is in buffer
void fb_fill_tile(uint16_t color);
void fb_set_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
void fb_set_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
void fb_paint_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void fb_render(void (*draw_scene)(void), uint16_t bg_color);
void fb_clear(uint16_t color);
void fb_flush(void);
#endif /* INC_FRAMEBUFFER_H_ */
