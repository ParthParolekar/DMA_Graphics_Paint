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

typedef struct {
    uint8_t  *data;       // pixel storage for this tile
    uint32_t  capacity;   // size of data in bytes
    uint16_t  x, y, w, h; // which screen rectangle it currently represents
} fb_tile_t;

static uint8_t buffer_a[FB_BUF_W * FB_BUF_H * 2];		//Size = 240 * 20 * 2 = 9600 bytes
static uint8_t buffer_b[FB_BUF_W * FB_BUF_H * 2];		//Size = 240 * 20 * 2 = 9600 bytes

static fb_tile_t tile_a = {
    .data = buffer_a, .capacity = sizeof(buffer_a),
    .x = 0, .y = 0, .w = FB_BUF_W, .h = FB_BUF_H
};

static fb_tile_t tile_b = {
    .data = buffer_b, .capacity = sizeof(buffer_b),
    .x = 0, .y = 0, .w = FB_BUF_W, .h = FB_BUF_H
};

static fb_tile_t *current = &tile_a;

static void fb_swap_tile(void){
	current = (current == &tile_a) ? &tile_b : &tile_a;
}



uint8_t fb_set_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
	display_wait_ready();	//Wait for the DMA transfer to complete if already ongoing

	fb_tile_t *t = current;

	if(w == 0 || h==0) return 0;
	if((uint32_t)w * h * 2 > t->capacity) return 0;
	t->x = x;
	t->y = y;
	t->w = w;
	t->h = h;
	return 1;
}

void fb_fill_tile(uint16_t color){
	display_wait_ready();	//Wait for the DMA transfer to complete if already ongoing

	fb_tile_t *t = current;

	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;
	uint32_t total = (uint32_t)t->w * t->h * 2;

	for(uint32_t i = 0; i < total; i += 2){
		t->data[i] = hi;
		t->data[i + 1] = lo;
	}
}

void fb_begin_tile(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bg_color){
	if(fb_set_tile(x, y, w, h)){
		fb_fill_tile(bg_color);
	}
}

void fb_set_pixel(uint16_t x, uint16_t y, uint16_t color) {
	fb_tile_t *t = current;
    if(x < t->x || x >= t->x + t->w) return;
    if(y < t->y || y >= t->y + t->h) return;

    uint32_t offset = (uint32_t)(y - t->y) * t->w * 2 + (uint32_t)(x - t->x) * 2;
    t->data[offset] = color >> 8;
    t->data[offset + 1] = color & 0xFF;
}

void fb_set_hline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	fb_tile_t *t = current;

	if(length == 0) return;

	if (y < t->y || y >= t->y + t->h) return;

	uint32_t x_start = x;
	uint32_t x_end = (uint32_t)x + length;

	if(x_start < t->x)	x_start = t->x;
	if(x_end > (uint32_t)t->x + t->w) x_end = (uint32_t)t->x + t->w;
	if(x_start >= x_end)		return;

	uint32_t offset = (uint32_t)(y - t->y) * t->w * 2 + (uint32_t)(x_start - t->x) * 2;

	for(uint32_t i = x_start; i<x_end; i++){
		t->data[offset] = color >> 8;
		t->data[offset + 1] = color & 0xFF;
		offset += 2;
	}
}

void fb_set_vline(uint16_t x, uint16_t y, uint16_t length, uint16_t color){
	if (length == 0) return;
	fb_tile_t *t = current;

	if(x < t->x || x >= t->x + t->w) return;

	uint32_t y_start = y;
	uint32_t y_end = (uint32_t)y + length;

	if(y_start < t->y)	y_start = t->y;
	if(y_end > (uint32_t)t->y + t->h) y_end = (uint32_t)t->y + t->h;
	if(y_start >= y_end)	return;

	uint32_t offset = (uint32_t)(y_start - t->y) * t->w * 2 + (uint32_t)(x - t->x) * 2;

	for(uint32_t i = y_start; i < y_end; i++){
		t->data[offset] = color >> 8;
		t->data[offset + 1] = color & 0xFF;
		offset += t->w * 2;
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
	fb_tile_t *t = current;

    uint16_t x1 = t->x + t->w - 1;
    uint16_t y1 = t->y + t->h - 1;
    uint16_t len = t->w * t->h * 2;

    set_window(t->x, t->y, x1, y1);
    display_data_buffer_dma(t->data, len);

    fb_swap_tile();
}

void fb_clear(uint16_t color) {
    fill_screen(color);   // Stage 2: pass-through. Stage 3: fill the tile buffer(s) instead.
}


