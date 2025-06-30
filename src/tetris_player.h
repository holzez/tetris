#if !defined(TETRIS_PLAYER_H)

#include "tetris_typedefs.h"
#include "tetris_math.h"
#include "tetris_world.h"

#define PLAYER_DATA_GRID_MAX_SIZE 16
#define PLAYER_DATA_KIND_COUNT 7
#define PLAYER_VALUE_COUNT 7

struct player_data_t
{
    vec2i_t dim;
    uint8 *grid;
};

struct player_t
{
    vec2i_t position;
    player_data_t data;
    uint8 value;
    uint8 nextPlayerKindId;
    uint8 nextPlayerValue;
};

bool InitPlayer(player_t *player);

bool IsPlayerPositionValid(world_t *world, player_data_t *playerData, vec2i_t testPosition);

void SavePlayerInWorld(world_t *world, player_t *player);

void RotatePlayer(world_t *world, player_t *player);

player_data_t GetPlayerKind(uint8 playerKindId);

void SpawnPlayer(world_t *world, player_t *player);

void RenderPlayer(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                  player_data_t *playerData, vec2i_t playerPosition, uint8 playerValue, vec2_t offset);

void RenderPlayer(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                  player_t *player, vec2_t offset);

#define TETRIS_PLAYER_H
#endif
