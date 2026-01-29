#include <stdint.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

#define FRAMEBUFFER_WIDTH 960
#define FRAMEBUFFER_HEIGHT 600

#define PLAYER_POS_X_STARTING_POSITION 400

#define PLAYER_POS_Y_STARTING_POSITION 420
#define PLAYER_JUMP_HEIGHT 150


#define OBSTACLE_BASE_SIZE 50
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

static int player_size = 50;
static int player_size_step = 1;

static int player_pos_x = PLAYER_POS_X_STARTING_POSITION;
static int player_pos_y = PLAYER_POS_Y_STARTING_POSITION;

static int player_active_jump = 0;
static int player_active_jump_up = 0;

static char* obstacle_course = "10010100010100010001000100010010010000010010010101010101010100101010101010101010100010101010101010001";
static int obstacle_course_starting_x = OBSTACLE_COURSE_STARTING_X;

EMSCRIPTEN_KEEPALIVE
int reset_game(void)
{
    player_pos_x = PLAYER_POS_X_STARTING_POSITION;
    player_pos_y = PLAYER_POS_Y_STARTING_POSITION;
    player_active_jump = 0;
    player_active_jump_up = 0;
    obstacle_course_starting_x = OBSTACLE_COURSE_STARTING_X;
    game_over = 0;
    return 0;
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
        }
        ox += OBSTACLE_COURSE_BASE_WIDTH;
        p++;
    }
}


void    render_obstacle(int x_position, int y_position)
{
    for (int y = y_position; y < y_position + OBSTACLE_BASE_SIZE; y++) 
    {
        for (int x = x_position; x < x_position + OBSTACLE_BASE_SIZE; x++)
        {
            if (x < 0) 
                continue;
            framebuffer[y * FRAMEBUFFER_WIDTH + x] = (uint32_t)(255 | (0 << 8) | (0 << 16) | (255 << 24));
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

    for (int y = player_pos_y; y < player_pos_y + player_size; y++) {
        for (int x = player_pos_x; x < player_pos_x + player_size; x++) {
            framebuffer[y * FRAMEBUFFER_WIDTH + x] =  (uint32_t)(0 | (0 << 8) | (0 << 16) | (255 << 24));;
        }
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




    // draw sky 
    for (int y = 0; y < FRAMEBUFFER_HEIGHT; y++) {
        for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
                framebuffer[y * FRAMEBUFFER_WIDTH + x] =  (uint32_t)(0 | (0 << 8) | (255 << 16) | (255 << 24));
            }
        }

    // draw ground
    for (int y = 400; y < FRAMEBUFFER_HEIGHT; y++) {
            for (int x = 0; x < FRAMEBUFFER_WIDTH; x++) {
                    framebuffer[y * FRAMEBUFFER_WIDTH + x] =  (uint32_t)(0 | (255 << 8) | (0 << 16) | (255 << 24));;
                }
            }
       
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
