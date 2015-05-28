#ifndef MAIN_GAME_CODE_H
#define MAIN_GAME_CODE_H 1
#include <SDL2/SDL.h>
#include <SDL_mixer.h>
#include "../vmath.h"

#define arrayLength(array) (sizeof(array)/sizeof(*(array)))
#define internalEdgeNegativeFlag 1 << 0
#define internalEdgePosetiveFlag 1 << 1
struct Rect {
	v2 p;
	v2 size;
    Uint8 internalEdgeFlags[2];
};

struct RenderRect {
	Rect rect;
    Uint8 r, g, b;
    bool shown;
};

enum EntityType {
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_WALL,
};

struct Entity {
	v2 p;
	v2 velocity;
	float topSpeed;
	Rect collisionRect;
	EntityType type;
	bool inactive;
	Entity *nextFreeEntity;
    bool jumping;
    int jumpTimer;
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
	Uint64 entitiesTop;
	Entity *firstFreeEntity;
	Entity *player;
};
#endif
