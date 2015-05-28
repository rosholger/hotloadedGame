#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_assert.h"
#include "SDL_RWops.h"
#include "SDL_mixer.h"
#include "SDL_image.h"
#undef main
#include <stdint.h>
#include "mainGameCode.h"
#include "jsmn.h"
#include <windows.h>

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)

char sharedObjectPath[500];
char sharedObjectCopyPath[500];

//void *handle;
HMODULE handle;
bool (*updateAndRenderGame)(void *);
int loadSo() {
//    dlerror();
//    handle = dlopen(sharedObjectPath, RTLD_LOCAL|RTLD_NOW);
//    if (!handle) {
//        printf("dlopen error: %s\n", dlerror());
//        return 0;
//    }
//    dlerror();
//    *(void **)(&updateAndRenderGame) = dlsym(handle, "updateAndRenderGame");
//    if (!updateAndRenderGame) {
//        printf("dlsym error: %s\n", dlerror());
//        return 0;
//    }
    SDL_Delay(250);
    if (!CopyFile(sharedObjectPath, sharedObjectCopyPath, false)) {
        printf("CopyFile error: 0x%08x tried to copy %s to %s\n", GetLastError(), sharedObjectPath, sharedObjectCopyPath);
        return 0;
    }
    handle = LoadLibrary(sharedObjectCopyPath);
    if (!handle) {
        printf("LoadLibrary error: %d, tried to LoadLibrary: %s\n", GetLastError(), sharedObjectCopyPath);
        return 0;
    }

    *(FARPROC *)(&updateAndRenderGame) = GetProcAddress(handle, "updateAndRenderGame");
    if (!updateAndRenderGame) {
        printf("GetProcAddress error: %d\n", GetLastError());
        return 0;
    }
    return 1;
}
int freeSo() {
    if (!FreeLibrary(handle)) {
        printf("CloseHandle error: 0x%08x\n", GetLastError());
        return 0;
    }
    updateAndRenderGame = 0;

//    dlerror();
//    if (dlclose(handle)) {
//        printf("dlclose error: %s\n", dlerror());
//        return 0;
//    }
//    handle = 0;
//    updateAndRenderGame = 0;
    return 1;
}
int checkIfFileModified(const char *path, FILETIME oldMTime, FILETIME *newMTime) {
//    struct stat fileStat;
//    int err = stat(path, &fileStat);
//    if (err) {
//        printf("failed to read stat\n");
//        return 0;
//    }
//    (*newMTime) = fileStat.st_mtim.tv_nsec;
//    return fileStat.st_mtim.tv_nsec != oldMTime;
    WIN32_FIND_DATA data;
    HANDLE t = FindFirstFile(sharedObjectPath, &data);
    (*newMTime) = data.ftLastWriteTime;
    FindClose(t);
    return CompareFileTime(newMTime, &oldMTime);
}

