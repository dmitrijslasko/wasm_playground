#include <emscripten/emscripten.h>
#include <stdint.h>
#include <stdio.h>

#include "settings.h"
#include "render_utils.h"


static uint8_t *sky1_pixels = NULL;
static int sky1_w = 0;
static int sky1_h = 0;
static float sky1_scroll_x = 0;

static uint8_t *sky2_pixels = NULL;
static int sky2_w = 0;
static int sky2_h = 0;
static float sky2_scroll_x = 0;


EMSCRIPTEN_KEEPALIVE
void set_sky_texture1(uint8_t *pixels, int w, int h)
{
    sky1_pixels = pixels;
    sky1_w = w;
    sky1_h = h;
}

EMSCRIPTEN_KEEPALIVE
void set_sky_texture2(uint8_t *pixels, int w, int h)
{
    sky2_pixels = pixels;
    sky2_w = w;
    sky2_h = h;
}
EMSCRIPTEN_KEEPALIVE
void draw_sky1(uint32_t *framebuffer, float dt)
{
    sky1_scroll_x += dt * -20;

    if (sky1_scroll_x <= -sky1_w)
        sky1_scroll_x += sky1_w;

    render_layer(sky1_pixels, sky1_w, sky1_h, sky1_scroll_x, 0, framebuffer);
    render_layer(sky1_pixels, sky1_w, sky1_h, sky1_w + sky1_scroll_x, 0, framebuffer);
}

EMSCRIPTEN_KEEPALIVE
void draw_sky2(uint32_t *framebuffer, float dt)
{
    sky2_scroll_x += dt * -15;

    if (sky2_scroll_x <= -sky2_w)
        sky2_scroll_x += sky2_w;

    render_layer(sky2_pixels, sky2_w, sky2_h, sky2_scroll_x, 0, framebuffer);
    render_layer(sky2_pixels, sky2_w, sky2_h, sky2_w + sky2_scroll_x, 0, framebuffer);
}