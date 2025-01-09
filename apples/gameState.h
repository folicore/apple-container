#pragma once

namespace gamestate {
    const FLOAT LOGICAL_WINDOW_SIZE_X = 1920.0f;
    const FLOAT LOGICAL_WINDOW_SIZE_Y = 1080.0f;

    struct GameState {
        FLOAT graphicalScale;
        FLOAT graphicalOffsetX;
        FLOAT grpahicalOffsetY;

        FLOAT logicalMouseX;
        FLOAT logicalMouseY;

        enum class Mode {
            MAIN_MENU,
            HELP_MENU,
            PLAYING
        };
        Mode mode;
    };
} // namespace gamestate