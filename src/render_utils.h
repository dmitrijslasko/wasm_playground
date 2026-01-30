#ifndef RENDER_UTILS_H
#define RENDER_UTILS_H

#include <stdint.h>

static inline uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t)(r | (g << 8) | (b << 16) | (a << 24));
}

static inline int aabb_overlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

#endif
