/*
 * framebuffer.c
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

#include "framebuffer.h"
#include "display_driver.h"

void fb_set_pixel(uint16_t x, uint16_t y, uint16_t color) {
    draw_pixel(x, y, color);   // Stage 2: pass-through. Stage 3: write into tile buffer instead.
}

void fb_clear(uint16_t color) {
    fill_screen(color);   // Stage 2: pass-through. Stage 3: fill the tile buffer(s) instead.
}

void fb_set_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	draw_hline(x, y, length, color);
}

void fb_set_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	draw_vline(x, y, length, color);
}

void fb_flush(void) {
    // Stage 2: nothing to do, draw_pixel() already pushed to display directly.
    // Stage 3: this is where tile → display push will happen.
}
