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
#define TILESIZE (25)

float getJoystickAxis(SDL_Joystick *joy, int axisIndex) {
    Sint16 axis = SDL_JoystickGetAxis(joy, axisIndex);
    float faxis = (((float)axis)+1)/32768;
    if (fabs(faxis) < 0.08) {
        faxis = 0;
    }
    return faxis;
}

void pushRenderRect(GameState *state, RenderRect rect) {
    if (state->renderRectsTop > 
            sizeof(state->renderRects)/sizeof(RenderRect)) {
        printf("PANICK\n");
        _exit(-1);
    }
    state->renderRects[state->renderRectsTop] = rect;
    ++state->renderRectsTop;
}

Entity *newEntity(GameState *gameState) {
    Entity *ret = 0;
    if (gameState->firstFreeEntity) {
        ret = gameState->firstFreeEntity;
        gameState->firstFreeEntity = ret->nextFreeEntity;
        ret->nextFreeEntity = 0;
        ret->inactive = 0;
    } else {
        SDL_assert(gameState->entitiesTop < arrayLength(
                    gameState->entities));
        ret = gameState->entities + gameState->entitiesTop;
        ++gameState->entitiesTop;
    }
    return ret;
}

// NOTE(Holger): This v2 is a 1D projection where x is min and y max.
v2 projectEntityOnAxis(v2 axis, Rect r) {
    v2 vertices[4];
    vertices[0] = r.p;
    vertices[1] = vertices[0] + v2{r.size.x, 0};
    vertices[2] = vertices[0] + r.size;
    vertices[3] = vertices[0] + v2{0, r.size.y};
    v2 projection;
    projection.x = v2dot(axis, vertices[0]);
    projection.y = projection.x;
    for(int i = 1; i < arrayLength(vertices); ++i) {
        float p = v2dot(axis, vertices[i]);
        projection.x = p < projection.x ? p : projection.x;
        projection.y = p > projection.y ? p : projection.y;
    }
    return projection;
}

bool projectionOverlap(v2 proj1, v2 proj2) {
    bool overlaps = true;
    if((proj1.y < proj2.x) || (proj2.y < proj1.x)) {
        overlaps = false;
    }
    return overlaps;
}

float getProjectionOverlap(v2 proj1, v2 proj2) {
    float case1 = proj1.y - proj2.x;
    float case2 = proj2.y - proj1.x;
    float smallest = -case1;
    if ((proj1.x + proj1.y)/2 < (proj2.x + proj2.y)/2) {
        smallest = -case1;
    } else {
        smallest = case2;
    }
    return smallest;
}

bool entitiesIntersect(Rect a, Rect b, v2 *mtv) {
    v2 axes[2];
    axes[0] = v2{1, 0};
    axes[1] = v2{0, 1};
    Uint8 internalAxisFlags[2];
    internalAxisFlags[0] = a.internalEdgeFlags[0] | b.internalEdgeFlags[0];
    internalAxisFlags[1] = a.internalEdgeFlags[1] | b.internalEdgeFlags[1];
    float overlap = 1.0 / 0.0; //NOTE(Holger): INFINITYYYY!!!
    v2 smallestAxis = v2{0, 0};
    for(int i = 0; i < arrayLength(axes); ++i) {
        v2 axis = axes[i];
        v2 proj1 = projectEntityOnAxis(axis, a);
        v2 proj2 = projectEntityOnAxis(axis, b);
        if(!projectionOverlap(proj1, proj2)) {
            return false;
        } else {
            float o = getProjectionOverlap(proj1, proj2);
            if (o < 0 && internalAxisFlags[i] & internalEdgePosetiveFlag) {
                continue;
            }
            if (o > 0 && internalAxisFlags[i] & internalEdgeNegativeFlag) {
                continue;
            }
            if(fabs(o) < fabs(overlap)) {
                overlap = o;
                smallestAxis = axis;
            }
        }
    }
    if (overlap == 0.0 || smallestAxis == v2{0, 0}) {
        return false;
    }
    (*mtv) = smallestAxis*overlap;
    return true;
}

bool entityCollidesWithAnything(GameState *state, Entity *entity, 
        v2 *mtv) {
    Rect ar = entity->collisionRect;
    ar.p += entity->p;
    v2 tempMtv{0, 0};
    for (Uint32 i = 0; i < arrayLength(state->entities); ++i) {
        Entity *other = &state->entities[i];
        if (entity != other) {
            Rect br = other->collisionRect;
            br.p += other->p;
            if (entitiesIntersect(ar, br, &tempMtv)) {
                (*mtv) = tempMtv;
                return true;
            }
        }
    }
    return false;
}