int strippToDir(char *source, int sourceLength) {
    int index = 0;
    int last = 0;
    while (*source++) {
        ++index;
        if (*source == '\\') {
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

void waitForKey() {
    if (IsDebuggerPresent()) {
        printf("Press any key to exit.");
        getchar();
    }
}

size_t writeStringToRW(SDL_RWops *rwops, const char *str) {
    Uint32 strSize = strlen(str);
    size_t result = SDL_RWwrite(rwops, str, strSize, 1);
    return result;
}
#if 0
void writeJsonObj(nx_json *node, SDL_RWops *rwops);

void writeJsonArray(nx_json *node, SDL_RWops *rwops);

void writeJsonNode(nx_json *node, SDL_RWops *rwops) {
    switch(node->type) {
        case NX_JSON_NULL :
            writeStringToRW(rwops, "null,\n");
            break;
        case NX_JSON_OBJECT :
            writeStringToRW(rwops, "{\n");
            writeJsonObj(node->child, rwops);
            writeStringToRW(rwops, "}\n");
            break;
        case NX_JSON_ARRAY :
            writeStringToRW(rwops, "[\n");
            writeJsonArray(node->child, rwops);
            writeStringToRW(rwops, "],\n");
            break;
        case NX_JSON_STRING :
            writeStringToRW(rwops, "\"");
            writeStringToRW(rwops, node->text_value);
            writeStringToRW(rwops, "\",\n");
            break;
        case NX_JSON_INTEGER :
            {
                char buf[50];
                _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%d,\n", node->int_value);
                writeStringToRW(rwops, buf);
            }
            break;
        case NX_JSON_DOUBLE :
            {
                char buf[50];
                _snprintf_s(buf, sizeof(buf), _TRUNCATE, "%lf,\n", node->dbl_value);
                writeStringToRW(rwops, buf);
            }
            break;
        case NX_JSON_BOOL :
            if (node->int_value) {
                writeStringToRW(rwops, "true,\n");
            } else {
                writeStringToRW(rwops, "false,\n");
            }
            break;
    }
}

void writeJsonArray(nx_json *node, SDL_RWops *rwops) {
    writeJsonNode(node, rwops);
    if (node->next) {
        writeJsonArray(node->next, rwops);
    }
}


void writeJsonObj(nx_json *node, SDL_RWops *rwops) {
    writeStringToRW(rwops, "\"");
    writeStringToRW(rwops, node->key);
    writeStringToRW(rwops, "\" : ");
    writeJsonNode(node, rwops);
    if (node->next) {
        writeJsonObj(node->next, rwops);
    }
}


void printJsonObj(nx_json *node);

void printJsonArray(nx_json *node) {
    switch(node->type) {
        case NX_JSON_NULL :
            printf("null,\n");
            break;
        case NX_JSON_OBJECT :
            printf("{\n");
            printJsonObj(node->child);
            printf("},\n");
            break;
        case NX_JSON_ARRAY :
            printf("[\n");
            printJsonArray(node->child);
            printf("],\n");
            break;
        case NX_JSON_STRING :
            printf("\"%s\",\n", node->text_value);
            break;
        case NX_JSON_INTEGER :
            printf("%d,\n", node->int_value);
            break;
        case NX_JSON_DOUBLE :
            printf("%lf,\n", node->dbl_value);
            break;
        case NX_JSON_BOOL :
            if (node->int_value) {
                printf("true,\n");
            } else {
                printf("false,\n");
            }
            break;
    }
    if (node->next) {
        printJsonArray(node->next);
    }
}

void printJsonObj(nx_json *node) {
    printf("\"%s\" : ", node->key);
    switch(node->type) {
        case NX_JSON_NULL :
            printf("null,\n");
            break;
        case NX_JSON_OBJECT :
            printf("{\n");
            printJsonObj(node->child);
            printf("},\n");
            break;
        case NX_JSON_ARRAY :
            printf("[\n");
            printJsonArray(node->child);
            printf("],\n");
            break;
        case NX_JSON_STRING :
            printf("\"%s\",\n", node->text_value);
            break;
        case NX_JSON_INTEGER :
            printf("%d,\n", node->int_value);
            break;
        case NX_JSON_DOUBLE :
            printf("%lf,\n", node->dbl_value);
            break;
        case NX_JSON_BOOL :
            if (node->int_value) {
                printf("true,\n");
            } else {
                printf("false,\n");
            }
            break;
    }
    if (node->next) {
        printJsonObj(node->next);
    }
}
#endif

enum jsonType {
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_FLOAT,
    JSON_TYPE_INT,
    JSON_TYPE_NULL,
    JSON_TYPE_BOOL,
    JSON_TYPE_STRING
};

struct jsonNode {
    jsonType type;
    char *name; //null if child of array;
    jsonNode *next;
    union { // ALL INVALID IF NULL!
        jsonNode *child; //object and array value;
        float floatValue;
        int intValue;
        bool boolValue;
        char *stringValue;
    };
};


void printAllJsmnTokens(jsmntok_t *tok, int len, char *source, jsonNode *node, int *numNodes);

int parseJsmnObject(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes);

int parseJsmnArray(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes);

int parseValue(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes) {
    int numElementsForward = 1;
    switch(tok->type) {
        case JSMN_PRIMITIVE :
            if (node) {
                if (*(source+tok->start) == 't') {
                    node->type = JSON_TYPE_BOOL;
                    node->boolValue = true;
                } else if (*(source+tok->start) == 'f') {
                    node->type = JSON_TYPE_BOOL;
                    node->boolValue = false;
                } else if (*(source+tok->start) == 'n') {
                    node->type = JSON_TYPE_NULL;
                } else if (*(source+tok->start) == '-' || (*(source+tok->start) >= '0' && *(source+tok->start) <= '9')) {
                    char buf[50];
                    memcpy(buf, (source+tok->start), tok->end - tok->start);
                    buf[tok->end - tok->start] = 0;
                    double fval = atof(buf);
                    if (fval == (int)fval) {
                        node->type = JSON_TYPE_INT;
                        node->intValue = fval;
                    } else {
                        node->type = JSON_TYPE_FLOAT;
                        node->floatValue = fval;
                    }
                }
            }
            break;
        case JSMN_STRING :
            if (node) {
                node->type = JSON_TYPE_STRING;
                node->stringValue = (char *)malloc((tok->end - tok->start)+1);
                memcpy(node->stringValue, (source+tok->start), tok->end - tok->start);
                node->stringValue[tok->end - tok->start] = 0;
            }
            break;
        case JSMN_OBJECT :
            numElementsForward = parseJsmnObject(tok, source, node, numNodes);
            break;
        case JSMN_ARRAY :
            numElementsForward = parseJsmnArray(tok, source, node, numNodes);
            break;
        default :
            break;
    }
     return numElementsForward;
}

int parseKeyValuePair(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes) {
    int numElementsForward = 1;
    if (node) {
        node->name = (char *)malloc((tok->end - tok->start)+1);
        memcpy(node->name, (source+tok->start), tok->end - tok->start);
        node->name[tok->end - tok->start] = 0;
    }
    numElementsForward += parseValue(tok+1, source, node, numNodes);
    return numElementsForward;
}

int parseJsmnArray(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes) {
    int numElements = tok->size;
    int numElementsForward = 1;
    ++tok;
    if (node) {
        node->type = JSON_TYPE_ARRAY;
        node->child = (jsonNode *)calloc(sizeof(jsonNode), 1);
    }
    jsonNode *currNode = 0;
    for (int i = 0; i < numElements; ++i) {
        if (!currNode) {
            if (numNodes) {
                ++(*numNodes);
            }
            if (node) {
                currNode = node->child;
            }
        } else {
            if (numNodes) {
                ++(*numNodes);
            }
            currNode->next = (jsonNode *)calloc(sizeof(jsonNode), 1);
            currNode = currNode->next;
        }
        int tempForward = parseValue(tok, source, currNode, numNodes);
        tok += tempForward;
        numElementsForward += tempForward;
    }
    return numElementsForward;
}

//node can be null, then it wont fill anything,
//if numNodes is not null it will be filled with the number of nodes that need to be alloced.
int parseJsmnObject(jsmntok_t *tok, char *source, jsonNode *node, int *numNodes) {
    int numElements = tok->size;
    int numElementsForward = 1;
    ++tok;
    if (node) {
        node->type = JSON_TYPE_OBJECT;
        node->child = (jsonNode *)calloc(sizeof(jsonNode), 1);
    }
    jsonNode *currNode = 0;
    for (int i = 0; i < numElements; ++i) {
        if (!currNode) {
            if (numNodes) {
                ++(*numNodes);
            }
            if (node) {
                currNode = node->child;
            }
        } else {
            if (numNodes) {
                ++(*numNodes);
            }
            currNode->next = (jsonNode *)calloc(sizeof(jsonNode), 1);
            currNode = currNode->next;
        }
        if (tok->type == JSMN_STRING || tok->type == JSMN_PRIMITIVE) {
            int tempForward = parseKeyValuePair(tok, source, currNode, numNodes);
            tok += tempForward;
            numElementsForward += tempForward;
        }
    }
    return numElementsForward;
}

void parseJsonRW(SDL_RWops *jsonRW, jsonNode *node) {
    char jsonRawData[500];
    int numRead = SDL_RWread(jsonRW, jsonRawData, sizeof(char), 500);
    jsonRawData[numRead] = 0;
    jsmn_parser parser;
    jsmn_init(&parser);
    int tokensSize = jsmn_parse(&parser, jsonRawData, 500, NULL, 0);
    jsmntok_t *tokens = (jsmntok_t *)calloc(tokensSize*sizeof(jsmntok_t), 1);
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, jsonRawData, 500, tokens, tokensSize);
    if (ret >= 0) {
        //parseJsmnObject(tokens, jsonRawData, node);
        int nodesToAlloc = 0;
        parseJsmnObject(tokens, jsonRawData, 0, &nodesToAlloc);
        printf("%d\n", nodesToAlloc);
    } else {
        switch(ret) {
            case JSMN_ERROR_INVAL: printf("inval"); break;
            case JSMN_ERROR_NOMEM: printf("nomem"); break;
            case JSMN_ERROR_PART: printf("part"); break;
        }
    }

}

int main(int argc, char const *argv[])
{

//    handle = 0;
    updateAndRenderGame = 0;
    FILETIME oldMTime = {};

    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        waitForKey();
        _exit(-1);
    }

    void *permStorage = calloc(1, Megabytes(64));
    GameState *state = (GameState *)permStorage;
    if (GetModuleFileName(0, state->executablePath, 500) < 0) {
        printf("GetModuleFileName err");
        waitForKey();
        return 0;
    }
    state->onePastLastSlash = strippToDir(state->executablePath, sizeof(state->executablePath));
    state->executablePath[state->onePastLastSlash] = 0;
    getAbsolutFromRelativePath(state, "gameCode.dll", sharedObjectPath, sizeof(sharedObjectPath));
    getAbsolutFromRelativePath(state, "gameCode_copy.dll", sharedObjectCopyPath, sizeof(sharedObjectCopyPath));
    printf("%s\n", sharedObjectPath);

#if 0
    SDL_Window *window = 0;
    if (SDL_CreateWindowAndRenderer(800, 600, 0, &window, &state->renderer) != 0) {
        printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        waitForKey();
        _exit(-1);
    }
    if (!loadSo()) {
        waitForKey();
        return 0;
    }
//    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
//        printf("open audio error: %s\n", Mix_GetError());
//        _exit(-1);
//    }
    Uint32 previous = SDL_GetTicks();
    Uint32 lag = 0;
    checkIfFileModified(sharedObjectPath, oldMTime, &oldMTime);
    bool running = true;
    while (running) {
        FILETIME newMTime;
        if (checkIfFileModified("./file", oldMTime, &newMTime)) {
            printf("Modified\n");
            oldMTime = newMTime;
            if (!freeSo()) {
                waitForKey();
                return 0;
            }
            if (!loadSo()) {
                waitForKey();
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
//    if (state->music) {
//        Mix_FreeMusic(state->music);
//        state->music = 0;
//    }
//    Mix_Quit();
    SDL_Quit();
    waitForKey();
    return 0;
#else

    char jsonPath[500];
    getAbsolutFromRelativePath(state, "testJson.json", jsonPath, sizeof(jsonPath));
    SDL_RWops *jsonRW = SDL_RWFromFile(jsonPath, "r");
#if 0
    char jsonRawData[500];
    int numRead = SDL_RWread(jsonRW, jsonRawData, sizeof(char), 500);
    jsonRawData[numRead] = 0;
    const nx_json *rootNode = nx_json_parse(jsonRawData, 0);
    printf("{\n");
    printJsonObj(rootNode->child);
    printf("}\n");

    SDL_RWclose(jsonRW);
    jsonRW = SDL_RWFromFile(jsonPath, "w");
    writeStringToRW(jsonRW, "{\n");
    writeJsonObj(rootNode->child, jsonRW);
    writeStringToRW(jsonRW, "}\n");
#endif
    jsonNode node = {};
    parseJsonRW(jsonRW, &node);
    SDL_RWclose(jsonRW);

    SDL_Quit();
    waitForKey();

    return 0;
#endif
}
