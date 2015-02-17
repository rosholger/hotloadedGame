#ifndef MAIN_GAME_CODE_H
#define MAIN_GAME_CODE_H 1
#include <SDL2/SDL.h>
#include <SDL_mixer.h>
#include "../vmath.h"
struct RenderRect {
    float x, y, h, w;
    Uint8 r, g, b;
    bool shown;
};
struct Entity {
	v2 p;
	v2 velocity;
	float topSpeed;
};
#define TILE_MAP_SIZE_X 25
#define TILE_MAP_SIZE_Y 15

struct GameState {
    bool isInitialised;
    char executablePath[500];
    int onePastLastSlash;
    SDL_Renderer *renderer;
    const Uint8 lastKeyboardState[512];
    SDL_Joystick *joystick;
    float t;
    Mix_Music *music;
    RenderRect renderRects[200];
    Uint16 renderRectsTop;
    char tileMap[375];
	Entity entities[256];	
};
#endif
