#if !defined(TETRIS_LEVEL_H)

#include "tetris_typedefs.h"
#include "tetris_world.h"
#include "tetris_player.h"
#include "tetris_input.h"
#include "tetris_assets.h"

#define SCORE_PER_ROW 100
#define MIN_STEP_MS 50
#define MAX_STEP_MS 500
#define DELTA_STEP_MS 25

struct level_t
{
    world_t world;
    player_t player;
    bool paused;
    bool gameOver;
    uint32 score;

    uint64 stepMs;
    uint64 currentStepMs;
    uint64 stepAccumulator;
};

void ResumeLevel(level_t *level);

void PauseLevel(level_t *level);

void ToggleLevelPaused(level_t *level);

void SetLevelGameOver(level_t *level);

void MovePlayer(world_t *world, player_t *player, game_input_t *input);

bool CheckGameOver(world_t *world, player_t *player);

uint8 DestroyFilledRows(world_t *world);

void ResetLevel(level_t *level);

void Restart(level_t *level);

void ApplyLevelInput(level_t *level, uint64 dt, game_input_t *input);

void DoLevelStep(level_t *level, game_input_t *input, uint64 dt);

void RenderLevel(SDL_Renderer *renderer, app_assets_t *assets, level_t *level, vec2i_t renderSize);

void RenderLevelOverlay(SDL_Renderer *renderer, vec2i_t renderSize, char *message);

void RenderScore(SDL_Renderer *renderer, vec2i_t renderSize, uint32 score);

#define TETRIS_LEVEL_H
#endif