#include "tetris_assets.h"

SDL_Texture *LoadTextureFromFile(SDL_Renderer *renderer, char *file)
{
    SDL_Surface *surface = IMG_Load(file);

    if (!surface)
    {
        SDL_Log("Couldn't load img: %s", SDL_GetError());
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (!texture)
    {
        SDL_Log("Couldn't create texture from surface: %s", SDL_GetError());
        return nullptr;
    }

    SDL_DestroySurface(surface);

    return texture;
}

bool LoadAssets(SDL_Renderer *renderer, app_assets_t *assets)
{
    assets->bgPatternTexture = LoadTextureFromFile(renderer, "res/Pattern01.png");
    assets->borderTexture = LoadTextureFromFile(renderer, "res/Border.png");
    assets->gridPatternTexture = LoadTextureFromFile(renderer, "res/GridPattern.png");
    assets->blockTexture[0] = LoadTextureFromFile(renderer, "res/Tetromino_block1_1.png");
    assets->blockTexture[1] = LoadTextureFromFile(renderer, "res/Tetromino_block1_2.png");
    assets->blockTexture[2] = LoadTextureFromFile(renderer, "res/Tetromino_block1_3.png");
    assets->blockTexture[3] = LoadTextureFromFile(renderer, "res/Tetromino_block1_4.png");
    assets->blockTexture[4] = LoadTextureFromFile(renderer, "res/Tetromino_block1_5.png");
    assets->blockTexture[5] = LoadTextureFromFile(renderer, "res/Tetromino_block1_6.png");
    assets->blockTexture[6] = LoadTextureFromFile(renderer, "res/Tetromino_block1_7.png");
    assets->blockTextureCount = 7;

    assets->fxClean[0] = LoadTextureFromFile(renderer, "res/Fx_clean01.png");
    assets->fxClean[1] = LoadTextureFromFile(renderer, "res/Fx_clean02.png");
    assets->fxClean[2] = LoadTextureFromFile(renderer, "res/Fx_clean03.png");
    assets->fxClean[3] = LoadTextureFromFile(renderer, "res/Fx_clean04.png");
    assets->fxClean[4] = LoadTextureFromFile(renderer, "res/Fx_clean05.png");
    assets->fxClean[5] = LoadTextureFromFile(renderer, "res/Fx_clean06.png");
    assets->fxClean[6] = LoadTextureFromFile(renderer, "res/Fx_clean07.png");
    assets->fxClean[7] = LoadTextureFromFile(renderer, "res/Fx_clean08.png");
    assets->fxClean[8] = LoadTextureFromFile(renderer, "res/Fx_clean09.png");
    assets->fxCleanCount = 9;

    assets->bgMusic = Mix_LoadMUS("res/music.mp3");
    assets->gameOverMusic = Mix_LoadWAV("res/game-over.mp3");
    assets->placeSfx = Mix_LoadWAV("res/place-sfx.mp3");

    if (!assets->bgPatternTexture || !assets->borderTexture || !assets->gridPatternTexture || !assets->bgMusic || !assets->gameOverMusic || !assets->placeSfx)
    {
        return false;
    }

    for (int i = 0; i < assets->blockTextureCount; ++i)
    {
        if (!assets->blockTexture[i])
        {
            return false;
        }
    }

    for (int i = 0; i < assets->fxCleanCount; ++i)
    {
        if (!assets->fxClean[i])
        {
            return false;
        }
    }

    return true;
}

bool FreeAssets(app_assets_t *assets)
{
    Mix_FreeMusic(assets->bgMusic);
    Mix_FreeChunk(assets->gameOverMusic);
    Mix_FreeChunk(assets->placeSfx);

    SDL_DestroyTexture(assets->bgPatternTexture);
    SDL_DestroyTexture(assets->borderTexture);

    for (int i = 0; i < assets->blockTextureCount; ++i)
    {
        SDL_DestroyTexture(assets->blockTexture[i]);
    }

    for (int i = 0; i < assets->fxCleanCount; ++i)
    {
        SDL_DestroyTexture(assets->fxClean[i]);
    }

    return true;
}

inline SDL_Texture *GetValueTexture(app_assets_t *assets, uint8 value)
{
    SDL_assert(value > 0 && value <= assets->blockTextureCount);
    return assets->blockTexture[value - 1];
}