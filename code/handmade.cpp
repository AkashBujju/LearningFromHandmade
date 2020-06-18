#include "handmade.h"

static void render_something(GameOffScreenBuffer *buffer, int x, int y);

static void game_update_and_render(GameOffScreenBuffer *buffer) {
	int blue_offset = 0;
	int green_offset = 0;
	render_something(buffer, blue_offset, green_offset);
}

static void render_something(GameOffScreenBuffer *buffer, int x, int y) {
	int width = buffer->width;
	int height = buffer->height;

	// Register: xx RR GG BB 
	// Memory  : BB GG RR xx
	uint8* row = (uint8*) buffer->memory;
	for(int i = 0; i < height; ++i) {
		uint32* pixel = (uint32*) row;
		for(int j = 0; j < width; ++j) {
			uint8 green = j + x;
			uint8 blue = i + y;
			*pixel = ((green << 16) | (blue << 8)) | blue;
			pixel += 1;
		}

		row += buffer->pitch;
	}
}
