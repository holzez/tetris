#if !defined(TETRIS_PLAYER_H)

#include "tetris_typedefs.h"
#include "tetris_math.h"
#include "tetris_world.h"

#define PLAYER_DIM 5

static constexpr uint8 kPlayerKind0[PLAYER_DIM][PLAYER_DIM] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}};

static constexpr uint8 kPlayerKind1[PLAYER_DIM][PLAYER_DIM] = {
    {0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0}};

static constexpr uint8 kPlayerKind2[PLAYER_DIM][PLAYER_DIM] = {
    {0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 0, 0, 0}};

static constexpr uint8 kPlayerKind3[PLAYER_DIM][PLAYER_DIM] = {
    {0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}};

static constexpr uint8 kPlayerKind4[PLAYER_DIM][PLAYER_DIM] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0}};

struct player_t
{
    vec2i_t position;
    uint8 *data;
    uint8 value;
};

bool IsPlayerPositionValid(world_t *world, uint8 *playerData, vec2i_t testPosition)
{
    for (uint8 y = 0; y < PLAYER_DIM; ++y)
    {
        for (uint8 x = 0; x < PLAYER_DIM; ++x)
        {
            if (playerData[y * PLAYER_DIM + x])
            {
                vec2i_t position = testPosition + vec2i_t{x, y};

                if (!IsWorldPositionValid(world, position) ||
                    !IsValueEmpty(GetWorldValue(world, position)))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

void SavePlayerInWorld(world_t *world, player_t *player)
{
    if (player->value)
    {
        for (uint8 y = 0; y < PLAYER_DIM; ++y)
        {
            for (uint8 x = 0; x < PLAYER_DIM; ++x)
            {
                if (player->data[y * PLAYER_DIM + x])
                {
                    vec2i_t position = player->position + vec2i_t{x, y};
                    SetWorldValue(world, position, player->value);
                }
            }
        }
    }
}

void RotatePlayer(world_t *world, player_t *player)
{
    uint8 newData[PLAYER_DIM][PLAYER_DIM];

    for (int i = 0; i < PLAYER_DIM; ++i)
    {
        for (int j = 0; j < PLAYER_DIM; ++j)
        {
            newData[j][PLAYER_DIM - i - 1] = player->data[i * PLAYER_DIM + j];
        }
    }

    if (IsPlayerPositionValid(world, (uint8 *)newData, player->position))
    {
        for (int y = 0; y < PLAYER_DIM; ++y)
        {
            for (int x = 0; x < PLAYER_DIM; ++x)
            {
                player->data[y * PLAYER_DIM + x] = newData[y][x];
            }
        }
    }
}

void SpawnPlayer(world_t *world, player_t *player)
{
    player->position = {world->size.x / 2, -PLAYER_DIM};
    player->value = SDL_rand(3) + 1;

    uint8 *allPlayerData[8] = {
        (uint8 *)kPlayerKind0,
        (uint8 *)kPlayerKind1,
        (uint8 *)kPlayerKind2,
        (uint8 *)kPlayerKind3,
        (uint8 *)kPlayerKind4,
    };

    uint8 playerKind = SDL_rand(5);

    uint8 *playerData = allPlayerData[playerKind];

    if (playerData)
    {
        for (int y = 0; y < PLAYER_DIM; ++y)
        {
            for (int x = 0; x < PLAYER_DIM; ++x)
            {
                player->data[y * PLAYER_DIM + x] = playerData[y * PLAYER_DIM + x];
            }
        }
    }
    else
    {
        SDL_Log("Invalid player kind: %d", playerKind);
        SDL_assert(false);
    }
}

#define TETRIS_PLAYER_H
#endif
