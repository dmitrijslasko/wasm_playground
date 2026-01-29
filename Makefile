all:
	emcc main.c \
	-O2 \
	-s WASM=1 \
	-s EXPORTED_FUNCTIONS='["_game_step", \
	"_reset_game", "_get_framebuffer", "_set_color", "_grow_rect", "_shrink_rect", "_move_rect", "_reset_game"]' \
	-s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
	-o demo.js