#pragma once

#include<string>
#include<vector>
#include<d2d1.h>

namespace gamestate {
    const FLOAT LOGICAL_WINDOW_SIZE_X = 1920.0f;
    const FLOAT LOGICAL_WINDOW_SIZE_Y = 1080.0f;

    const D2D1_RECT_F APPLES_PLAY_AREA = {
        .left = 400.0f,
        .top = 115.0f,
        .right = 1840.0f,
        .bottom = 955.0f,
    };

    struct Apple {
        INT value;
        bool popped = false;
        bool inDrag = false;

        FLOAT posX;
        FLOAT posY;
        FLOAT angle = false;

        FLOAT velX;
        FLOAT velY;
        FLOAT accY;
        FLOAT velAngular;

        Apple(INT value, FLOAT posX, FLOAT posY);
        void pop() { popped = true; }
        void animate(UINT64 deltaTimeMs);
    };

    struct GameState {
        UINT64 previousTimeMs;
        UINT64 currentTimeMs;

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

        INT appleCountX;
        INT appleCountY;
        INT playTime;
        FLOAT appleSize;

        struct SingletonPlay {
            INT score;
            UINT64 startTimeMs;
            std::vector<std::vector<Apple>> apples;
            bool inDrag;
            float dragStartX;
            float dragStartY;
        };
        SingletonPlay play;
    };


    struct Button {
        std::wstring text;
        FLOAT left, top, right, bottom;

        bool hoverOver(FLOAT mouseX, FLOAT mouseY) const {
            return mouseX >= left && mouseX <= right &&
                mouseY >= top && mouseY <= bottom;
        }
    };

    const Button buttonMainMenuStart = {
        .text = L"Start",
        .left = 720.0f,
        .top = 560.0f,
        .right = 1200.0f,
        .bottom = 700.0f,
    };

    const Button buttonMainMenuHelp = {
        .text = L"Help",
        .left = 720.0f,
        .top = 800.0f,
        .right = 1200.0f,
        .bottom = 940.0f,
    };

    const Button buttonHelpMenuBack = {
        .text = L"Back",
        .left = buttonMainMenuHelp.left,
        .top = buttonMainMenuHelp.top,
        .right = buttonMainMenuHelp.right,
        .bottom = buttonMainMenuHelp.bottom,
    };
} // namespace gamestate