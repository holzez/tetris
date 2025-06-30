#if !defined(TETRIS_ASSETS_H)

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "tetris_typedefs.h"

struct app_assets_t
{
    SDL_Texture *bgPatternTexture;
    SDL_Texture *borderTexture;
    SDL_Texture *gridPatternTexture;

    uint8 blockTextureCount;
    SDL_Texture *blockTexture[9];

    uint8 fxCleanCount;
    SDL_Texture *fxClean[9];

    Mix_Music *bgMusic;
    Mix_Chunk *gameOverMusic;
    Mix_Chunk *placeSfx;
};

SDL_Texture *LoadTextureFromFile(SDL_Renderer *renderer, char *file);

bool LoadAssets(SDL_Renderer *renderer, app_assets_t *assets);

bool FreeAssets(app_assets_t *assets);

SDL_Texture *GetValueTexture(app_assets_t *assets, uint8 value);

#define TETRIS_ASSETS_H
#endif