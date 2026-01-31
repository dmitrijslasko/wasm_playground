#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <emscripten/emscripten.h>

#include "functions.h"
#include "settings.h"
#include "render_utils.h"


static uint32_t framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];

typedef enum {
    GAME_ACTIVE,
    GAME_PAUSED,
    GAME_OVER
} e_game_state;

static e_game_state game_state = GAME_PAUSED;

static uint8_t current_r = 255;
static uint8_t current_g = 0;
static uint8_t current_b = 255;
static uint8_t current_a = 255;

static int player_size = PLAYER_SIZE_STARTING;
static int player_size_step = 1;

static int player_pos_x = PLAYER_POS_X_STARTING_POSITION;
static int player_pos_y = PLAYER_Y_BASE_POSITION;

static int player_active_jump = 0;
static int player_active_jump_up = 0;
static float player_vel_y = 0.0f;

static char* obstacle_course = "10201001010001010001000100010010101001010100101001010000010010010010000010010010101010101010100101010101010101010100010101010101010001";
static char* bonus_course =    "111110102032011002100012000010103201001010010013101010001010000010014001010010100100010021103010200401010100010100010001000000101010011";

static char* bonus_course_copy = NULL;

static uint8_t *obstacle_pixels = NULL;
static int obstacle_w = 0;
static int obstacle_h = 0;
static float obstacle_course_x = OBSTACLE_COURSE_STARTING_X;
static float obstacle_course_movement_speed = OBSTACLE_COURSE_MOVEMENT_SPEED;

static uint8_t *bonus_pixels = NULL;
static int bonus_w = 0;
static int bonus_h = 0;
static float bonus_course_x = BONUS_COURSE_STARTING_X;
static float bonus_course_movement_speed = BONUS_COURSE_MOVEMENT_SPEED;

static int game_score = 0;
static int high_score = 0;
static float game_time = 0.0f;

static int bonus_collected = 0;
static int jump_triggered = 0;
static int game_over_triggered = 0;

static float speed_factor = 0.0f;


EMSCRIPTEN_KEEPALIVE
void set_obstacle_texture(uint8_t *pixels, int w, int h)
{
    obstacle_pixels = pixels;
    obstacle_w = w;
    obstacle_h = h;
}

EMSCRIPTEN_KEEPALIVE
void set_bonus_texture(uint8_t *pixels, int w, int h)
{
    bonus_pixels = pixels;
    bonus_w = w;
    bonus_h = h;
}

void init_bonus_course_copy(void)
{
    bonus_course_copy = strdup(bonus_course);
    if (!bonus_course_copy) {
        // handle error
    }
}

