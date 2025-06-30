#include "tetris_player.h"

static constexpr uint8 kPlayerKind0[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {1, 1, 1, 1}};

static constexpr uint8 kPlayerKind1[2][2] = {
    {1, 1},
    {1, 1}};

static constexpr uint8 kPlayerKind2[3][3] = {
    {1, 0, 0},
    {1, 0, 0},
    {1, 1, 0}};

static constexpr uint8 kPlayerKind3[3][3] = {
    {1, 0, 0},
    {1, 1, 0},
    {0, 1, 0}};

static constexpr uint8 kPlayerKind4[3][3] = {
    {0, 0, 0},
    {1, 1, 1},
    {0, 1, 0}};

static constexpr uint8 kPlayerKind5[3][3] = {
    {0, 0, 1},
    {0, 0, 1},
    {0, 1, 1}};

static constexpr uint8 kPlayerKind6[3][3] = {
    {0, 0, 1},
    {0, 1, 1},
    {0, 1, 0}};

static player_data_t kPlayerData0 = {
    {4, 4},
    (uint8 *)kPlayerKind0};

static player_data_t kPlayerData1 = {
    {2, 2},
    (uint8 *)kPlayerKind1};

static player_data_t kPlayerData2 = {
    {3, 3},
    (uint8 *)kPlayerKind2};

static player_data_t kPlayerData3 = {
    {3, 3},
    (uint8 *)kPlayerKind3};

static player_data_t kPlayerData4 = {
    {3, 3},
    (uint8 *)kPlayerKind4};

static player_data_t kPlayerData5 = {
    {3, 3},
    (uint8 *)kPlayerKind5};

static player_data_t kPlayerData6 = {
    {3, 3},
    (uint8 *)kPlayerKind6};

bool InitPlayer(player_t *player)
{
    player->nextPlayerKindId = SDL_rand(PLAYER_DATA_KIND_COUNT);
    player->nextPlayerValue = SDL_rand(PLAYER_VALUE_COUNT) + 1;
    player->data.grid = (uint8 *)SDL_calloc(PLAYER_DATA_GRID_MAX_SIZE * PLAYER_DATA_GRID_MAX_SIZE, sizeof(uint8));

    return player->data.grid != 0;
}

bool IsPlayerPositionValid(world_t *world, player_data_t *playerData, vec2i_t testPosition)
{
    for (uint8 y = 0; y < playerData->dim.y; ++y)
    {
        for (uint8 x = 0; x < playerData->dim.x; ++x)
        {
            if (playerData->grid[y * playerData->dim.x + x])
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
        for (uint8 y = 0; y < player->data.dim.y; ++y)
        {
            for (uint8 x = 0; x < player->data.dim.x; ++x)
            {
                if (player->data.grid[y * player->data.dim.x + x])
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
    uint8 newData[PLAYER_DATA_GRID_MAX_SIZE][PLAYER_DATA_GRID_MAX_SIZE];
    SDL_assert(player->data.dim.x == player->data.dim.y);
    SDL_assert(player->data.dim.x <= PLAYER_DATA_GRID_MAX_SIZE);
    SDL_assert(player->data.dim.y <= PLAYER_DATA_GRID_MAX_SIZE);

    uint8 sqDim = player->data.dim.x;

    for (int i = 0; i < sqDim; ++i)
    {
        for (int j = 0; j < sqDim; ++j)
        {
            newData[j][sqDim - i - 1] = player->data.grid[i * sqDim + j];
        }
    }

    player_data_t newPlayerData{
        player->data.dim,
        (uint8 *)newData};

    if (IsPlayerPositionValid(world, &newPlayerData, player->position))
    {
        for (int y = 0; y < player->data.dim.y; ++y)
        {
            for (int x = 0; x < player->data.dim.x; ++x)
            {
                player->data.grid[y * player->data.dim.x + x] = newData[y][x];
            }
        }
    }
}

player_data_t GetPlayerKind(uint8 playerKindId)
{
    player_data_t allPlayerData[8] = {
        kPlayerData0,
        kPlayerData1,
        kPlayerData2,
        kPlayerData3,
        kPlayerData4,
        kPlayerData5,
        kPlayerData6,
    };

    return allPlayerData[playerKindId];
}

void SpawnPlayer(world_t *world, player_t *player)
{
    player->value = player->nextPlayerValue;
    uint8 playerKindId = player->nextPlayerKindId;
    player->nextPlayerKindId = SDL_rand(PLAYER_DATA_KIND_COUNT);
    player->nextPlayerValue = SDL_rand(PLAYER_VALUE_COUNT) + 1;

    player_data_t newPlayerData = GetPlayerKind(playerKindId);
    player->data.dim = newPlayerData.dim;

    for (int y = 0; y < newPlayerData.dim.y; ++y)
    {
        for (int x = 0; x < newPlayerData.dim.x; ++x)
        {
            player->data.grid[y * newPlayerData.dim.x + x] = newPlayerData.grid[y * newPlayerData.dim.x + x];
        }
    }

    player->position = {(world->size.x - player->data.dim.x) / 2, -player->data.dim.y};
}

void RenderPlayer(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                  player_data_t *playerData, vec2i_t playerPosition, uint8 playerValue, vec2_t offset)
{
    for (uint8 y = 0; y < playerData->dim.y; ++y)
    {
        for (uint8 x = 0; x < playerData->dim.x; ++x)
        {
            uint8 hasValue = playerData->grid[y * playerData->dim.x + x];

            if (hasValue && playerPosition.y + y >= 0)
            {
                RenderWorldItem(renderer, assets, world, playerValue, playerPosition + vec2i_t{x, y}, offset);
            }
        }
    }
}

void RenderPlayer(SDL_Renderer *renderer, app_assets_t *assets, world_t *world,
                  player_t *player, vec2_t offset)
{
    RenderPlayer(renderer, assets, world, &player->data, player->position, player->value, offset);
}