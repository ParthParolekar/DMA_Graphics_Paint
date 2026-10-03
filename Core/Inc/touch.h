/*
 * touch.h
 *
 *  Created on: 02-Oct-2026
 *      Author: Parth
 */
#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>

// Read one raw 12-bit sample (0-4095) for X and Y from the XPT2046.
void touch_read_raw(uint16_t *x, uint16_t *y);
uint8_t touch_is_pressed(void);
uint8_t touch_read_averaged(uint16_t *x, uint16_t *y);
void touch_to_screen(uint16_t raw_x, uint16_t raw_y, uint16_t *sx, uint16_t *sy);

#endif
