#include "gameLogic.h"

#include "helper.h"

using gamestate::GameState;

namespace {
    const INT DEFAULT_APPLES_X = 17;
    const INT DEFAULT_APPLES_Y = 10;

    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs);
    void helpMenu(GameState& gameState, const Controller& controller, UINT64 timeMs);
    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs);

} // namespace

void gameLogic::init(UINT64 timeMs, GameState& gameState) {
    gameState.mode = GameState::Mode::MAIN_MENU;
}

void gameLogic::processFrame(const Controller& controller, GameState& gameState, UINT64 timeMs) {
    // assume the game plays in an 1920x1080 window
    Controller::PairXY<INT> windowSize = controller.windowSize();
    FLOAT windowRatio = static_cast<FLOAT>(windowSize.x) / static_cast<FLOAT>(windowSize.y);
    gameState.graphicalScale = (windowRatio > gamestate::LOGICAL_WINDOW_SIZE_X / gamestate::LOGICAL_WINDOW_SIZE_Y) ?
        (windowSize.y / gamestate::LOGICAL_WINDOW_SIZE_Y) : (windowSize.x / gamestate::LOGICAL_WINDOW_SIZE_X);

    gameState.graphicalOffsetX = (windowSize.x - 1920 * gameState.graphicalScale) / 2.0f;
    gameState.grpahicalOffsetY = (windowSize.y - 1080 * gameState.graphicalScale) / 2.0f;

    Controller::PairXY<INT> mousePos = controller.mousePos();
    gameState.logicalMouseX = (mousePos.x - windowSize.x / 2.0f) / gameState.graphicalScale + gamestate::LOGICAL_WINDOW_SIZE_X / 2.0f;
    gameState.logicalMouseY = (mousePos.y - windowSize.y / 2.0f) / gameState.graphicalScale + gamestate::LOGICAL_WINDOW_SIZE_Y / 2.0f;
}

namespace {
    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs) {
        
    }

    void helpMenu(GameState& gameState, const Controller& controller, UINT64 timeMs) {

    }

    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs) {

    }
}

void gameLogic::free() {

}