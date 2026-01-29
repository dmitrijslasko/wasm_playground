#include <stdint.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

#define FRAMEBUFFER_WIDTH 960
#define FRAMEBUFFER_HEIGHT 600

#define PLAYER_POS_X_STARTING_POSITION 400

#define PLAYER_POS_Y_STARTING_POSITION 420
#define PLAYER_JUMP_HEIGHT 250

#define GROUND_Y 500


#define OBSTACLE_BASE_SIZE 100

#define OBSTACLE_Y_BASE_POSITION 420
#define OBSTACLE_COURSE_BASE_WIDTH 150
#define OBSTACLE_COURSE_STARTING_X 1520
#define OBSTACLE_COURSE_MOVEMENT_SPEED 3


static uint32_t framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

static int game_over = 0;

static uint8_t current_r = 255;
static uint8_t current_g = 0;
static uint8_t current_b = 255;
static uint8_t current_a = 255;

static int player_size = 100;
static int player_size_step = 1;

static int player_pos_x = PLAYER_POS_X_STARTING_POSITION;
static int player_pos_y = PLAYER_POS_Y_STARTING_POSITION;

static int player_active_jump = 0;
static int player_active_jump_up = 0;

static char* obstacle_course = "10010100010100010001000100010010010000010010010101010101010100101010101010101010100010101010101010001";
static int obstacle_course_starting_x = OBSTACLE_COURSE_STARTING_X;

static uint8_t *sky_pixels = NULL;
static int sky_w = 0;
static int sky_h = 0;

static uint8_t *ground_pixels = NULL;
static int ground_w = 0;
static int ground_h = 0;

static uint8_t *obstacle_pixels = NULL;
static int obstacle_w = 0;
static int obstacle_h = 0;

static int game_score = 0;

EMSCRIPTEN_KEEPALIVE
void set_sky_texture(uint8_t *pixels, int w, int h)
{
    sky_pixels = pixels;
    sky_w = w;
    sky_h = h;
}

EMSCRIPTEN_KEEPALIVE
void set_ground_texture(uint8_t *pixels, int w, int h)
{
    ground_pixels = pixels;
    ground_w = w;
    ground_h = h;
}

EMSCRIPTEN_KEEPALIVE
void set_obstacle_texture(uint8_t *pixels, int w, int h)
{
    obstacle_pixels = pixels;
    obstacle_w = w;
    obstacle_h = h;
}

static inline uint32_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t)(r | (g << 8) | (b << 16) | (a << 24));
}

