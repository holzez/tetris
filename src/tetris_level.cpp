#include "tetris_level.h"

void ResumeLevel(level_t *level)
{
    level->paused = false;
}

void PauseLevel(level_t *level)
{
    if (!level->gameOver)
    {
        level->paused = true;
    }
}

void ToggleLevelPaused(level_t *level)
{
    if (level->paused)
    {
        ResumeLevel(level);
    }
    else
    {
        PauseLevel(level);
    }
}

void SetLevelGameOver(level_t *level)
{
    level->gameOver = true;
    level->paused = false;
}

void MovePlayer(world_t *world, player_t *player, game_input_t *input)
{
    vec2i_t newPosition = player->position;

    /**
     * @todo Handle holding buttons
     */
    if (WasInputButtonPressedOnce(&input->left))
    {
        newPosition.x -= 1;
    }

    if (WasInputButtonPressedOnce(&input->right))
    {
        newPosition.x += 1;
    }

#if 0
    newPosition.x -= GetInputButtonDownCount(&input->left);
    newPosition.x += GetInputButtonDownCount(&input->right);
#endif

    if (IsPlayerPositionValid(world, &player->data, newPosition))
    {
        player->position = newPosition;
    }

    if (WasInputButtonPressedOnce(&input->rotate))
    {
        RotatePlayer(world, player);
    }
}

bool CheckGameOver(world_t *world, player_t *player)
{
    for (int x = 0; x < world->size.x; ++x)
    {
        if (!IsValueEmpty(GetWorldValue(world, {x, 0})))
        {
            return true;
        }
    }

    if (!IsPlayerPositionValid(world, &player->data, player->position))
    {
        return false;
    }

    return false;
}

uint8 DestroyFilledRows(world_t *world)
{
    uint8 destroyedRows = 0;

    for (int8 y = world->size.y - 1; y >= 0; --y)
    {
        if (IsWorldRowFilled(world, y))
        {
            destroyedRows++;

            /**
             * @todo Destroy rows after animation
             */

            for (int8 row = y; row > 0; --row)
            {
                for (int x = 0; x < world->size.x; ++x)
                {
                    SetWorldValueUnchecked(world, {x, row}, GetWorldValueUnchecked(world, {x, row - 1}));
                }
            }

            for (int x = 0; x < world->size.x; ++x)
            {
                SetWorldValueUnchecked(world, {x, 0}, 0);
            }

            y++;
        }
    }

    return destroyedRows;
}

void ResetLevel(level_t *level)
{
    level->paused = false;
    level->gameOver = false;
    level->score = 0;
    level->stepMs = MAX_STEP_MS;
}

void Restart(level_t *level)
{
    ResetLevel(level);
    ResetWorld(&level->world);
    SpawnPlayer(&level->world, &level->player);

    /**
     * @todo Gamepad LED and Music
     */
#if 0
    if (level->input.gamepadId)
    {
        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(appState->input.gamepadId);

        if (gamepad)
        {
            SDL_SetGamepadLED(gamepad, 0x00, 0x00, 0xFF);
        }
    }

    Mix_PlayMusic(level->assets.bgMusic, -1);
#endif
}

void ApplyLevelInput(level_t *level, uint64 dt, game_input_t *input)
{
    if (!level->paused && !level->gameOver)
    {
        MovePlayer(&level->world, &level->player, input);
        level->currentStepMs = input->down.isDown ? MIN_STEP_MS : level->stepMs;
    }
}

void DoLevelStep(level_t *level, game_input_t *input, uint64 dt)
{
    level->stepAccumulator += dt;

    if (level->stepAccumulator >= level->currentStepMs)
    {
        level->stepAccumulator = 0;

        if (!level->paused && !level->gameOver)
        {
            vec2i_t newPosition = level->player.position + vec2i_t{0, 1};

            if (IsPlayerPositionValid(&level->world, &level->player.data, newPosition))
            {
                level->player.position = newPosition;
            }
            else
            {
                // Mix_PlayChannel(SOUND_CHANNEL_SFX, as->assets.placeSfx, 0);

                SavePlayerInWorld(&level->world, &level->player);
                uint8 destroyedRows = DestroyFilledRows(&level->world);
                level->score += destroyedRows * SCORE_PER_ROW;
                SpawnPlayer(&level->world, &level->player);

                if (input->gamepadId)
                {
                    SDL_Gamepad *gamepad = SDL_GetGamepadFromID(input->gamepadId);

                    if (gamepad)
                    {
                        uint16 strength = destroyedRows ? 0xFFFF : 0x1000;
                        SDL_RumbleGamepad(gamepad, strength, strength, 500);
                    }
                }

                if (destroyedRows)
                {
                    level->stepMs = SDL_max(MIN_STEP_MS, level->stepMs - DELTA_STEP_MS);
                }

                if (CheckGameOver(&level->world, &level->player))
                {
                    SetLevelGameOver(level);
                    // Mix_HaltMusic();
                    // Mix_PlayChannel(SOUND_CHANNEL_SFX, as->assets.gameOverMusic, 0);

                    if (input->gamepadId)
                    {
                        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(input->gamepadId);

                        if (gamepad)
                        {
                            SDL_RumbleGamepad(gamepad, 0xFFFF, 0xFFFF, 1000);
                            SDL_SetGamepadLED(gamepad, 0xFF, 0x00, 0x00);
                        }
                    }
                }
            }
        }
    }
}

