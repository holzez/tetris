#if !defined(TETRIS_WORLD_H)

#include "tetris_typedefs.h"
#include "tetris_math.h"

struct world_t
{
    vec2i_t size;
    /**
     * @brief The world data.
     * @note The data is stored in a row-major order.
     * If Y is negative, the value must be 0 without checking data.
     */
    uint8 *data;
};

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

#define TETRIS_WORLD_H
#endif