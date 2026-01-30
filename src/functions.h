#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdint.h>

void set_sky_texture1(uint8_t *pixels, int w, int h);
void set_sky_texture2(uint8_t *pixels, int w, int h);
void draw_sky1(uint32_t *framebuffer, float dt);
void draw_sky2(uint32_t *framebuffer, float dt);

void set_ground_texture(uint8_t *pixels, int w, int h);
void draw_ground(uint32_t *framebuffer, float dt);

#endif