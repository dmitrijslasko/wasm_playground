#include <stdint.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

#define WIDTH 320
#define HEIGHT 200

static uint32_t framebuffer[WIDTH * HEIGHT];
static uint8_t current_r = 255;
static uint8_t current_g = 0;
static uint8_t current_b = 255;
static uint8_t current_a = 255;
static int rect_size = 20;
static int rect_step = 1;
static int rect_x = (WIDTH - 20) / 2;
static int rect_y = (HEIGHT - 20) / 2;

EMSCRIPTEN_KEEPALIVE
void game_step(float dt)
{
    static int once = 0;

    if (!once) {
        once = 1;
        puts("HELLO FROM C");
    }

    (void)dt;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            framebuffer[y * WIDTH + x] =
                (uint32_t)(0 | (0 << 8) | (0 << 16) | (255 << 24));
        }
    }

    int max_size = (WIDTH < HEIGHT) ? WIDTH : HEIGHT;
    int size = rect_size;
    if (size > max_size) {
        size = max_size;
    }
    int x0 = rect_x;
    int y0 = rect_y;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 + size > WIDTH) x0 = WIDTH - size;
    if (y0 + size > HEIGHT) y0 = HEIGHT - size;
    uint32_t color =
        (uint32_t)(current_r | (current_g << 8) | (current_b << 16) | (current_a << 24));

    for (int y = y0; y < y0 + size; y++) {
        for (int x = x0; x < x0 + size; x++) {
            framebuffer[y * WIDTH + x] = color;
        }
    }
}

EMSCRIPTEN_KEEPALIVE
uint8_t *get_framebuffer(void)
{
    return (uint8_t *)framebuffer;
}

EMSCRIPTEN_KEEPALIVE
void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    current_r = r;
    current_g = g;
    current_b = b;
    current_a = a;
}

EMSCRIPTEN_KEEPALIVE
void grow_rect(void)
{
    int max_size = (WIDTH < HEIGHT) ? WIDTH : HEIGHT;
    rect_size += rect_step;
    if (rect_size > max_size) {
        rect_size = rect_step;
    }
}

EMSCRIPTEN_KEEPALIVE
void shrink_rect(void)
{
    rect_size -= rect_step;
    if (rect_size < rect_step) {
        rect_size = rect_step;
    }
}

EMSCRIPTEN_KEEPALIVE
void move_rect(int dx, int dy)
{
    rect_x += dx;
    rect_y += dy;

    int max_size = (WIDTH < HEIGHT) ? WIDTH : HEIGHT;
    int size = rect_size;
    if (size > max_size) {
        size = max_size;
    }

    if (rect_x < 0) rect_x = 0;
    if (rect_y < 0) rect_y = 0;
    if (rect_x + size > WIDTH) rect_x = WIDTH - size;
    if (rect_y + size > HEIGHT) rect_y = HEIGHT - size;
}
