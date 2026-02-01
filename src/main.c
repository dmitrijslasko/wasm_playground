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
	GAME_OVER = -1,
	GAME_ACTIVE,
	GAME_PAUSED,
	GAME_WON
} e_game_status;

static t_texture player_tex;

static e_game_status game_state = GAME_PAUSED;

static uint8_t current_r = 255;
static uint8_t current_g = 0;
static uint8_t current_b = 255;
static uint8_t current_a = 255;

static int player_size = PLAYER_SIZE_STARTING;
static int player_size_step = 1;

static int player_pos_x = PLAYER_X_STARTING_POSITION;
static int player_pos_y = PLAYER_Y_GROUND_POSITION;

static int player_active_jump = 0;
static int player_active_jump_up = 0;
static float player_vel_y = 0.0f;

static char* obstacle_course = "102010010100010100010001000100101010010101001010010100000100100100100000100100101010101010101101010100010101010101000101000101010100010101";
static char* bonus_course =    "123410101221032011002100012000010103201001010010043101010001010000010014001020210100100010021103010200401010100010100010001000000101010043";

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

static float game_score = 0;
static int high_score = 0;
static float game_time = 0.0f;

static int bonus_collected = 0;
static int jump_triggered = 0;
static int game_over_triggered = 0;

static float speed_factor = 0.0f;
static float speed_factor_level_threshold = 0.0f;

// ------------------------------------------------------------

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
	player_pos_x = PLAYER_X_STARTING_POSITION;
	player_pos_y = PLAYER_Y_GROUND_POSITION;
	player_active_jump = 0;
	player_active_jump_up = 0;
	player_vel_y = 0.0f;
	obstacle_course_x = OBSTACLE_COURSE_STARTING_X;
	bonus_course_x = BONUS_COURSE_STARTING_X;

	
	game_score = 0;
	high_score = game_score;
	game_time = 0.0f;
	
	player_tex.current_frame = 0;
	
	bonus_collected = 0;
	jump_triggered = 0;
	game_over_triggered = 0;
	obstacle_course_movement_speed = OBSTACLE_COURSE_MOVEMENT_SPEED;
	bonus_course_movement_speed = BONUS_COURSE_MOVEMENT_SPEED;
	init_bonus_course_copy();

	speed_factor = 0.0f;
	game_state = GAME_ACTIVE;

	return 0;
}

void set_player_texture(uint8_t *data, int w, int h)
{
	player_tex.pixels = data;
	player_tex.width = w;
	player_tex.height = h;

	player_tex.frame_count = 9;
	player_tex.frame_width = w / player_tex.frame_count;
	player_tex.frame_height = h;

	player_tex.current_frame = 0;
	player_tex.frame_time = 0.0f;
	player_tex.frame_duration = 0.08f;
}

void update_player_animation(float dt)
{
	player_tex.frame_time += dt;

	if (player_tex.frame_time >= player_tex.frame_duration)
	{
		player_tex.frame_time = 0.0f;
		player_tex.current_frame =
			(player_tex.current_frame + 1) % player_tex.frame_count;
	}
}

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

// ------------------------------------------------------------
// ------------------------------------------------------------
// ------------------------------------------------------------

