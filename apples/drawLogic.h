#pragma once

#include "myD2D.h"
#include "gameState.h"

namespace drawLogic {
    void init(const MyD2DObjectCollection& myd2d, rtd rtdv);
    void drawFrame(const MyD2DObjectCollection& myd2d, const gamestate::GameState& gameState);
    void free(rtd rtdv);
} // namespace drawLogic
