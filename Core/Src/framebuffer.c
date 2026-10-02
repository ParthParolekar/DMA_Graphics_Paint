/*
 * framebuffer.c
 *
 *  Created on: 28-Sept-2026
 *      Author: Parth
 */

#include "framebuffer.h"
#include "display_driver.h"

#define FB_BUF_W 	240
#define FB_BUF_H	20

static uint8_t tile_buffer[FB_BUF_W * FB_BUF_H * 2];		//Size = 240 * 20 * 2 = 9600 bytes

//The point at which the tile sits and the size
static uint16_t tile_x = 0;
static uint16_t tile_y = 0;
static uint16_t tile_w = FB_BUF_W;
static uint16_t tile_h = FB_BUF_H;

uint8_t fb_set_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	if(w == 0 || h==0) return 0;
	if((uint32_t)w * h > (uint32_t)FB_BUF_W * FB_BUF_H) return 0;
	tile_x = x;
	tile_y = y;
	tile_w = w;
	tile_h = h;
	return 1;
}

void fb_fill_tile(uint16_t color){
	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	uint32_t total = (uint32_t)tile_w * tile_h * 2;

	for(uint32_t i = 0; i < total; i += 2){
		tile_buffer[i] = hi;
		tile_buffer[i + 1] = lo;
	}
}

void fb_begin_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bg_color){
	if(fb_set_tile(x, y, w, h)){
		fb_fill_tile(bg_color);
	}
}

void fb_set_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if(x < tile_x || x >= tile_x + tile_w) return;
    if(y < tile_y || y >= tile_y + tile_h) return;

    uint32_t offset = (uint32_t)(y - tile_y) * tile_w * 2 + (uint32_t)(x - tile_x) * 2;
    tile_buffer[offset] = color >> 8;
    tile_buffer[offset + 1] = color & 0xFF;
}

void fb_set_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	if(length == 0) return;

	if (y < tile_y || y >= tile_y + tile_h) return;

	uint32_t x_start = x;
	uint32_t x_end = (uint32_t)x + length;

	if(x_start < tile_x)	x_start = tile_x;
	if(x_end > (uint32_t)tile_x + tile_w) x_end = (uint32_t)tile_x + tile_w;
	if(x_start >= x_end)		return;

	uint32_t offset = (uint32_t)(y - tile_y) * tile_w * 2 + (uint32_t)(x_start - tile_x) * 2;

	for(uint32_t i = x_start; i<x_end; i++){
		tile_buffer[offset] = color >> 8;
		tile_buffer[offset + 1] = color & 0xFF;
		offset += 2;
	}
}

void fb_set_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	if (length == 0) return;

	if(x < tile_x || x >= tile_x + tile_w) return;

	uint32_t y_start = y;
	uint32_t y_end = (uint32_t)y + length;

	if(y_start < tile_y)	y_start = tile_y;
	if(y_end > (uint32_t)tile_y + tile_h) y_end = (uint32_t)tile_y + tile_h;
	if(y_start >= y_end)	return;

	uint32_t offset = (uint32_t)(y_start - tile_y) * tile_w * 2 + (uint32_t)(x - tile_x) * 2;

	for(uint32_t i = y_start; i < y_end; i++){
		tile_buffer[offset] = color >> 8;
		tile_buffer[offset + 1] = color & 0xFF;
		offset += tile_w * 2;
	}
}

void fb_paint_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    if (fb_set_tile(x, y, w, h)) {     // only proceed if the size fits the buffer
        fb_fill_tile(color);
        fb_flush();
    }
}

void fb_render(void (*draw_scene)(void), uint16_t bg_color){
	for (uint16_t row = 0; row < 320; row += FB_BUF_H){
		fb_begin_tile(0, row, FB_BUF_W, FB_BUF_H, bg_color);
		draw_scene();
		fb_flush();
	}
}

void fb_flush(void) {
    uint16_t x1 = tile_x + tile_w - 1;
    uint16_t y1 = tile_y + tile_h - 1;
    uint16_t len = tile_w * tile_h * 2;

    set_window(tile_x, tile_y, x1, y1);
    display_data_buffer(tile_buffer, len);
}

void fb_clear(uint16_t color) {
    fill_screen(color);   // Stage 2: pass-through. Stage 3: fill the tile buffer(s) instead.
}


