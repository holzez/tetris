#include "tetris_world.h"

bool InitWorld(world_t *world)
{
    world->size = {16, 24};
    world->data = (uint8 *)SDL_calloc(world->size.x * world->size.y, sizeof(uint8));
    world->itemRenderSize = {40.0f, 40.0f};
    return world->data != 0;
}

/**
 * @note Negative Y valid and always empty.
 */
inline bool IsWorldPositionValid(world_t *world, vec2i_t position)
{
    return position.x >= 0 && position.x < world->size.x &&
           position.y < world->size.y;
}

/**
 * @note If position invalid, return 0.
 */
inline uint8 GetWorldValue(world_t *world, vec2i_t position)
{
    if (position.y < 0)
    {
        return 0;
    }

    if (IsWorldPositionValid(world, position))
    {
        return world->data[position.y * world->size.x + position.x];
    }
    else
    {
        return 0;
    }
}

inline uint8 GetWorldValueUnchecked(world_t *world, vec2i_t position)
{
    if (position.y < 0)
    {
        return 0;
    }

    return world->data[position.y * world->size.x + position.x];
}

inline void SetWorldValueUnchecked(world_t *world, vec2i_t position, uint8 value)
{
    if (position.y >= 0)
    {
        world->data[position.y * world->size.x + position.x] = value;
    }
}

inline void SetWorldValue(world_t *world, vec2i_t position, uint8 value)
{
    if (IsWorldPositionValid(world, position))
    {
        SetWorldValueUnchecked(world, position, value);
    }
}

inline bool IsValueEmpty(uint8 value)
{
    return value == 0;
}

bool IsWorldRowFilled(world_t *world, uint8 row)
{
    for (int x = 0; x < world->size.x; ++x)
    {
        if (IsValueEmpty(GetWorldValue(world, {x, row})))
        {
            return false;
        }
    }

    return true;
}

void ResetWorld(world_t *world)
{
    for (int y = 0; y < world->size.y; ++y)
    {
        for (int x = 0; x < world->size.x; ++x)
        {
            SetWorldValueUnchecked(world, {x, y}, 0);
        }
    }
}

void RenderWorldItem(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                     uint8 value, vec2i_t position, vec2_t offset)
{
    if (value)
    {
        SDL_FRect blockTextureSrcRect{
            45.0f,
            45.0f,
            30.0f,
            30.0f};

        SDL_Texture *blockTexture = GetValueTexture(assets, value);

        SDL_FRect rect{
            offset.x + position.x * world->itemRenderSize.w,
            offset.y + position.y * world->itemRenderSize.h,
            world->itemRenderSize.w,
            world->itemRenderSize.h};

        SDL_RenderTexture(renderer, blockTexture, &blockTextureSrcRect, &rect);
    }
}

void RenderWorld(SDL_Renderer *renderer, app_assets_t *assets, world_t *world, vec2_t offset)
{
    for (int itemY = 0; itemY < world->size.y; ++itemY)
    {
        for (int itemX = 0; itemX < world->size.x; ++itemX)
        {
            uint8 value = world->data[itemY * world->size.x + itemX];
            vec2i_t position{itemX, itemY};
            RenderWorldItem(renderer, assets, world, value, position, offset);
        }
    }
}