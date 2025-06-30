#if !defined(TETRIS_WORLD_H)

#include "tetris_typedefs.h"
#include "tetris_math.h"
#include "tetris_assets.h"

struct world_t
{
    vec2_t itemRenderSize;

    vec2i_t size;
    /**
     * @brief The world data.
     * @note The data is stored in a row-major order.
     * If Y is negative, the value must be 0 without checking data.
     */
    uint8 *data;
};

bool InitWorld(world_t *world);

/**
 * @note Negative Y valid and always empty.
 */
bool IsWorldPositionValid(world_t *world, vec2i_t position);

/**
 * @note If position invalid, return 0.
 */
uint8 GetWorldValue(world_t *world, vec2i_t position);

uint8 GetWorldValueUnchecked(world_t *world, vec2i_t position);

void SetWorldValueUnchecked(world_t *world, vec2i_t position, uint8 value);

void SetWorldValue(world_t *world, vec2i_t position, uint8 value);

bool IsValueEmpty(uint8 value);

bool IsWorldRowFilled(world_t *world, uint8 row);

void ResetWorld(world_t *world);

void RenderWorldItem(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                     uint8 value, vec2i_t position, vec2_t offset);

void RenderWorld(SDL_Renderer *renderer, app_assets_t *assets, world_t *world, vec2_t offset);

#define TETRIS_WORLD_H
#endif