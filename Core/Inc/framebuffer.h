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
void fb_clear(uint16_t color);
void fb_flush(void);
void fb_set_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);
void fb_set_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color);

#endif /* INC_FRAMEBUFFER_H_ */