extern "C"
bool updateAndRenderGame(void *permStorage) {
    GameState *state = (GameState *)permStorage;
    // Init
    if (!state->isInitialised) {
        if (SDL_NumJoysticks() > 0) {
            state->joystick = SDL_JoystickOpen(0);
            if (state->joystick) {
                printf("Opened Joystick 0\n");
                printf("Name: %s\n", SDL_JoystickNameForIndex(0));
                printf("Number of Axes: %d\n", 
                        SDL_JoystickNumAxes(state->joystick));
                printf("Number of Buttons: %d\n", 
                        SDL_JoystickNumButtons(state->joystick));
                printf("Number of Balls: %d\n", 
                        SDL_JoystickNumBalls(state->joystick));
                printf("Number of Hats: %d\n", 
                        SDL_JoystickNumHats(state->joystick));
            } else {
                printf("Couldn't open Joystick 0\n");
            }
            SDL_JoystickEventState(0);
            SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
            SDL_EventState(SDL_KEYUP, SDL_IGNORE);

        }
        printf("GameState size %fMB\n", 
                ((float)sizeof(GameState))/Megabytes(1));

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
1000001000000000000000001\
1000000000000000000100001\
1000000000001000000000001\
1000010000000000000000001\
1000000000000000001000001\
1110000000000000000000001\
1110000000000000000000001\
1111111111111111111111111\
";

        // int tileSize = 25;
        for (int y = 0; y < TILE_MAP_SIZE_Y; ++y) {
            for (int x = 0; x < TILE_MAP_SIZE_X; ++x) {
                if (temp[x+y*TILE_MAP_SIZE_X] == '1') {
                    Entity *wallEntity = newEntity(state);

                    wallEntity->p.x = x * TILESIZE;
                    wallEntity->p.y = y * TILESIZE;
                    wallEntity->collisionRect.p.x = 0;
                    wallEntity->collisionRect.p.y = 0;
                    wallEntity->collisionRect.size.x = TILESIZE;
                    wallEntity->collisionRect.size.y = TILESIZE;
                    wallEntity->type = ENTITY_TYPE_WALL;
                    if (y > 0 && temp[x+(y-1)*TILE_MAP_SIZE_X] == '1') {
                        wallEntity->collisionRect.internalEdgeFlags[1] |= 
                            internalEdgeNegativeFlag;
                    }
                    if (y < TILE_MAP_SIZE_Y-1 && 
                            temp[x+(y+1)*TILE_MAP_SIZE_X] == '1') {
                        wallEntity->collisionRect.internalEdgeFlags[1] |= 
                            internalEdgePosetiveFlag;
                    }
                    if (x > 0 && temp[(x-1)+y*TILE_MAP_SIZE_X] == '1') {
                        wallEntity->collisionRect.internalEdgeFlags[0] |= 
                            internalEdgeNegativeFlag;
                    }
                    if (x < TILE_MAP_SIZE_X-1 && 
                            temp[(x+1)+y*TILE_MAP_SIZE_X] == '1') {
                        wallEntity->collisionRect.internalEdgeFlags[0] |= 
                            internalEdgePosetiveFlag;
                    }
                }
            }
        }

        // memcpy(state->tileMap, temp, 375);
        // Entity e = {};
        // e.p = {10, 10};
        // e.topSpeed = 1;
        // e.type = ENTITY_TYPE_PLAYER;
        // state->entities[0] = e;

        state->player = newEntity(state);
        state->player->p = {50, 50};
        state->player->collisionRect.size = v2{20, 20};
        state->player->topSpeed = 1;
        state->player->type = ENTITY_TYPE_PLAYER;
        state->isInitialised = true;
    }
    state->player->topSpeed = 4;
    float accVal = 0.5;
    v2 acc = {0, 0};
    // Joystick
    {
        SDL_JoystickUpdate();
        if (getJoystickAxis(state->joystick, 0) != 0 || 
                getJoystickAxis(state->joystick, 1) != 0) {
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

    // Keyboard
    {
        int numKeys;
        const Uint8 *keyboardState = SDL_GetKeyboardState(&numKeys);
        if (keyboardState[SDL_SCANCODE_ESCAPE] && 
                !state->lastKeyboardState[SDL_SCANCODE_ESCAPE]) {
            printf("break!");
        }
        if (keyboardState[SDL_SCANCODE_UP] && 
                !state->lastKeyboardState[SDL_SCANCODE_UP]) {
            if (state->player->velocity.y < 6 && 
                    state->player->velocity.y > -0.01) {
                state->player->velocity.y = -8;
                state->player->jumping = true;
            }
        }
        if (!keyboardState[SDL_SCANCODE_UP] && 
                state->lastKeyboardState[SDL_SCANCODE_UP]) {
            if (state->player->jumping) {
                state->player->velocity.y *= 0.35;
                state->player->jumping = false;
            }
        }
        if (keyboardState[SDL_SCANCODE_LEFT]) {
            acc.x -= accVal;
        }
        if (keyboardState[SDL_SCANCODE_RIGHT]) {
            acc.x += accVal;
        }

        memcpy((void *)state->lastKeyboardState, keyboardState, 
                numKeys*sizeof(Uint8));
    }
    // NOTE(holger): acc should get normalized when 
    // using digital input only
    // or if this is a platformer not at all!
    // acc = v2normalize(acc);
    // acc += v2{0, +0.8};
    // state->player->velocity += acc;
    // float damp = state->player->topSpeed/
    // (state->player->topSpeed+accVal);
    // state->player->velocity *= damp;
    // state->player->p += state->player->velocity;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_WINDOWEVENT : {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                    return true;
                }
            } break;
            case SDL_QUIT : {
            } break;
        }
    }
    // state->t += 0.001;
    // RenderRect rect = {};
    // rect.rect.p.x = (state->player->p.x*6);
    // rect.rect.p.y = (state->player->p.y*6);
    // rect.rect.size.x = 20;
    // rect.rect.size.y = 20;
    // rect.r = 127 + 100*sin(state->t);
    // rect.g = 127 + 100*sin(state->t*2.0);
    // rect.b = 127 + 100*sin(state->t*4.0);
    // rect.shown = true;
    // pushRenderRect(state, rect);


    // Update Entities
    {
        for (Uint64 i = 0; i < state->entitiesTop; ++i) {
            Entity *e = state->entities + i;
            switch(e->type) {
                case ENTITY_TYPE_PLAYER:{
                    if (e->jumping) {
                        if (e->velocity.y > -0.21) {
                            e->jumping = false;
                        }
                    }
                    v2 mtv{0, 0};
                    e->velocity += acc;
                    e->velocity.y += 0.3;
                    float damp = e->topSpeed/(e->topSpeed+accVal);
                    e->velocity.x *= damp;
                    e->p += e->velocity;
                    for (int k = 0; k < 5; k++) {
                        if (!entityCollidesWithAnything(state, e, &mtv)) {
                            break;
                        }
                        e->p += mtv*1;
                        if (fabs(mtv.x) > fabs(mtv.y)) {
                            e->velocity.x *= 0;
                        } else {
                            e->velocity.y *= 0;
                        }
                    }

                    state->t += 0.001;
                    RenderRect rect = {};
                    rect.rect.p.x = (e->p.x);
                    rect.rect.p.y = (e->p.y);
                    rect.rect.size.x = 20;
                    rect.rect.size.y = 20;
                    rect.r = 127;
                    rect.g = 127;
                    rect.b = 127;
                    rect.shown = true;
                    pushRenderRect(state, rect);
                } break;

                case ENTITY_TYPE_WALL:{
                    RenderRect tileRect = {};
                    tileRect.rect.p = e->p;
                    tileRect.rect.size.x = TILESIZE;
                    tileRect.rect.size.y = TILESIZE;
                    tileRect.r = 255;
                    tileRect.g = 255;
                    tileRect.b = 255;
                    tileRect.shown = true;
                pushRenderRect(state, tileRect);
                } break;

                default:{
                    SDL_assert(false);
                } break;
            }
        }
    }
    // int tileSize = 25;
    // for (int y = 0; y < TILE_MAP_SIZE_Y; ++y) {
        // for (int x = 0; x < TILE_MAP_SIZE_X; ++x) {
            // if (state->tileMap[x+y*TILE_MAP_SIZE_X] == '1') {
                // RenderRect tileRect = {};
                // tileRect.rect.p.x = x * (tileSize);
                // tileRect.rect.p.y = y * (tileSize);
                // tileRect.rect.size.x = tileSize;
                // tileRect.rect.size.y = tileSize;
                // tileRect.r = 255;
                // tileRect.g = 255;
                // tileRect.b = 255;
                // tileRect.shown = true;
                // pushRenderRect(state, tileRect);
            // }
        // }
    // }
    //state->renderRects[0] = rect;
    // SDL_Rect testBox = {};
    // testBox.x = (state->x);
    // testBox.y = (state->y);
    // testBox.w = 20;
    // testBox.h = 20;
    // SDL_SetRenderDrawColor(state->renderer, 127 + 100*sin(state->t), 
    // 127 + 100*sin(state->t*2.0), 127 + 100*sin(state->t*4.0), 255);
    // SDL_RenderFillRect(state->renderer, &testBox);
    // SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    return false;
}
