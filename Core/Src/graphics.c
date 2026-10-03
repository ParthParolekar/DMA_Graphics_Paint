/*
 * graphics.c
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

#include "graphics.h"
#include "framebuffer.h"

void gfx_set_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= 240 || y >= 320) return;   // bounds check — silently ignore out-of-range
    fb_set_pixel(x, y, color);
}

void gfx_clear_screen(uint16_t color){
	fb_clear(color);
}

void gfx_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	if(x >= 240 || y>=320 || length == 0) return;
	if (x + length >= 240) length = 240 - x;
	fb_set_hline(x, y, length, color);
}

void gfx_draw_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	if(x >= 240 || y>=320 || length == 0) return;
	if (y + length >= 320) length = 320 - y;
	fb_set_vline(x, y, length, color);
}

void gfx_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t fill){

	if(fill){
		for (uint16_t i = 0; i < height; i++){
			gfx_draw_hline(x, y+i, width, color);
		}
	}else{
		gfx_draw_hline(x, y, width, color);
		gfx_draw_hline(x, y + height - 1, width, color);
		gfx_draw_vline(x, y, height, color);
		gfx_draw_vline(x + width - 1, y, height, color);
	}
}

void gfx_draw_brush_stamp(uint16_t cx, uint16_t cy, uint16_t size, uint16_t color){
	uint16_t half = size/2;
	uint16_t x = cx >= half ? cx - half : 0;
	uint16_t y = cy >= half ? cy - half : 0;

	if(x + size > 240) x = 240-size;
	if(y + size > 320) y = 320-size;

	gfx_draw_rectangle(x, y, size, size, color, 1);
}

void gfx_paint_stamp(uint16_t cx, uint16_t cy, uint16_t size, uint16_t color){
	uint16_t half = size/2;
	uint16_t x = cx >= half ? cx - half : 0;
	uint16_t y = cy >= half ? cy - half : 0;

	if(x + size > 240) x = 240-size;
	if(y + size > 320) y = 320-size;

	fb_paint_rectangle(x, y, size, size, color);
}

// Paint a thick line from (x0,y0) to (x1,y1) by stamping along it.
// Writes straight to the display, like gfx_paint_stamp().
void gfx_paint_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                    uint16_t size, uint16_t color) {
    int32_t dx = (int32_t)x1 - x0;
    int32_t dy = (int32_t)y1 - y0;

    int32_t adx = dx < 0 ? -dx : dx;
    int32_t ady = dy < 0 ? -dy : dy;
    int32_t dist = adx > ady ? adx : ady;      // length along the longer axis

    int32_t step = size / 2;                   // distance between stamps
    if (step < 1) step = 1;

    int32_t n = (dist + step - 1) / step;      // number of steps, rounded up

    for (int32_t i = 0; i <= n; i++) {
        int32_t x = n ? x0 + dx * i / n : x0;
        int32_t y = n ? y0 + dy * i / n : y0;
        gfx_paint_stamp((uint16_t)x, (uint16_t)y, size, color);
    }
}


