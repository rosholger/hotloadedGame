#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "mainGameCode.h"

#define PI 3.14159265
extern "C" bool updateAndRenderGame(void *permStorage) {
    GameState *state = (GameState *)permStorage;
    state->t += 0.016;
    SDL_Rect testBox = {};
    testBox.x = (80.0 + 15.0*cos(state->t/2.0) + 20.0*cos(state->t*20.0) + 0.5);
    testBox.y = (80.0 + 15.0*sin(state->t/2.0) + 20.0*sin(state->t*20.0) + 0.5);
    testBox.w = 20;
    testBox.h = 20;
    SDL_SetRenderDrawColor(state->renderer, 127 + 100*sin(state->t), 127 + 100*sin(state->t*2.0), 127 + 100*sin(state->t*4.0), 255);
    SDL_RenderFillRect(state->renderer, &testBox);
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    return false;
}