void RenderLevel(SDL_Renderer *renderer, app_assets_t *assets, level_t *level, vec2i_t renderSize)
{
    world_t *world = &level->world;
    player_t *player = &level->player;

    vec2_t itemSize{40.0f, 40.0f};
    vec2_t gridSize{itemSize.w * world->size.x, itemSize.h * world->size.y};
    vec2_t offset{((real32)renderSize.w - gridSize.w - 20.0f) / 2.0f,
                  ((real32)renderSize.h - gridSize.h - 20.0f) / 2.0f};

    SDL_FRect gridRect{
        offset.x,
        offset.y,
        gridSize.w,
        gridSize.h};

    SDL_FRect blockTextureSrcRect{
        45.0f,
        45.0f,
        30.0f,
        30.0f};

    real32 gridScale = itemSize.w / 30.0f;
    SDL_RenderTextureTiled(renderer, assets->gridPatternTexture, nullptr, gridScale, &gridRect);

    RenderWorld(renderer, assets, world, offset);
    RenderPlayer(renderer, assets, world, player, offset);

    player_data_t nextPlayerKind = GetPlayerKind(player->nextPlayerKindId);
    vec2_t nextPlayerSize{
        itemSize.w * nextPlayerKind.dim.x,
        itemSize.h * nextPlayerKind.dim.y};
    vec2_t nextPlayerOffset{
        (real32)renderSize.w - offset.x + (offset.x - nextPlayerSize.w) / 2.0f,
        ((real32)renderSize.h - nextPlayerSize.h) / 2.0f,
    };
    RenderPlayer(renderer, assets, world, &nextPlayerKind, vec2i_t{0, 0}, player->nextPlayerValue, nextPlayerOffset);

    real32 gridBorderSize = 16.0f;

    SDL_FRect gridBorderDestRect{
        gridRect.x - gridBorderSize,
        gridRect.y - gridBorderSize,
        gridRect.w + gridBorderSize * 2.0f,
        gridRect.h + gridBorderSize * 2.0f};

    SDL_FRect gridBorderSrcRect{35.0f,
                                85.0f,
                                330.0f,
                                630.0f};

    SDL_RenderTexture9Grid(renderer, assets->borderTexture, &gridBorderSrcRect,
                           gridBorderSize, gridBorderSize, gridBorderSize, gridBorderSize,
                           1.0f, &gridBorderDestRect);

    if (level->gameOver)
    {
        RenderLevelOverlay(renderer, renderSize, "GAME OVER\nPRESS R TO RESTART");
    }
    else if (level->paused)
    {
        RenderLevelOverlay(renderer, renderSize, "PAUSE\nPRESS P TO RESUME\nPRESS R TO RESTART");
    }

    RenderScore(renderer, renderSize, level->score);
}

void RenderLevelOverlay(SDL_Renderer *renderer, vec2i_t renderSize, char *message)
{
    const real32 scale = 4.0f;
    const real32 x = ((renderSize.w / 4.0f) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    const real32 y = ((renderSize.h / 4.0f) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xA0);
    SDL_RenderFillRect(renderer, nullptr);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDebugText(renderer, x, y, message);
}

void RenderScore(SDL_Renderer *renderer, vec2i_t renderSize, uint32 score)
{
    char scoreString[256];
    SDL_snprintf(scoreString, 256, "Score: %d", score);
    const real32 scale = 2.0f;
    const real32 x = ((renderSize.w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(scoreString)) - 16.0f;

    SDL_SetRenderScale(renderer, scale, scale);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDebugText(renderer, x, 16.0f, scoreString);
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}