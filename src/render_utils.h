#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <math.h>
#include <stdint.h>
#include "settings.h"

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


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

// static inline int pixel_perfect_collision(
//     uint32_t *a_pixels, int aw, int ah, int ax, int ay,
//     uint32_t *b_pixels, int bw, int bh, int bx, int by
// ) {
//     if (!aabb_overlap(ax, ay, aw, ah, bx, by, bw, bh))
//         return 0;
//     return 1;

//     int ix = MAX(ax, bx);
//     int iy = MAX(ay, by);
//     int iw = MIN(ax + aw, bx + bw) - ix;
//     int ih = MIN(ay + ah, by + bh) - iy;

//     for (int y = 0; y < ih; y++) {
//         for (int x = 0; x < iw; x++) {

//             int axp = ix - ax + x;
//             int ayp = iy - ay + y;
//             int bxp = ix - bx + x;
//             int byp = iy - by + y;

//             uint32_t ap = a_pixels[ayp * aw + axp];
//             uint32_t bp = b_pixels[byp * bw + bxp];

//             uint8_t a_alpha = ap >> 24;
//             uint8_t b_alpha = bp >> 24;

//             if (a_alpha > 0 && b_alpha > 0)
//                 return 1; // REAL collision
//         }
//     }
//     return 0;
// }

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

            if (a < 255) {
                continue;
            }

            if (r == 0 && g == 0   && b == 0)
                continue;

            if (r == 255 && g == 255 && b == 255)
                continue;

            framebuffer[y * FRAMEBUFFER_WIDTH + x] = pack_rgba(r, g, b, a);
        }
    }
}

static inline void render_sprite_frame(
    t_texture *sprite,
    int size_x,
    int size_y,
    int x_position,
    int y_ground,
    int frame_index,
    uint32_t *framebuffer
) {
    if (!sprite->pixels || sprite->width <= 0 || sprite->height <= 0)
        return;

    // Convert ground Y → top-left Y
    int y_position = y_ground - size_y;

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

            int sx =
                frame_index * sprite->frame_width
                + dx * sprite->frame_width / size_x;

            int si = (sy * sprite->width + sx) * 4;

            uint8_t r = sprite->pixels[si + 0];
            uint8_t g = sprite->pixels[si + 1];
            uint8_t b = sprite->pixels[si + 2];
            uint8_t a = sprite->pixels[si + 3];

            // transparency rules
            if (!DRAW_TRANSPARENCY)
            {
            if (a < 255)
                continue;
            if (r == 0   && g == 0   && b == 0)
                continue;
            if (r == 255 && g == 255 && b == 255)
                continue;
            }

            framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                pack_rgba(r, g, b, a);
        }
    }
}



#endif
