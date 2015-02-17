#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_assert.h>
#include <SDL_mixer.h>
#undef main
#include <stdint.h>
#include "mainGameCode.h"

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)

char sharedObjectPath[500];

void *handle;
bool (*updateAndRenderGame)(void *);
int loadSo() {
    dlerror();
    handle = dlopen(sharedObjectPath, RTLD_LOCAL|RTLD_NOW);
    if (!handle) {
        printf("dlopen error: %s\n", dlerror());
        return 0;
    }
    dlerror();
    *(void **)(&updateAndRenderGame) = dlsym(handle, "updateAndRenderGame");
    if (!updateAndRenderGame) {
        printf("dlsym error: %s\n", dlerror());
        return 0;
    }
    return 1;
}
int freeSo() {
    dlerror();
    if (dlclose(handle)) {
        printf("dlclose error: %s\n", dlerror());
        return 0;
    }
    handle = 0;
    updateAndRenderGame = 0;
    return 1;
    //TODO(öaklsdj): 
}
int checkIfFileModified(const char *path, time_t oldMTime, time_t *newMTime) {
    struct stat fileStat;
    int err = stat(path, &fileStat);
    if (err) {
        printf("failed to read stat\n");
        return 0;
    }
    (*newMTime) = fileStat.st_mtim.tv_nsec;
    return fileStat.st_mtim.tv_nsec != oldMTime;
}

int strippToDir(char *source, int sourceLength) {
    int index = 0;
    int last = 0;
    while (*source++) {
        ++index;
        if (*source == '/') {
            last = index + 1;
        }
    }
    return last;
}

void getAbsolutFromRelativePath(GameState *state, char const *relativePath, char *destBuf, int destBufSize) {
    //int relSize = strlen(relativePath);
    //SDL_assert( < destBufSize);
    strcpy(destBuf, state->executablePath);
    strcpy(destBuf + state->onePastLastSlash, relativePath);
} 

int main(int argc, char const *argv[])
{


    handle = 0;
    updateAndRenderGame = 0;
    time_t oldMTime = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        _exit(-1);
    }

    void *permStorage = calloc(1, Megabytes(64));
    GameState *state = (GameState *)permStorage;
    readlink("/proc/self/exe", state->executablePath, sizeof(state->executablePath));
    state->onePastLastSlash = strippToDir(state->executablePath, sizeof(state->executablePath));
    state->executablePath[state->onePastLastSlash] = 0;
    getAbsolutFromRelativePath(state, "GameCode/gameCode.so", sharedObjectPath, sizeof(sharedObjectPath));
    printf("%s\n", sharedObjectPath);
    if (!loadSo()) {
        
        return 0;
    }

    SDL_Window *window = 0;
    
    if (SDL_CreateWindowAndRenderer(800, 600, 0, &window, &state->renderer) != 0) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        _exit(-1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("open audio error: %s\n", Mix_GetError());
        _exit(-1);
    }
    Uint32 previous = SDL_GetTicks();
    Uint32 lag = 0;
    // while (1) {
    //     Uint32 current = SDL_GetTicks();
    //     Uint32 elapsed = current - previous;
    //     previous = current;
    //     if (Event::processEvents()) {
    //         quitGraphics();
    //         return;
    //     }
    //     lag += elapsed;
    //     if (Event::getHaveFocus()) {
    //         while (lag >= 10) {
    //             step(sc);
    //             lag -= 10;
    //             Event::updatePressReleaseState();
    //         }
    //     }
    //     renderStep();
    // }
    bool running = true;
    while (running) {
        time_t newMTime = 0;
        if (checkIfFileModified("./file", oldMTime, &newMTime)) {
            printf("Modified\n");
            oldMTime = newMTime;
            if (!freeSo()) {
                return 0;
            }
            if (!loadSo()) {
                return 0;
            }
        }

        SDL_RenderClear(state->renderer);
        Uint32 current = SDL_GetTicks();
        Uint32 elapsed = current - previous;
        previous = current;
        lag += elapsed;
        while (lag >= 10) {
            memset(state->renderRects, 0, sizeof(state->renderRects));
            state->renderRectsTop = 0;
            if (updateAndRenderGame(permStorage)) {
                running = false;
                break;
            }
            lag -= 10;
        }
        for (int i = 0; i < 200; i++) {
            RenderRect *rect = state->renderRects + i;
            if (rect->shown) {
                SDL_SetRenderDrawColor(state->renderer, rect->r, rect->g, rect->b, 255);
                SDL_Rect sdlrect = {};
                sdlrect.x = rect->x+0.5;
                sdlrect.y = rect->y+0.5;
                sdlrect.h = rect->h+0.5;
                sdlrect.w = rect->w+0.5;
                SDL_RenderFillRect(state->renderer, &sdlrect);
            }
        }
        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
        SDL_RenderPresent(state->renderer);
    }
    if (state->music) {
        Mix_FreeMusic(state->music);
        state->music = 0;
    }
    Mix_Quit();
    SDL_Quit();
    return 0;
}
