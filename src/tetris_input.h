#if !defined(TETRIS_INPUT_H)

#include <SDL3\SDL.h>
#include "tetris_typedefs.h"

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

void SetInputButtonDown(game_input_button_t *button, bool isDown);

uint8 GetInputButtonDownCount(const game_input_button_t *button);

bool WasInputButtonPressedOnce(const game_input_button_t *button);

void FlushInput(game_input_t *input);

#define TETRIS_INPUT_H
#endif