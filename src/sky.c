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

static uint8_t *sky3_pixels = NULL;
static int sky3_w = 0;
static int sky3_h = 0;
static float sky3_scroll_x = 0;


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
void set_sky_texture3(uint8_t *pixels, int w, int h)
{
    sky3_pixels = pixels;
    sky3_w = w;
    sky3_h = h;
}

EMSCRIPTEN_KEEPALIVE
void draw_sky1(uint32_t *framebuffer, float dt)
{
    sky1_scroll_x += dt * 0;

    if (sky1_scroll_x <= -sky1_w)
        sky1_scroll_x += sky1_w;

    render_layer(sky1_pixels, sky1_w, sky1_h, sky1_scroll_x, 0, framebuffer);
    render_layer(sky1_pixels, sky1_w, sky1_h, sky1_w + sky1_scroll_x, 0, framebuffer);
}

EMSCRIPTEN_KEEPALIVE
void draw_sky2(uint32_t *framebuffer, float dt)
{
    sky2_scroll_x += dt * -10;

    if (sky2_scroll_x <= -sky2_w)
        sky2_scroll_x += sky2_w;

    render_layer(sky2_pixels, sky2_w, sky2_h, sky2_scroll_x, 300, framebuffer);
    render_layer(sky2_pixels, sky2_w, sky2_h, sky2_w + sky2_scroll_x, 300, framebuffer);
}

EMSCRIPTEN_KEEPALIVE
void draw_sky3(uint32_t *framebuffer, float dt)
{
    sky3_scroll_x += dt * -50;

    if (sky3_scroll_x <= -sky3_w)
        sky3_scroll_x += sky3_w;

    render_layer(sky3_pixels, sky3_w, sky3_h, sky3_scroll_x, 0, framebuffer);
    render_layer(sky3_pixels, sky3_w, sky3_h, sky3_w + sky3_scroll_x, 0, framebuffer);
}