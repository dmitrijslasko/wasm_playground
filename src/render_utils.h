#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <stdint.h>
#include "settings.h"

typedef struct s_texture {
    uint8_t *pixels;
    int width;
    int height;

    int frame_width;
    int frame_height;
    int frame_count;

    int current_frame;
    float frame_time;
    float frame_duration;
} t_texture;

static inline uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t)(r | (g << 8) | (b << 16) | (a << 24));
}

static inline int aabb_overlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

static inline void render_layer(uint8_t *pixels, int w, int h, int x_position, int y_position, uint32_t *framebuffer)
{
    if (!pixels || w <= 0 || h <= 0) {
        return;
    }

    int start_x = x_position < 0 ? 0 : x_position;
    int start_y = y_position < 0 ? 0 : y_position;
    int end_x = x_position + w;
    int end_y = y_position + h;

    if (end_x > FRAMEBUFFER_WIDTH) end_x = FRAMEBUFFER_WIDTH;
    if (end_y > FRAMEBUFFER_HEIGHT) end_y = FRAMEBUFFER_HEIGHT;
    if (start_x >= end_x || start_y >= end_y) {
        return;
    }

    for (int y = start_y; y < end_y; y++) {
        int sy = y - y_position;
        for (int x = start_x; x < end_x; x++) {
            int sx = x - x_position;
            int si = (sy * w + sx) * 4;
            uint8_t r = pixels[si + 0];
            uint8_t g = pixels[si + 1];
            uint8_t b = pixels[si + 2];
            uint8_t a = pixels[si + 3];
            if (a == 0) {
                continue;
            }
            framebuffer[y * FRAMEBUFFER_WIDTH + x] = pack_rgba(r, g, b, a);
        }
    }
}

static inline void render_sprite_frame(
    t_texture *sprite,
    int size_x,
    int size_y,
    int x_position,
    int y_position,
    int frame_index,
    uint32_t *framebuffer
) {
    if (!sprite->pixels || sprite->width <= 0 || sprite->height <= 0)
        return;

    int start_x = x_position < 0 ? 0 : x_position;
    int start_y = y_position < 0 ? 0 : y_position;

    int end_x = x_position + size_x;
    int end_y = y_position + size_y;

    if (end_x > FRAMEBUFFER_WIDTH)  end_x = FRAMEBUFFER_WIDTH;
    if (end_y > FRAMEBUFFER_HEIGHT) end_y = FRAMEBUFFER_HEIGHT;
    if (start_x >= end_x || start_y >= end_y)
        return;

    for (int y = start_y; y < end_y; y++) {
        int dy = y - y_position;
        int sy = dy * sprite->height / size_y;

        for (int x = start_x; x < end_x; x++) {
            int dx = x - x_position;

            int sx = frame_index * sprite->frame_width
            + dx * sprite->frame_width / size_x;

            int si = (sy * sprite->width + sx) * 4;

            // alpha blend
            uint8_t a = sprite->pixels[si + 3];
            if (a == 0)
                continue;

            uint8_t r = sprite->pixels[si + 0];
            uint8_t g = sprite->pixels[si + 1];
            uint8_t b = sprite->pixels[si + 2];

            framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                pack_rgba(r, g, b, a);
        }
    }
}




#endif
