all:
	emcc main.c \
	-O2 \
	-s WASM=1 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s EXPORTED_FUNCTIONS='["_malloc", "_free", "_set_sky_texture", "_set_ground_texture", "_set_obstacle_texture", "_game_step", \
	"_reset_game", "_get_game_score", "_get_framebuffer", "_set_color", "_grow_rect", "_shrink_rect", "_move_rect", \
	"_get_player_x", "_get_player_y", "_get_player_size"]' \
	-s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
	-o demo.js