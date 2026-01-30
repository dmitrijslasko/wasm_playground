#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h>

void set_sky_texture(uint8_t *pixels, int w, int h);
void draw_sky(uint32_t *framebuffer);

void set_ground_texture(uint8_t *pixels, int w, int h);
void draw_ground(uint32_t *framebuffer);

#endif