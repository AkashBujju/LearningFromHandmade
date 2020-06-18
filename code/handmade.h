#ifndef HANDMADE_H
#define HANDMADE_H

typedef struct GameOffScreenBuffer{
	void *memory;
	int width;
	int height;
	int pitch;
} GameOffScreenBuffer;

/* Services that the game provides to the platform layer */
static void game_update_and_render();

/* Services that the platform layer provides to the game */

#endif