EMSCRIPTEN_KEEPALIVE
int reset_game(void)
{
    player_pos_x = PLAYER_POS_X_STARTING_POSITION;
    player_pos_y = PLAYER_Y_BASE_POSITION;
    player_active_jump = 0;
    player_active_jump_up = 0;
    player_vel_y = 0.0f;
    obstacle_course_x = OBSTACLE_COURSE_STARTING_X;
    bonus_course_x = BONUS_COURSE_STARTING_X;
    game_state = GAME_ACTIVE;

    game_score = 0;
    high_score = game_score;
    game_time = 0.0f;

    speed_factor = 0.0f;
    bonus_collected = 0;
    jump_triggered = 0;
    game_over_triggered = 0;
    obstacle_course_movement_speed = OBSTACLE_COURSE_MOVEMENT_SPEED;
    bonus_course_movement_speed = BONUS_COURSE_MOVEMENT_SPEED;
    init_bonus_course_copy();
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int get_game_score(void) {
    return game_time;
}

EMSCRIPTEN_KEEPALIVE
int get_game_over(void) {
    return game_state == GAME_OVER;
}

EMSCRIPTEN_KEEPALIVE
int get_bonus_collected(void) {
    int value = bonus_collected;
    bonus_collected = 0;
    return value;
}

EMSCRIPTEN_KEEPALIVE
int get_jump_triggered(void) {
    int value = jump_triggered;
    jump_triggered = 0;
    return value;
}

EMSCRIPTEN_KEEPALIVE
int get_game_over_triggered(void) {
    int value = game_over_triggered;
    game_over_triggered = 0;
    return value;
}

void update_obstacle_collisions(void) {
    float ox = obstacle_course_x;
    char *p = obstacle_course;

    while (*p && ox < FRAMEBUFFER_WIDTH) {
        if (*p >= '1') {
            if (aabb_overlap(player_pos_x, player_pos_y, player_size, player_size,
                             ox, OBSTACLE_Y_BASE_POSITION,
                             OBSTACLE_BASE_SIZE, OBSTACLE_BASE_SIZE)) {
                game_state = GAME_OVER;
                game_over_triggered = 1;
                }
            }
        ox += OBSTACLE_COURSE_BASE_WIDTH;
        p++;
    }
}

void update_bonus_collisions(void) {
    float ox = bonus_course_x;
    char *p = bonus_course_copy;

    if (!p)
        return; // allocation failed

    while (*p && ox < FRAMEBUFFER_WIDTH) {
        if (*p >= '1') {
            if (aabb_overlap(player_pos_x, player_pos_y, player_size, player_size,
                             ox, BONUS_Y_BASE_POSITION,
                             BONUS_BASE_SIZE, BONUS_BASE_SIZE)) {
                game_score += BONUS_BASE_VALUE;
                speed_factor = 0.0f;
                game_time += 1.0f;
                *p = '0';
                bonus_collected = 1;
                }
            }
        ox += BONUS_COURSE_BASE_WIDTH;
        p++;
    }
}

void    render_bonus(int x_position, int y_position)
{
    const int size = BONUS_BASE_SIZE;
    if (bonus_pixels && bonus_w > 0 && bonus_h > 0) {
        for (int y = 0; y < size; y++) {
            int py = y_position + y;
            if (py < 0 || py >= FRAMEBUFFER_HEIGHT) {
                continue;
            }
            int sy = (y * bonus_h) / size;
            for (int x = 0; x < size; x++) {
                int px = x_position + x;
                if (px < 0 || px >= FRAMEBUFFER_WIDTH) {
                    continue;
                }
                int sx = (x * bonus_w) / size;
                int si = (sy * bonus_w + sx) * 4;
                uint8_t r = bonus_pixels[si + 0];
                uint8_t g = bonus_pixels[si + 1];
                uint8_t b = bonus_pixels[si + 2];
                uint8_t a = bonus_pixels[si + 3];
                if (a == 0)
                     continue;
                framebuffer[py * FRAMEBUFFER_WIDTH + px] = pack_rgba(r, g, b, a);
            }
        }
    }
}

void    render_bonus_course(void)
{
    float bonus_x_position = bonus_course_x;
    // process the bonus course string and render the obstacles
    char *bonus_course_ptr = bonus_course_copy;
    while (*bonus_course_ptr != '\0' && bonus_x_position < FRAMEBUFFER_WIDTH)
    {
        if (bonus_x_position < -BONUS_BASE_SIZE)
            ;
        else if (*bonus_course_ptr >= '1') {
            if (*bonus_course_ptr == '1')
                render_bonus(bonus_x_position, 350);
            else if (*bonus_course_ptr == '2')
                render_bonus(bonus_x_position, 200);
            else if (*bonus_course_ptr == '3')
                render_bonus(bonus_x_position, 150);
            else if (*bonus_course_ptr == '4')
                render_bonus(bonus_x_position, 100);
            }
        bonus_x_position += BONUS_COURSE_BASE_WIDTH;
        bonus_course_ptr++;
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
    float obstacle_x_position = obstacle_course_x;
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

void update_player_position(float dt)
{
    if (dt <= 0.0f) {
        return;
    }

    if (!player_active_jump) {
        player_pos_y = PLAYER_Y_BASE_POSITION;
        player_vel_y = 0.0f;
        return;
    }

    player_vel_y += GRAVITY * dt;
    player_pos_y += (int)(player_vel_y * dt);
    if (player_pos_y >= PLAYER_Y_BASE_POSITION) {
        player_pos_y = PLAYER_Y_BASE_POSITION;
        player_vel_y = 0.0f;
        player_active_jump = 0;
    }
}
// void update_score(int value)
// {
//     if (obstacle_course_x + OBSTACLE_BASE_SIZE < player_pos_x)
//         game_score += value;
// }

EMSCRIPTEN_KEEPALIVE
void game_step(float dt)
{
    static int once = 0;
    if (!once) {
        once = 1;
        reset_game();
        // puts("HELLO FROM C");
        // init_bonus_course_copy();
    }
    
    if (game_state == GAME_OVER)
        return ;

    if (obstacle_course_x + OBSTACLE_BASE_SIZE < player_pos_x)
        game_score += 1 * (int)(speed_factor + 1.0f);
    
    game_time += dt;

    if (dt < 0.0f) {
        dt = 0.0f;
    }

    if (obstacle_course_x < 0) {
        speed_factor = (float)(fabs(obstacle_course_x) / FRAMEBUFFER_WIDTH);
    }
    if (speed_factor > 6.0f)
        speed_factor = 6.0f;

    obstacle_course_movement_speed += speed_factor * dt;
    obstacle_course_x -= obstacle_course_movement_speed * dt;

    bonus_course_movement_speed += speed_factor * dt;
    bonus_course_x -= bonus_course_movement_speed * dt;

    // clear the framebuffer
    // if (!cleared) {
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
            framebuffer[y * FRAMEBUFFER_WIDTH + x] =
                pack_rgba(0, 0, 0, 255);
        }
    }
    //     cleared = 1;
    // }
    
    // int x0 = player_pos_x;
    // int y0 = player_pos_y;
    // if (x0 < 0) x0 = 0;=
    // if (y0 < 0) y0 = 0;
    // if (x0 + size > FRAMEBUFFER_WIDTH) x0 = FRAMEBUFFER_WIDTH - size;
    // if (y0 + size > FRAMEBUFFER_HEIGHT) y0 = FRAMEBUFFER_HEIGHT - size;
    // uint32_t color =
    //     (uint32_t)(current_r | (current_g << 8) | (current_b << 16) | (current_a << 24));

    update_player_position(dt);

    draw_sky1(framebuffer, dt);
    draw_sky2(framebuffer, dt);
    draw_ground(framebuffer, dt);
       
    render_obstacle_course();
    update_obstacle_collisions();
    
    render_bonus_course();
    update_bonus_collisions();
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
    if (game_state == GAME_OVER || player_active_jump)
        return;
    player_active_jump = 1;
    player_active_jump_up = 1;
    player_vel_y = -JUMP_VELOCITY;
    jump_triggered = 1;
}

EMSCRIPTEN_KEEPALIVE
void player_down(void)
{
    if (game_state == GAME_OVER)
        return;
    if (!player_active_jump)
        return;
    if (player_active_jump_up)
        return;
    if (player_vel_y <= 0.0f)
        return;
    if (player_vel_y < FAST_FALL_VELOCITY) {
        player_vel_y = FAST_FALL_VELOCITY;
    }
}


EMSCRIPTEN_KEEPALIVE
void move_rect(int dx, int dy)
{
    if (game_state == GAME_OVER)
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

EMSCRIPTEN_KEEPALIVE
float get_speed(void)
{
    return speed_factor;
}