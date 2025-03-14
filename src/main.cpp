#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "tetris_typedefs.h"
#include "tetris_math.h"
#include "tetris_world.h"
#include "tetris_player.h"

#define SCORE_PER_ROW 100
#define MIN_STEP_MS 50
#define MAX_STEP_MS 500
#define DELTA_STEP_MS 25

static constexpr uint64 kWidth = 1920;
static constexpr uint64 kHeight = 1080;
static constexpr uint64 inputMs = 100;

static uint64 fpsTimer{0};
static uint64 inputTimer{0};
static uint64 playerTimer{0};

struct game_input_button_t
{
    bool isDown;
    uint8 transitionCount;
};

struct game_input_t
{
    SDL_JoystickID gamepadId;

    union
    {
        struct
        {
            game_input_button_t left;
            game_input_button_t right;
            game_input_button_t down;
            game_input_button_t rotate;
        };

        game_input_button_t buttons[4];
    };
};

struct app_state_t
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    world_t world;
    player_t player;
    game_input_t input;
    bool paused;
    bool gameOver;
    uint32 score;
    uint64 stepMs;
};

inline color_t GetValueColor(uint8 value)
{
    switch (value)
    {
    case 1:
    {
        return {0xFF, 0x00, 0x00};
        break;
    }
    case 2:
    {
        return {0x00, 0xFF, 0x00};
        break;
    }
    case 3:
    {
        return {0x00, 0x00, 0xFF};
        break;
    }
    default:
        SDL_assert(false);
        return {};
    }
}

static void SetInputButtonDown(game_input_button_t *button, bool isDown)
{
    if (button->isDown != isDown)
    {
        button->isDown = isDown;
        button->transitionCount++;
    }
}

static inline uint8 GetInputButtonDownCount(const game_input_button_t *button)
{
    uint8 result = 0;

    if (button->transitionCount)
    {
        if (button->transitionCount % 2 == 0)
        {
            result = button->transitionCount / 2;
        }
        else
        {
            if (button->isDown)
            {
                result = (uint8)(SDL_ceilf((real32)button->transitionCount / 2.0f));
            }
            else
            {
                result = (uint8)(SDL_floorf((real32)button->transitionCount / 2.0f));
            }
        }
    }
    else if (button->isDown)
    {
        result = 1;
    }

    return result;
}

static inline bool WasInputButtonPressedOnce(const game_input_button_t *button)
{
    return button->isDown && button->transitionCount == 1 ||
           !button->isDown && button->transitionCount > 1;
}

static void FlushInput(game_input_t *input)
{
    input->left.transitionCount = 0;
    input->right.transitionCount = 0;
    input->down.transitionCount = 0;
    input->rotate.transitionCount = 0;
}

static void MovePlayer(world_t *world, player_t *player, game_input_t *input)
{
    vec2i_t newPosition = player->position;
    newPosition.x -= GetInputButtonDownCount(&input->left);
    newPosition.x += GetInputButtonDownCount(&input->right);

    if (IsPlayerPositionValid(world, &player->data, newPosition))
    {
        player->position = newPosition;
    }

    if (WasInputButtonPressedOnce(&input->rotate))
    {
        RotatePlayer(world, player);
    }
}

static bool CheckGameOver(world_t *world, player_t *player)
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