void update_obstacle_collisions(void) {
	float ox = obstacle_course_x;
	char *p = obstacle_course;

	while (*p && ox < FRAMEBUFFER_WIDTH) {
		if (*p >= '1') {
			if (aabb_overlap(player_pos_x + 120, player_pos_y - player_size, player_size - 220, player_size,
							 ox, OBSTACLE_Y_GROUND_POSITION - OBSTACLE_BASE_SIZE + 50,
							 OBSTACLE_BASE_SIZE, OBSTACLE_BASE_SIZE)) 
								{
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
		return;

	while (*p && ox < FRAMEBUFFER_WIDTH) {
		if (*p >= '1') {
			if (aabb_overlap(player_pos_x + 100, 
								player_pos_y - player_size, 
								player_size - 200, 
								player_size,
							 	ox, 
								get_bonus_y_position(p),
							 	BONUS_BASE_SIZE, 
								BONUS_BASE_SIZE)) {
				game_score += 1.0f;

				bonus_collected = 1;
				*p = '0';
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
				if (!DRAW_TRANSPARENCY)
				{
					if (a < 255)
						continue;
				}
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
		else if (*bonus_course_ptr >= '1')
				render_bonus(bonus_x_position, get_bonus_y_position(bonus_course_ptr));
		bonus_x_position += BONUS_COURSE_BASE_WIDTH;
		bonus_course_ptr++;
	}
}

void render_obstacle(int x_position, int y_ground)
{
	if (!obstacle_pixels || obstacle_w <= 0 || obstacle_h <= 0)
		return;

	// Convert ground Y → top-left Y
	int y_position = y_ground - OBSTACLE_BASE_SIZE;

	for (int y = 0; y < OBSTACLE_BASE_SIZE; y++) {
		int py = y_position + y;
		if (py < 0 || py >= FRAMEBUFFER_HEIGHT)
			continue;

		int sy = (y * obstacle_h) / OBSTACLE_BASE_SIZE;

		for (int x = 0; x < OBSTACLE_BASE_SIZE; x++) {
			int px = x_position + x;
			if (px < 0 || px >= FRAMEBUFFER_WIDTH)
				continue;

			int sx = (x * obstacle_w) / OBSTACLE_BASE_SIZE;
			int si = (sy * obstacle_w + sx) * 4;

			uint8_t r = obstacle_pixels[si + 0];
			uint8_t g = obstacle_pixels[si + 1];
			uint8_t b = obstacle_pixels[si + 2];
			uint8_t a = obstacle_pixels[si + 3];

			// Optional transparency handling
			if (!DRAW_TRANSPARENCY)
            {
			if (a == 0)
				continue;
			}

			framebuffer[py * FRAMEBUFFER_WIDTH + px] =
				pack_rgba(r, g, b, a);
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
				render_obstacle(obstacle_x_position, OBSTACLE_Y_GROUND_POSITION);
		}
		obstacle_x_position += OBSTACLE_COURSE_BASE_WIDTH;
		obstacle_course_ptr++;
	}
	if (obstacle_x_position < 0)
		game_state = GAME_WON;
}

void update_player_position(float dt)
{
	if (dt <= 0.0f)
		return;

	if (!player_active_jump) {
		player_pos_y = PLAYER_Y_GROUND_POSITION;
		player_vel_y = 0.0f;
		return;
	}

	player_vel_y += GRAVITY * dt;
	player_pos_y += (int)(player_vel_y * dt);

	// apex reached → allow fast fall
	if (player_vel_y > 0.0f)
		player_active_jump_up = 0;

	// landing
	if (player_pos_y >= PLAYER_Y_GROUND_POSITION) {
		player_pos_y = PLAYER_Y_GROUND_POSITION;
		player_vel_y = 0.0f;
		player_active_jump = 0;
		player_active_jump_up = 0;
	}
}

// void update_score(int value)
// {
//     if (obstacle_course_x + OBSTACLE_BASE_SIZE < player_pos_x)
//         game_score += value;
// }

void draw_player(int x, int y, int size)
{
	render_sprite_frame(&player_tex, player_size, player_size, player_pos_x, player_pos_y, player_tex.current_frame, framebuffer);
}

void update_speed_factor(float dt)
{
	if (game_state == GAME_ACTIVE) 
	{
		speed_factor = game_time / 15;
		if (speed_factor > MAX_GAME_SPEED_FACTOR)
			speed_factor = MAX_GAME_SPEED_FACTOR;
	}
}

EMSCRIPTEN_KEEPALIVE
void game_step(float dt)
{
	if (dt < 0.0f) {
		dt = 0.0f;
	}

	static int once = 0;
	if (!once) {
		once = 1;
		reset_game();
	}
	
	if (game_state == GAME_OVER)
		return ;

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
	
	update_player_animation(dt);

	if (game_state == GAME_ACTIVE)
		game_time += dt;

	update_speed_factor(dt);

	obstacle_course_movement_speed += speed_factor * dt;
	obstacle_course_x -= obstacle_course_movement_speed * dt;

	bonus_course_movement_speed += speed_factor * dt;
	bonus_course_x -= bonus_course_movement_speed * dt;
	
	update_player_position(dt);

	draw_sky1(framebuffer, dt);
	draw_sky2(framebuffer, dt);
	draw_sky3(framebuffer, dt);
	
	draw_ground(framebuffer, dt);
	
	// render the obstacle course 
	render_obstacle_course();
	if (game_state == GAME_ACTIVE)
	    update_obstacle_collisions();
	
	// render the bonus course
	render_bonus_course();
	if (game_state == GAME_ACTIVE)
		update_bonus_collisions();

	draw_player(0, 0, player_size);
	
	draw_ground2(framebuffer, dt);
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
	(void)dy; // explicitly unused

	if (game_state == GAME_OVER)
		return;

	player_pos_x += dx * 2;

	if (player_pos_x < 0)
		player_pos_x = 0;
	if (player_pos_x + player_size > FRAMEBUFFER_WIDTH)
		player_pos_x = FRAMEBUFFER_WIDTH - player_size;
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

EMSCRIPTEN_KEEPALIVE
int get_game_score(void) {
	return game_time + game_score;
}

EMSCRIPTEN_KEEPALIVE
int get_game_status(void) {
	return game_state;
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
