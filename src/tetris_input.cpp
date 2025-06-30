#include "tetris_input.h"

void SetInputButtonDown(game_input_button_t *button, bool isDown)
{
    if (button->isDown != isDown)
    {
        button->isDown = isDown;
        button->transitionCount++;
    }
}

inline uint8 GetInputButtonDownCount(const game_input_button_t *button)
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

inline bool WasInputButtonPressedOnce(const game_input_button_t *button)
{
    return button->isDown && button->transitionCount == 1 ||
           !button->isDown && button->transitionCount > 1;
}

void FlushInput(game_input_t *input)
{
    input->left.transitionCount = 0;
    input->right.transitionCount = 0;
    input->down.transitionCount = 0;
    input->rotate.transitionCount = 0;
}