static uint8 DestroyFilledRows(world_t *world)
{
    uint8 destroyedRows = 0;

    for (int8 y = world->size.y - 1; y >= 0; --y)
    {
        if (IsWorldRowFilled(world, y))
        {
            destroyedRows++;

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

static void ResetAppState(app_state_t *appState)
{
    appState->paused = false;
    appState->gameOver = false;
    appState->score = 0;
    appState->stepMs = MAX_STEP_MS;
}

static void Restart(app_state_t *appState)
{
    ResetAppState(appState);
    ResetWorld(&appState->world);
    SpawnPlayer(&appState->world, &appState->player);

    if (appState->input.gamepadId)
    {
        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(appState->input.gamepadId);

        if (gamepad)
        {
            SDL_SetGamepadLED(gamepad, 0x00, 0x00, 0xFF);
        }
    }
}

static void HandleKeyboardEvent(app_state_t *appState, SDL_Scancode scancode, bool isDown)
{
    game_input_t *input = &appState->input;
    input->gamepadId = 0;

    if (isDown)
    {
        switch (scancode)
        {
        case SDL_SCANCODE_R:
            Restart(appState);
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_P:
            if (!appState->gameOver)
            {
                appState->paused = !appState->paused;
            }
            break;
        }
    }

    if (!appState->paused && !appState->gameOver)
    {
        switch (scancode)
        {
        case SDL_SCANCODE_LEFT:
            SetInputButtonDown(&input->left, isDown);
            break;
        case SDL_SCANCODE_RIGHT:
            SetInputButtonDown(&input->right, isDown);
            break;
        case SDL_SCANCODE_DOWN:
            SetInputButtonDown(&input->down, isDown);
            break;
        case SDL_SCANCODE_SPACE:
            SetInputButtonDown(&input->rotate, isDown);
            break;
        }
    }
}

static void HandleGamepadButtonEvent(app_state_t *appState, uint8 button, SDL_JoystickID gamepadId, bool isDown)
{
    game_input_t *input = &appState->input;
    input->gamepadId = gamepadId;

    if (isDown)
    {
        switch (button)
        {
        case SDL_GAMEPAD_BUTTON_START:
            if (!appState->gameOver)
            {
                appState->paused = !appState->paused;
            }
            else
            {
                Restart(appState);
            }
            break;
        case SDL_GAMEPAD_BUTTON_BACK:
            Restart(appState);
            break;
        }
    }

    if (!appState->paused && !appState->gameOver)
    {
        switch (button)
        {
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
        case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:
        case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:
            SetInputButtonDown(&input->left, isDown);
            break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
        case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:
        case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:
            SetInputButtonDown(&input->right, isDown);
            break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
            SetInputButtonDown(&input->down, isDown);
            break;
        case SDL_GAMEPAD_BUTTON_SOUTH:
            SetInputButtonDown(&input->rotate, isDown);
            break;
        }
    }
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "Tetris");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, "1.0.0");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, "ru.holzez.tetris");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, "Holzez");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "Copyright (c) 2025 Holzez");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        SDL_Log("Couldn't init sdl: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    app_state_t *as = (app_state_t *)SDL_calloc(1, sizeof(app_state_t));

    if (!as)
    {
        SDL_Log("Couldn't allocate memory for app state: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *appstate = as;

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", kWidth, kHeight, 0, &as->window, &as->renderer))
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderVSync(as->renderer, false))
    {
        SDL_Log("Set vsync error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_srand(1);

    ResetAppState(as);
    as->world.size = {16, 24};
    as->world.data = (uint8 *)SDL_calloc(as->world.size.x * as->world.size.y, sizeof(uint8));

    if (!as->world.data)
    {
        SDL_Log("Couldn't allocate memory for world data: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    as->player.nextPlayerKindId = SDL_rand(PLAYER_DATA_KIND_COUNT);
    as->player.nextPlayerValue = SDL_rand(PLAYER_VALUE_COUNT) + 1;
    as->player.data.grid = (uint8 *)SDL_calloc(PLAYER_DATA_GRID_MAX_SIZE * PLAYER_DATA_GRID_MAX_SIZE, sizeof(uint8));

    if (!as->player.data.grid)
    {
        SDL_Log("Couldn't allocate memory for player data: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SpawnPlayer(&as->world, &as->player);

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    app_state_t *as = (app_state_t *)appstate;

    switch (event->type)
    {
    case SDL_WINDOW_HIDDEN:
        if (as->gameOver)
        {
            as->paused = true;
        }
        break;
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
        switch (event->key.scancode)
        {
        case SDL_SCANCODE_Q:
            return SDL_APP_SUCCESS;
        default:
            HandleKeyboardEvent(as, event->key.scancode, true);
            break;
        }
        break;
    case SDL_EVENT_KEY_UP:
        HandleKeyboardEvent(as, event->key.scancode, false);
        break;
    case SDL_EVENT_GAMEPAD_ADDED:
    {
        const SDL_JoystickID gamepadId = event->gdevice.which;
        SDL_Gamepad *gamepad = SDL_OpenGamepad(gamepadId);

        if (gamepad)
        {
            SDL_SetGamepadLED(gamepad, 0x00, 0x00, 0xFF);
        }
        break;
    }
    case SDL_EVENT_GAMEPAD_REMOVED:
    {
        const SDL_JoystickID gamepadId = event->gdevice.which;
        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(gamepadId);

        if (gamepad)
        {
            SDL_CloseGamepad(gamepad);
        }

        break;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        HandleGamepadButtonEvent(as, event->gbutton.button, event->gdevice.which, true);
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        HandleGamepadButtonEvent(as, event->gbutton.button, event->gdevice.which, false);
        break;
    }

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_state_t *as = (app_state_t *)appstate;
    world_t *world = &as->world;
    player_t *player = &as->player;

    char fpsString[256];
    vec2i_t renderSize;
    Uint64 fps = 0;
    Uint64 nsPerFrame = 0;
    real32 textPaddingX = 16.0f;
    real32 textPaddingY = 16.0f;

    Uint64 lastFrameNS = fpsTimer;
    fpsTimer = SDL_GetTicksNS();

    if (lastFrameNS > 0)
    {
        nsPerFrame = fpsTimer - lastFrameNS;
        fps = 1000000000 / nsPerFrame;
        SDL_snprintf(fpsString, 256, "FPS %d", fps);
    }

    /* Draw the message */
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(as->renderer);

    SDL_GetCurrentRenderOutputSize(as->renderer, &renderSize.w, &renderSize.h);

    // UPDATE WORLD
    {
        uint64 now = SDL_GetTicks();
        uint64 stepMs = as->input.down.isDown ? MIN_STEP_MS : as->stepMs;

        for (int i = 0; i < 4; ++i)
        {
            game_input_button_t *button = &as->input.buttons[i];
            SDL_FRect rect{
                textPaddingX + i * (textPaddingX + 20.0f),
                textPaddingY * 4 + SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE,
                20.0f,
                20.0f,
            };

            // DEBUG BUTTONS
            if (WasInputButtonPressedOnce(button) || button->isDown)
            {
                SDL_SetRenderDrawColor(as->renderer, 0x00, 0xFF, 0x00, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(as->renderer, 0xFF, 0x00, 0x00, 0xFF);
            }

            SDL_RenderFillRect(as->renderer, &rect);
        }

        if (now - inputTimer >= inputMs)
        {
            if (!as->paused && !as->gameOver)
            {
                MovePlayer(world, player, &as->input);
            }

            FlushInput(&as->input);
            inputTimer = now;
        }

        if (now - playerTimer >= stepMs)
        {
            if (!as->paused && !as->gameOver)
            {
                vec2i_t newPosition = player->position + vec2i_t{0, 1};

                if (IsPlayerPositionValid(world, &player->data, newPosition))
                {
                    player->position = newPosition;
                }
                else
                {
                    SavePlayerInWorld(world, player);
                    uint8 destroyedRows = DestroyFilledRows(world);
                    as->score += destroyedRows * SCORE_PER_ROW;
                    SpawnPlayer(world, player);

                    if (as->input.gamepadId)
                    {
                        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(as->input.gamepadId);

                        if (gamepad)
                        {
                            uint16 strength = destroyedRows ? 0xFFFF : 0x1000;
                            SDL_RumbleGamepad(gamepad, strength, strength, 500);
                        }
                    }

                    if (destroyedRows)
                    {
                        as->stepMs = SDL_max(MIN_STEP_MS, as->stepMs - DELTA_STEP_MS);
                    }

                    if (CheckGameOver(world, player))
                    {
                        as->gameOver = true;

                        if (as->input.gamepadId)
                        {
                            SDL_Gamepad *gamepad = SDL_GetGamepadFromID(as->input.gamepadId);

                            if (gamepad)
                            {
                                SDL_RumbleGamepad(gamepad, 0xFFFF, 0xFFFF, 1000);
                                SDL_SetGamepadLED(gamepad, 0xFF, 0x00, 0x00);
                            }
                        }
                    }
                }
            }

            playerTimer = now;
        }
    }

    // RENDER WORLD
    {
        vec2_t itemSize{40.0f, 40.0f};
        vec2_t gridSize{itemSize.w * world->size.x, itemSize.h * world->size.y};
        vec2_t offset{((real32)renderSize.w - gridSize.w - 20.0f) / 2.0f,
                      ((real32)renderSize.h - gridSize.h - 20.0f) / 2.0f};

        for (int itemY = 0; itemY < world->size.y; ++itemY)
        {
            for (int itemX = 0; itemX < world->size.x; ++itemX)
            {
                uint8 value = world->data[itemY * world->size.x + itemX];

                if (value)
                {
                    color_t color = GetValueColor(value);

                    SDL_FRect rect{
                        offset.x + itemX * itemSize.w,
                        offset.y + itemY * itemSize.h,
                        itemSize.w,
                        itemSize.h};

                    SDL_SetRenderDrawColor(as->renderer, color.r, color.g, color.b, 0xFF);
                    SDL_RenderFillRect(as->renderer, &rect);
                    SDL_SetRenderDrawColor(as->renderer, 0xAA, 0xAA, 0xAA, 0x33);
                    SDL_RenderRect(as->renderer, &rect);
                }
            }
        }

        if (player->value)
        {
            color_t playerColor = GetValueColor(player->value);
            real32 playerStartX = offset.x + player->position.x * itemSize.w;
            real32 playerStartY = offset.y + player->position.y * itemSize.h;

            for (uint8 y = 0; y < player->data.dim.y; ++y)
            {
                for (uint8 x = 0; x < player->data.dim.x; ++x)
                {
                    uint8 hasValue = player->data.grid[y * player->data.dim.x + x];

                    if (hasValue && player->position.y + y >= 0)
                    {
                        SDL_FRect rect{
                            playerStartX + x * itemSize.w,
                            playerStartY + y * itemSize.h,
                            itemSize.w,
                            itemSize.h};

                        SDL_SetRenderDrawColor(as->renderer, playerColor.r, playerColor.g, playerColor.b, 0xFF);
                        SDL_RenderFillRect(as->renderer, &rect);
                        SDL_SetRenderDrawColor(as->renderer, 0xAA, 0xAA, 0xAA, 0x33);
                        SDL_RenderRect(as->renderer, &rect);
                    }
                }
            }
        }

        {
            player_data_t nextPlayerKind = GetPlayerKind(player->nextPlayerKindId);

            if (nextPlayerKind.grid)
            {
                vec2_t nextPlayerSize{
                    itemSize.w * nextPlayerKind.dim.x,
                    itemSize.h * nextPlayerKind.dim.y};
                vec2_t nextPlayerOffset{
                    (real32)renderSize.w - offset.x + (offset.x - nextPlayerSize.w) / 2.0f,
                    ((real32)renderSize.h - nextPlayerSize.h) / 2.0f,
                };

                for (int y = 0; y < nextPlayerKind.dim.y; ++y)
                {
                    for (int x = 0; x < nextPlayerKind.dim.x; ++x)
                    {
                        uint8 hasValue = nextPlayerKind.grid[y * nextPlayerKind.dim.x + x];

                        if (hasValue)
                        {
                            color_t color = GetValueColor(player->nextPlayerValue);
                            SDL_FRect rect{
                                nextPlayerOffset.x + x * itemSize.w,
                                nextPlayerOffset.y + y * itemSize.h,
                                itemSize.w,
                                itemSize.h};

                            SDL_SetRenderDrawColor(as->renderer, color.r, color.g, color.b, 0xFF);
                            SDL_RenderFillRect(as->renderer, &rect);
                            SDL_SetRenderDrawColor(as->renderer, 0xAA, 0xAA, 0xAA, 0x33);
                            SDL_RenderRect(as->renderer, &rect);
                        }
                    }
                }

                SDL_FRect nextPlayerRect{
                    nextPlayerOffset.x,
                    nextPlayerOffset.y,
                    nextPlayerSize.w,
                    nextPlayerSize.h,
                };

                SDL_SetRenderDrawColor(as->renderer, 0xAA, 0xAA, 0xAA, 0xFF);
                SDL_RenderRect(as->renderer, &nextPlayerRect);
            }
        }

        SDL_SetRenderDrawColor(as->renderer, 0xAA, 0xAA, 0xAA, 0xFF);
        SDL_FRect gridRect{
            offset.x,
            offset.y,
            gridSize.w,
            gridSize.h};

        SDL_RenderRect(as->renderer, &gridRect);
    }

    if (as->paused)
    {
        const char *message = "PAUSE";
        const real32 scale = 4.0f;
        const real32 x = ((renderSize.w / 4.0f) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
        const real32 y = ((renderSize.h / 4.0f) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

        SDL_SetRenderScale(as->renderer, scale, scale);
        SDL_SetRenderDrawColor(as->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDebugText(as->renderer, x, y, message);
    }

    if (as->gameOver)
    {
        const char *message = "GAME OVER";
        const real32 scale = 4.0f;
        const real32 x = ((renderSize.w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
        const real32 y = ((renderSize.h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

        SDL_SetRenderScale(as->renderer, scale, scale);
        SDL_SetRenderDrawColor(as->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDebugText(as->renderer, x, y, message);
        SDL_SetRenderScale(as->renderer, 1.0f, 1.0f);
    }

    {
        char scoreString[256];
        SDL_snprintf(scoreString, 256, "Score: %d", as->score);
        const real32 scale = 2.0f;
        const real32 x = ((renderSize.w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(scoreString)) - textPaddingX;

        SDL_SetRenderScale(as->renderer, scale, scale);
        SDL_SetRenderDrawColor(as->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDebugText(as->renderer, x, textPaddingY, scoreString);
        SDL_SetRenderScale(as->renderer, 1.0f, 1.0f);
    }

#if 0
    {
        SDL_SetRenderScale(as->renderer, 2.0f, 2.0f);
        SDL_SetRenderDrawColor(as->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDebugText(as->renderer, textPaddingX, textPaddingY, fpsString);
        SDL_SetRenderScale(as->renderer, 1.0f, 1.0f);
    }
#endif

    SDL_RenderPresent(as->renderer);

    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    int gamepadsCount;
    SDL_JoystickID *gamepads = SDL_GetGamepads(&gamepadsCount);

    for (int i = 0; i < gamepadsCount; ++i)
    {
        SDL_Gamepad *gamepad = SDL_GetGamepadFromID(gamepads[i]);

        if (gamepad)
        {
            SDL_CloseGamepad(gamepad);
        }
    }

    if (appstate != nullptr)
    {
        app_state_t *as = (app_state_t *)appstate;
        if (as->world.data != nullptr)
        {
            SDL_free(as->world.data);
        }
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);
        SDL_Quit();
    }
}