static void draw_sky(void)
{
    if (!sky_pixels || sky_w <= 0 || sky_h <= 0) {
        return;
    }

    for (int y = 0; y < GROUND_Y; y++) {
        int sy = (y * sky_h) / GROUND_Y;
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

static void draw_ground(void)
{
    if (!ground_pixels || ground_w <= 0 || ground_h <= 0) {
        for (int y = GROUND_Y; y < FRAMEBUFFER_HEIGHT; y++) {
            for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                    pack_rgba(0, 255, 0, 255);
            }
        }
        return;
    }

    const int ground_height = FRAMEBUFFER_HEIGHT - GROUND_Y;
    for (int y = GROUND_Y; y < FRAMEBUFFER_HEIGHT; y++) {
        int sy = ((y - GROUND_Y) * ground_h) / ground_height;
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

EMSCRIPTEN_KEEPALIVE
int reset_game(void)
{
    player_pos_x = PLAYER_POS_X_STARTING_POSITION;
    player_pos_y = PLAYER_POS_Y_STARTING_POSITION;
    player_active_jump = 0;
    player_active_jump_up = 0;
    obstacle_course_starting_x = OBSTACLE_COURSE_STARTING_X;
    game_over = 0;
    game_score = 0;
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_game_score(void)
{
    return game_score;
}

int    check_collision(int obstacle_x_pos, int obstacle_y_pos)
{
    if (player_pos_x == obstacle_x_pos && player_pos_y == obstacle_y_pos)
        return 1;
    return 0;
}

int aabb_overlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

void update_collision(void) {
    int ox = obstacle_course_starting_x;
    char *p = obstacle_course;

    while (*p && ox < FRAMEBUFFER_WIDTH) {
        if (*p >= '1') {
            if (aabb_overlap(player_pos_x, player_pos_y, player_size, player_size,
                             ox, OBSTACLE_Y_BASE_POSITION,
                             OBSTACLE_BASE_SIZE, OBSTACLE_BASE_SIZE)) {
                game_over = 1;
            }
            if (player_pos_x > ox + OBSTACLE_BASE_SIZE)
                game_score++;
            }
        ox += OBSTACLE_COURSE_BASE_WIDTH;
        p++;
    }
}


void    render_obstacle(int x_position, int y_position)
{
    const int size = OBSTACLE_BASE_SIZE;
    if (obstacle_pixels && obstacle_w > 0 && obstacle_h > 0) {
        for (int y = 0; y < size; y++) {
            int py = y_position + y;
            if (py < 0 || py >= FRAMEBUFFER_HEIGHT) {
                continue;
            }
            int sy = (y * obstacle_h) / size;
            for (int x = 0; x < size; x++) {
                int px = x_position + x;
                if (px < 0 || px >= FRAMEBUFFER_WIDTH) {
                    continue;
                }
                int sx = (x * obstacle_w) / size;
                int si = (sy * obstacle_w + sx) * 4;
                uint8_t r = obstacle_pixels[si + 0];
                uint8_t g = obstacle_pixels[si + 1];
                uint8_t b = obstacle_pixels[si + 2];
                uint8_t a = obstacle_pixels[si + 3];
                framebuffer[py * FRAMEBUFFER_WIDTH + px] = pack_rgba(r, g, b, a);
            }
        }
        return;
    }

    for (int y = y_position; y < y_position + size; y++) {
        if (y < 0 || y >= FRAMEBUFFER_HEIGHT) {
            continue;
        }
        for (int x = x_position; x < x_position + size; x++) {
            if (x < 0 || x >= FRAMEBUFFER_WIDTH) {
                continue;
            }
            framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                pack_rgba(255, 0, 0, 255);
        }
    }
}
void    render_obstacle_course(void)
{
    int obstacle_x_position = obstacle_course_starting_x;
    // process the obstacle course string and render the obstacles
    char *obstacle_course_ptr = obstacle_course;
    
    while (*obstacle_course_ptr != '\0' && obstacle_x_position < FRAMEBUFFER_WIDTH)
    {
        if (*obstacle_course_ptr >= '1')
        {
            if (obstacle_x_position < -OBSTACLE_BASE_SIZE)
                ;
            else
                render_obstacle(obstacle_x_position, OBSTACLE_Y_BASE_POSITION);
        }
        obstacle_x_position += OBSTACLE_COURSE_BASE_WIDTH;
        obstacle_course_ptr++;
    }
}

void render_player(void)
{
    if (player_active_jump)
    {
        if (player_active_jump_up)
        {
            player_pos_y -= 5;
            if (player_pos_y < PLAYER_POS_Y_STARTING_POSITION - PLAYER_JUMP_HEIGHT)
                player_active_jump_up = 0;
        }
        else
        {
            player_pos_y += 7;
            if (player_pos_y >= PLAYER_POS_Y_STARTING_POSITION)
            {
                player_pos_y = PLAYER_POS_Y_STARTING_POSITION;
                player_active_jump = 0;
            }
        }
    }
    else 
    {
        player_pos_y = PLAYER_POS_Y_STARTING_POSITION;
    }
}


EMSCRIPTEN_KEEPALIVE
void game_step(float dt)
{
    static int once = 0;
    // static int cleared = 0;

    if (!once) {
        once = 1;
        puts("HELLO FROM C");
    }
    
    if (game_over)
        return ;

    obstacle_course_starting_x -= OBSTACLE_COURSE_MOVEMENT_SPEED;
    (void)dt;

    // if (!cleared) {
        for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
            for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                    (uint32_t)(0 | (0 << 8) | (0 << 16) | (255 << 24));
            }
        }
    //     cleared = 1;
    // }

    int max_size = (FRAMEBUFFER_WIDTH < FRAMEBUFFER_HEIGHT) ? FRAMEBUFFER_WIDTH : FRAMEBUFFER_HEIGHT;
    int size = player_size;
    if (size > max_size) {
        size = max_size;
    }
    
    int x0 = player_pos_x;
    int y0 = player_pos_y;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 + size > FRAMEBUFFER_WIDTH) x0 = FRAMEBUFFER_WIDTH - size;
    if (y0 + size > FRAMEBUFFER_HEIGHT) y0 = FRAMEBUFFER_HEIGHT - size;
    uint32_t color =
        (uint32_t)(current_r | (current_g << 8) | (current_b << 16) | (current_a << 24));

    draw_sky();

    draw_ground();
       
    render_obstacle_course();
    render_player();
    update_collision();
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
    int max_size = (FRAMEBUFFER_WIDTH < FRAMEBUFFER_HEIGHT) ? FRAMEBUFFER_WIDTH : FRAMEBUFFER_HEIGHT;
    player_size += player_size_step;
    if (player_size > max_size) {
        player_size = player_size_step;
    }
}

EMSCRIPTEN_KEEPALIVE
void shrink_rect(void)
{
    player_size -= player_size_step;
    if (player_size < player_size_step) {
        player_size = player_size_step;
    }
}
EMSCRIPTEN_KEEPALIVE
void player_up(void)
{
    if (player_active_jump)
        return;
    player_active_jump = 1;
    player_active_jump_up = 1;
}

EMSCRIPTEN_KEEPALIVE
void player_down(void)
{
    player_active_jump = 0;
}


EMSCRIPTEN_KEEPALIVE
void move_rect(int dx, int dy)
{
    if (game_over)
        return;
        
    player_pos_x += dx;
    player_pos_y += dy;

    int max_size = (FRAMEBUFFER_WIDTH < FRAMEBUFFER_HEIGHT) ? FRAMEBUFFER_WIDTH : FRAMEBUFFER_HEIGHT;
    int size = player_size;
    if (size > max_size) {
        size = max_size;
    }

    if (player_pos_x < 0) player_pos_x = 0;
    if (player_pos_y < 0) player_pos_y = 0;
    if (player_pos_x + size > FRAMEBUFFER_WIDTH) player_pos_x = FRAMEBUFFER_WIDTH - size;
    if (player_pos_y + size > FRAMEBUFFER_HEIGHT) player_pos_y = FRAMEBUFFER_HEIGHT - size;
}

EMSCRIPTEN_KEEPALIVE
int get_player_x(void)
{
    return player_pos_x;
}

EMSCRIPTEN_KEEPALIVE
int get_player_y(void)
{
    return player_pos_y;
}

EMSCRIPTEN_KEEPALIVE
int get_player_size(void)
{
    return player_size;
}
