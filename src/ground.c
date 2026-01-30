#include <emscripten/emscripten.h>
#include <stdint.h>

#include "settings.h"
#include "render_utils.h"


static uint8_t *ground_pixels = NULL;
static int ground_w = 0;
static int ground_h = 0;
static float ground_scroll_x = 0;

EMSCRIPTEN_KEEPALIVE
void set_ground_texture(uint8_t *pixels, int w, int h)
{
    ground_pixels = pixels;
    ground_w = w;
    ground_h = h;
}

void draw_ground(uint32_t *framebuffer, float dt)
{
    ground_scroll_x += dt * -25;

    if (ground_scroll_x <= -ground_w)
        ground_scroll_x += ground_w;

    render_layer(ground_pixels, ground_w, ground_h, ground_scroll_x, GROUND_LEVEL_Y, framebuffer);
    render_layer(ground_pixels, ground_w, ground_h, ground_w + ground_scroll_x, GROUND_LEVEL_Y, framebuffer);
}