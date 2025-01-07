#include "gameLogic.h"

#include "helper.h"

namespace {
    INT DEFAULT_APPLES_X = 17, DEFAULT_APPLES_Y = 10;

    UINT64 lastTimeMs;

    enum gs {
        MAIN_MENU,
        HELP_MENU,
        PLAYING
    };
    gs gameState;

} // namespace

void gameLogic::init(UINT64 timeMs) {
    lastTimeMs = timeMs;
    gameState = gs::MAIN_MENU;
}

void gameLogic::processFrame(const Controller& controller, DrawOrders& drawOrders, UINT64 timeMs) {
    // assume the game plays in an 1920x1080 window
    Controller::PairXY<INT> windowSize = controller.windowSize();
    FLOAT windowRatio = static_cast<FLOAT>(windowSize.x) / static_cast<FLOAT>(windowSize.y);
    FLOAT scaleRatio = (windowRatio > 16.0f / 9.0f) ? (windowSize.y / 1080.0f) : (windowSize.x / 1920.0f);

    drawOrders.finalScaleFactor = scaleRatio;
    drawOrders.finalOffsetX = (windowSize.x - 1920 * scaleRatio) / 2.0f;
    drawOrders.finalOffsetY = (windowSize.y - 1080 * scaleRatio) / 2.0f;

    drawOrders.drawQueue.push(new DrawableBorder());

    Controller::PairXY<INT> mousePos = controller.mousePos();
    Controller::PairXY<FLOAT> logicalMousePos(0.0f, 0.0f);
    logicalMousePos.x = (mousePos.x - windowSize.x / 2.0f) / scaleRatio + 960.0f;
    logicalMousePos.y = (mousePos.y - windowSize.y / 2.0f) / scaleRatio + 540.0f;
    
    
    if (logicalMousePos.x < 0.0f || logicalMousePos.x > 1920.0f || logicalMousePos.y < 0.0f || logicalMousePos.y > 1080.0f) {drawOrders.drawQueue.pop();}
}

void gameLogic::free() {

}