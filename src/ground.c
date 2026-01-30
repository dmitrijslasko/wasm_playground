#include <emscripten/emscripten.h>
#include <stdint.h>

#include "settings.h"
#include "render_utils.h"


static uint8_t *ground_pixels = NULL;
static int ground_w = 0;
static int ground_h = 0;

EMSCRIPTEN_KEEPALIVE
void set_ground_texture(uint8_t *pixels, int w, int h)
{
    ground_pixels = pixels;
    ground_w = w;
    ground_h = h;
}

void draw_ground(uint32_t *framebuffer)
{
    if (!ground_pixels || ground_w <= 0 || ground_h <= 0) {
        for (int y = GROUND_LEVEL_Y; y < FRAMEBUFFER_HEIGHT; y++) {
            for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                    pack_rgba(0, 255, 0, 255);
            }
        }
        return;
    }

    const int ground_height = FRAMEBUFFER_HEIGHT - GROUND_LEVEL_Y;
    for (int y = GROUND_LEVEL_Y; y < FRAMEBUFFER_HEIGHT; y++) {
        int sy = ((y - GROUND_LEVEL_Y) * ground_h) / ground_height;
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            int sx = (x * ground_w) / FRAMEBUFFER_WIDTH;
            int si = (sy * ground_w + sx) * 4;
            uint8_t r = ground_pixels[si + 0];
            uint8_t g = ground_pixels[si + 1];
            uint8_t b = ground_pixels[si + 2];
            uint8_t a = ground_pixels[si + 3];
            framebuffer[y * FRAMEBUFFER_WIDTH + x] = pack_rgba(r, g, b, a);
        }
    }
}