#pragma once

#include "controller.h"
#include "gameState.h"

namespace gameLogic {
    void init(UINT64 timeMs, gamestate::GameState& gameState);
    bool processFrame(const Controller& controller, gamestate::GameState& gameState, UINT64 timeMs);
    void free();
} // namespaace gameLogic
