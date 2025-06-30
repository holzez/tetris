#if !defined(TETRIS_FX_H)

#include <SDL3/SDL.h>
#include "tetris_typedefs.h"
#include "tetris_math.h"

#define MAX_FS_COUNT 16

struct fx_t
{
    bool enabled;
    uint64 startTimeMs;
    uint8 msPerFrame;
    uint64 lastFrameMs;
    uint64 frameTimer;
    uint8 frameCount;
    vec2i_t size;
    uint8 textureCount;
    SDL_Texture **textures;
    vec2i_t startPosition;
    vec2i_t endPosition;
    uint32 durationMs;
    real32 progress;
};

struct fx_pool_t
{
    fx_t *fxs[MAX_FS_COUNT];
};

void AddFx(fx_pool_t *fxPool, uint64 tickMs, vec2i_t size, uint8 textureCount, SDL_Texture **textures,
           vec2i_t startPosition, vec2i_t endPosition, uint32 durationMs)
{
    for (uint8 i = 0; i < MAX_FS_COUNT; ++i)
    {
        if (!fxPool->fxs[i] || !fxPool->fxs[i]->enabled)
        {
            fxPool->fxs[i] = (fx_t *)SDL_malloc(sizeof(fx_t));
            fx_t *fx = fxPool->fxs[i];
            fx->enabled = true;
            fx->startTimeMs = tickMs;
            fx->msPerFrame = 100;
            fx->lastFrameMs = tickMs;
            fx->frameTimer = 0;
            fx->frameCount = 0;
            fx->size = size;
            fx->textureCount = textureCount;
            fx->textures = textures;
            fx->startPosition = startPosition;
            fx->endPosition = endPosition;
            fx->durationMs = durationMs;
            fx->progress = 0.0f;
            break;
        }
    }
}

void UpdateFx(fx_t *fx, uint64 ticksMs)
{
    fx->progress = (real32)(ticksMs - fx->startTimeMs) / fx->durationMs;

    if (fx->progress > 1.0f)
    {
        fx->enabled = false;
        return;
    }

    uint64 dt = ticksMs - fx->lastFrameMs;
    fx->lastFrameMs = ticksMs;
    fx->frameTimer += dt;

    if (fx->frameTimer >= fx->msPerFrame)
    {
        fx->frameTimer = 0;
        fx->frameCount = (fx->frameCount + 1) % fx->textureCount;
    }
}

void UpdateFxPool(fx_pool_t *fxPool, uint64 tickMs)
{
    for (int i = 0; i < MAX_FS_COUNT; ++i)
    {
        if (fxPool->fxs[i] && fxPool->fxs[i]->enabled)
        {
            UpdateFx(fxPool->fxs[i], tickMs);
        }
    }
}

#define TETRIS_FX_H
#endif