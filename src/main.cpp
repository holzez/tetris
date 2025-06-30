#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "tetris_typedefs.h"
#include "tetris_math.h"
#include "tetris_world.cpp"
#include "tetris_player.cpp"
#include "tetris_fx.h"
#include "tetris_input.cpp"
#include "tetris_assets.cpp"
#include "tetris_level.cpp"

static constexpr uint64 kWidth = 1920;
static constexpr uint64 kHeight = 1080;
static constexpr uint64 inputMs = 100;

static uint64 fpsTimer{0};
static uint64 lastTickMs{0};

enum eSoundChannels
{
    SOUND_CHANNEL_MUSIC = 0,
    SOUND_CHANNEL_SFX,
    SOUND_CHANNEL_COUNT,
};

struct app_state_t
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AudioDeviceID audioDeviceId;
    game_input_t input;
    level_t level;

    fx_pool_t cleanFxPool;

    app_assets_t assets;
};

static void HandleKeyboardEvent(app_state_t *appState, SDL_Scancode scancode, bool isDown)
{
    game_input_t *input = &appState->input;
    input->gamepadId = 0;

    if (isDown)
    {
        switch (scancode)
        {
        case SDL_SCANCODE_R:
            ResetLevel(&appState->level);
            break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_P:
            ToggleLevelPaused(&appState->level);
            break;
        }
    }

    switch (scancode)
    {
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
        SetInputButtonDown(&input->left, isDown);
        break;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
        SetInputButtonDown(&input->right, isDown);
        break;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_S:
        SetInputButtonDown(&input->down, isDown);
        break;
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_W:
    case SDL_SCANCODE_SPACE:
        SetInputButtonDown(&input->rotate, isDown);
        break;
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
            ToggleLevelPaused(&appState->level);
            break;
        case SDL_GAMEPAD_BUTTON_BACK:
            ResetLevel(&appState->level);
            break;
        }
    }

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
    case SDL_GAMEPAD_BUTTON_DPAD_UP:
        SetInputButtonDown(&input->rotate, isDown);
        break;
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

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO))
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

    if (!Mix_Init(MIX_INIT_MP3))
    {
        SDL_Log("Couldn't init mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec audioSpec;
    SDL_zero(audioSpec);
    audioSpec.format = SDL_AUDIO_F32;
    audioSpec.channels = 2;
    audioSpec.freq = 44100;

    as->audioDeviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec);
    if (!as->audioDeviceId)
    {
        SDL_Log("Couldn't open audio device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!Mix_OpenAudio(as->audioDeviceId, nullptr))
    {
        SDL_Log("Couldn't open mixer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (Mix_AllocateChannels(SOUND_CHANNEL_COUNT) != SOUND_CHANNEL_COUNT)
    {
        SDL_Log("Couldn't allocate channels: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_srand(1);

    ResetLevel(&as->level);

    if (!InitWorld(&as->level.world))
    {
        SDL_Log("Couldn't init world: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!InitPlayer(&as->level.player))
    {
        SDL_Log("Couldn't init player: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!LoadAssets(as->renderer, &as->assets))
    {
        SDL_Log("Couldn't load assets: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SpawnPlayer(&as->level.world, &as->level.player);
    lastTickMs = SDL_GetTicks();
    // Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    // Mix_PlayMusic(as->assets.bgMusic, -1);

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    app_state_t *as = (app_state_t *)appstate;

    switch (event->type)
    {
    case SDL_WINDOW_HIDDEN:
        PauseLevel(&as->level);
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
    level_t *level = &as->level;

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
    SDL_RenderTextureTiled(as->renderer, as->assets.bgPatternTexture, nullptr, 2.0f, nullptr);

    SDL_GetCurrentRenderOutputSize(as->renderer, &renderSize.w, &renderSize.h);

    uint64 now = SDL_GetTicks();
    uint64 dt = now - lastTickMs;
    ApplyLevelInput(level, dt, &as->input);
    FlushInput(&as->input);
    DoLevelStep(level, &as->input, dt);
    lastTickMs = SDL_GetTicks();
    RenderLevel(as->renderer, &as->assets, level, renderSize);

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
        if (as->level.world.data != nullptr)
        {
            SDL_free(as->level.world.data);
        }
        FreeAssets(&as->assets);

        Mix_CloseAudio();
        SDL_CloseAudioDevice(as->audioDeviceId);

        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);

        Mix_Quit();
        SDL_Quit();
    }
}