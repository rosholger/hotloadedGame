#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#undef main
#include <stdint.h>
#include "mainGameCode.h"

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)



void *handle;
bool (*updateAndRenderGame)(void *);
int loadSo() {
    dlerror();
    handle = dlopen("./GameCode/gameCode.so", RTLD_LOCAL|RTLD_NOW);
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
int main(int argc, char const *argv[])
{
    handle = 0;
    updateAndRenderGame = 0;
    if (!loadSo()) {
        
        return 0;
    }
    time_t oldMTime = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        _exit(-1);
    }

    void *permStorage = calloc(1, Megabytes(64));
    GameState *state = (GameState *)permStorage;
    SDL_Window *window = 0;
    
    if (SDL_CreateWindowAndRenderer(800, 600, 0, &window, &state->renderer) != 0) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        _exit(-1);
    }
    while(1) {
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
        if (updateAndRenderGame(permStorage)) {
            break;
        }
        SDL_RenderPresent(state->renderer);
        usleep(16);
    }
    SDL_Quit();
    return 0;
}