#include <emscripten/emscripten.h>
#include <stdint.h>

#include "settings.h"
#include "render_utils.h"


static uint8_t *ground_pixels = NULL;
static int ground_w = 0;
static int ground_h = 0;
static float ground_scroll_x = 0;

static uint8_t *ground_pixels2 = NULL;
static int ground_w2 = 0;
static int ground_h2 = 0;
static float ground_scroll_x2 = 0;

EMSCRIPTEN_KEEPALIVE
void set_ground_texture(uint8_t *pixels, int w, int h)
{
    ground_pixels = pixels;
    ground_w = w;
    ground_h = h;
}

EMSCRIPTEN_KEEPALIVE
void set_ground_texture2(uint8_t *pixels, int w, int h)
{
	ground_pixels2 = pixels;
	ground_w2 = w;
	ground_h2 = h;
}

void draw_ground(uint32_t *framebuffer, float dt)
{
    ground_scroll_x += dt * -50;

    if (ground_scroll_x <= -ground_w)
        ground_scroll_x += ground_w;

    render_layer(ground_pixels, ground_w, ground_h, ground_scroll_x, GROUND_LEVEL_Y, framebuffer);
    render_layer(ground_pixels, ground_w, ground_h, ground_w + ground_scroll_x, GROUND_LEVEL_Y, framebuffer);
}

void draw_ground2(uint32_t *framebuffer, float dt)
{
    ground_scroll_x2  += dt * -120;

    if (ground_scroll_x2 <= -ground_w2)
        ground_scroll_x2 += ground_w2;

    render_layer(ground_pixels2, ground_w2, ground_h2, ground_scroll_x2, GROUND_LEVEL_Y + 60, framebuffer);
    render_layer(ground_pixels2, ground_w2, ground_h2, ground_w2 + ground_scroll_x2, GROUND_LEVEL_Y + 60, framebuffer);
}