#include <emscripten/emscripten.h>
#include <stdint.h>

#include "settings.h"
#include "render_utils.h"


static uint8_t *sky_pixels = NULL;
static int sky_w = 0;
static int sky_h = 0;


EMSCRIPTEN_KEEPALIVE
void set_sky_texture(uint8_t *pixels, int w, int h)
{
    sky_pixels = pixels;
    sky_w = w;
    sky_h = h;
}

EMSCRIPTEN_KEEPALIVE
void draw_sky(uint32_t *framebuffer)
{
    if (!sky_pixels || sky_w <= 0 || sky_h <= 0) {
        return;
    }

    for (int y = 0; y < GROUND_LEVEL_Y; y++) {
        int sy = (y * sky_h) / GROUND_LEVEL_Y;
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            int sx = (x * sky_w) / FRAMEBUFFER_WIDTH;
            int si = (sy * sky_w + sx) * 4;
            uint8_t r = sky_pixels[si + 0];
            uint8_t g = sky_pixels[si + 1];
            uint8_t b = sky_pixels[si + 2];
            uint8_t a = sky_pixels[si + 3];
            framebuffer[y * FRAMEBUFFER_WIDTH + x] = pack_rgba(r, g, b, a);
        }
    }
}