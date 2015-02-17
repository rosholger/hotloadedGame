#include <stdio.h>
#include <string.h>
#include <unistd.h>
// #include <SDL2/SDL_audio.h>
#include <SDL_mixer.h>
#include <math.h>
#include "mainGameCode.h"
#define PI 3.14159265
#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)

float getJoystickAxis(SDL_Joystick *joy, int axisIndex) {
    Sint16 axis = SDL_JoystickGetAxis(joy, axisIndex);
    float faxis = (((float)axis)+1)/32768;
    if (fabs(faxis) < 0.08) {
        faxis = 0;
    }
    return faxis;
}

void pushRenderRect(GameState *state, RenderRect rect) {
    if (state->renderRectsTop > sizeof(state->renderRects)/sizeof(RenderRect)) {
        printf("PANICK\n");
        _exit(-1);
    }
    state->renderRects[state->renderRectsTop] = rect;
    ++state->renderRectsTop;
    

}

extern "C" 
bool updateAndRenderGame(void *permStorage) {
    GameState *state = (GameState *)permStorage;
    if (!state->isInitialised) {
        if (SDL_NumJoysticks() > 0) {
            state->joystick = SDL_JoystickOpen(0);
            if (state->joystick) {
                printf("Opened Joystick 0\n");
                printf("Name: %s\n", SDL_JoystickNameForIndex(0));
                printf("Number of Axes: %d\n", SDL_JoystickNumAxes(state->joystick));
                printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(state->joystick));
                printf("Number of Balls: %d\n", SDL_JoystickNumBalls(state->joystick));
                printf("Number of Hats: %d\n", SDL_JoystickNumHats(state->joystick));
            } else {
                printf("Couldn't open Joystick 0\n");
            }
            SDL_JoystickEventState(0);
            SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
            SDL_EventState(SDL_KEYUP, SDL_IGNORE);

        }
        printf("GameState size %fMB\n", ((float)sizeof(GameState))/Megabytes(1));

        state->music = Mix_LoadMUS("test.wav");
        if (!state->music) {
            printf("load music error: %s\n", Mix_GetError());
            return true;
        }
        char const *temp = 
"\
1111111111111111111111111\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1000000000000000000000001\
1111111111111111111111111\
";

        memcpy(state->tileMap, temp, 375); 
		Entity e = {};
		e.p = {10, 10};
		e.topSpeed = 1;
		state->entities[0] = e;

        state->isInitialised = true;  
    }
    state->entities[0].topSpeed = 0.5;
	float accVal = 0.5;
	v2 acc = {0, 0};
    {
        SDL_JoystickUpdate();
        if (getJoystickAxis(state->joystick, 0) != 0 || getJoystickAxis(state->joystick, 1) != 0) {
            acc.x += accVal*getJoystickAxis(state->joystick, 0);
            acc.y += accVal*getJoystickAxis(state->joystick, 1);
        } else {
            Uint8 hatState = SDL_JoystickGetHat(state->joystick, 0);
            if (hatState & SDL_HAT_UP) {
                acc.y -= accVal;
            }
    
            if (hatState & SDL_HAT_DOWN) {
                acc.y += accVal;
            }
            if (hatState & SDL_HAT_LEFT) {
                acc.x -= accVal;
            }
            if (hatState & SDL_HAT_RIGHT) {
                acc.x += accVal;
            }
        }
    }

	{
        SDL_PumpEvents();
        int numKeys;
        const Uint8 *keyboardState = SDL_GetKeyboardState(&numKeys);
        if (keyboardState[SDL_SCANCODE_ESCAPE] && !state->lastKeyboardState[SDL_SCANCODE_ESCAPE]) {
        }    
        if (keyboardState[SDL_SCANCODE_UP]) {
            acc.y -= accVal;
        }
        if (keyboardState[SDL_SCANCODE_DOWN]) {
            acc.y += accVal;
        }
        if (keyboardState[SDL_SCANCODE_LEFT]) {
            acc.x -= accVal;
        }
        if (keyboardState[SDL_SCANCODE_RIGHT]) {
            acc.x += accVal;
        }



        memcpy((void *)state->lastKeyboardState, keyboardState, numKeys*sizeof(Uint8));
	}
	// NOTE(holger): acc should get normalized when using digital input only
	// or if this is a platformer not at all!
	// acc = v2normalize(acc);
	state->entities[0].velocity += acc;
	float damp = state->entities[0].topSpeed/(state->entities[0].topSpeed+accVal);
	state->entities[0].velocity *= damp;
	state->entities[0].p += state->entities[0].velocity;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_WINDOWEVENT : {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                    return true;
                }
            } break;
        }
    }
    state->t += 0.001;
    RenderRect rect = {};
    rect.x = (state->entities[0].p.x*6);
    rect.y = (state->entities[0].p.y*6);
    rect.w = 20;
    rect.h = 20;
    rect.r = 127 + 100*sin(state->t);
    rect.g = 127 + 100*sin(state->t*2.0);
    rect.b = 127 + 100*sin(state->t*4.0);
    rect.shown = true;
    pushRenderRect(state, rect);


    int tileSize = 25;
    for (int y = 0; y < TILE_MAP_SIZE_Y; ++y) {
        for (int x = 0; x < TILE_MAP_SIZE_X; ++x) {
            if (state->tileMap[x+y*TILE_MAP_SIZE_X] == '1') {
                RenderRect tileRect = {};
                tileRect.x = x * (tileSize);
                tileRect.y = y * (tileSize);
                tileRect.w = tileSize;
                tileRect.h = tileSize;
                tileRect.r = 255;
                tileRect.g = 255;
                tileRect.b = 255;
                tileRect.shown = true;
                pushRenderRect(state, tileRect);
            }
        }
    }
    //state->renderRects[0] = rect;
    // SDL_Rect testBox = {};
    // testBox.x = (state->x);
    // testBox.y = (state->y);
    // testBox.w = 20;
    // testBox.h = 20;
    // SDL_SetRenderDrawColor(state->renderer, 127 + 100*sin(state->t), 127 + 100*sin(state->t*2.0), 127 + 100*sin(state->t*4.0), 255);
    // SDL_RenderFillRect(state->renderer, &testBox);
    // SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    return false;
}
