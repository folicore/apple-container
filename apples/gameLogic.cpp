#include "gameLogic.h"

#include<random>
#include "helper.h"

using gamestate::GameState;
using gamestate::Apple;

namespace {
    const INT DEFAULT_APPLES_X = 17;
    const INT DEFAULT_APPLES_Y = 10;
    const INT DEFAULT_PLAY_TIME_SECONDS = 120;


    // random floats:
    std::mt19937_64 rng;
    FLOAT randomFloat(FLOAT v_min, FLOAT v_max) {
        const INT v = 0x8'0000;
        std::uniform_int_distribution unidist(0, v);
        INT rng_result = unidist(rng);
        FLOAT result = static_cast<FLOAT>(rng_result) / static_cast<FLOAT>(v);
        return result * (v_max - v_min) + v_min;
    }
    FLOAT randomFloat(FLOAT v_max = 1.0f) {
        return randomFloat(0.0f, v_max);
    }


    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs);
    void helpMenu(GameState& gameState, const Controller& controller);
    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs);

} // namespace

void gameLogic::init(UINT64 timeMs, GameState& gameState) {
    rng.seed(timeMs);

    gameState.mode = GameState::Mode::MAIN_MENU;

    gameState.appleCountX = DEFAULT_APPLES_X;
    gameState.appleCountY = DEFAULT_APPLES_Y;
    gameState.playTime = DEFAULT_PLAY_TIME_SECONDS;

    gameState.previousTimeMs = timeMs;
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
    
    switch (gameState.mode) {
    case GameState::Mode::MAIN_MENU:
        mainMenu(gameState, controller, timeMs);
        break;

    case GameState::Mode::HELP_MENU:
        helpMenu(gameState, controller);
        break;

    case GameState::Mode::PLAYING:
        playing(gameState, controller, timeMs);
        break;
    }

    gameState.previousTimeMs = timeMs;
}

gamestate::Apple::Apple(INT value, FLOAT posX, FLOAT posY) {
    this->value = value;

    this->posX = posX;
    this->posY = posY;

    velX = randomFloat(-247.1f, 247.1f);
    velY = randomFloat(-643.1f, -656.9f);
    accY = randomFloat(1261.2f, 1395.3f);
    velAngular = randomFloat(-82.8f, 82.8f);
}

void gamestate::Apple::animate(UINT64 deltaTimeMs) {
    if (!popped) { return; }
    if (posY > 1500.0f) { return; }

    FLOAT deltaTimeSec = deltaTimeMs / 1000.0f;

    posX += velX * deltaTimeSec;

    posY += velY * deltaTimeSec;
    velY += accY * deltaTimeSec;

    angle += velAngular * deltaTimeSec;
}

namespace {
    void initPlaying(GameState& gameState, UINT64 timeMs) {
        gameState.play.startTime = timeMs;

        FLOAT playAreaSizeX = gamestate::APPLES_PLAY_AREA.right - gamestate::APPLES_PLAY_AREA.left;
        FLOAT playAreaSizeY = gamestate::APPLES_PLAY_AREA.bottom - gamestate::APPLES_PLAY_AREA.top;
        gameState.appleSize = min(playAreaSizeX / gameState.appleCountX, playAreaSizeY / gameState.appleCountY);

        FLOAT playAreaCenterX = (gamestate::APPLES_PLAY_AREA.right + gamestate::APPLES_PLAY_AREA.left) / 2.0f;
        FLOAT appleMinX = playAreaCenterX - (gameState.appleCountX / 2.0f) * gameState.appleSize;
        FLOAT playAreaCenterY = (gamestate::APPLES_PLAY_AREA.bottom + gamestate::APPLES_PLAY_AREA.top) / 2.0f;
        FLOAT appleMinY = playAreaCenterY - (gameState.appleCountY / 2.0f) * gameState.appleSize;

        std::uniform_int_distribution appleDistr(1, 9);
        INT valueSum = 0;
        for (INT x = 0; x < gameState.appleCountX; x++) {
            gameState.play.apples.push_back(std::vector<Apple>());
            for (INT y = 0; y < gameState.appleCountY; y++) {
                INT value = appleDistr(rng);
                valueSum += value;

                gameState.play.apples.back().push_back(Apple(value,
                    appleMinX + gameState.appleSize * (x + 0.5f),
                    appleMinY + gameState.appleSize * (y + 0.5f)));
            }
        }

        // make sum of values divisible by 10 to make board more clearable
        while (valueSum % 10 != 0) {
            INT x = std::uniform_int_distribution(0, gameState.appleCountX - 1)(rng);
            INT y = std::uniform_int_distribution(0, gameState.appleCountY - 1)(rng);

            if (gameState.play.apples[x][y].value != 1) {
                gameState.play.apples[x][y].value--;
                valueSum--;
            }
        }
    }

    void endPlaying(GameState& gameState) {
        gameState.play.apples.clear();
    }

    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs) {
        if (gamestate::buttonMainMenuStart.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON)) {
            gameState.mode = GameState::Mode::PLAYING;
            initPlaying(gameState, timeMs);
            return;
        }

        if (gamestate::buttonMainMenuHelp.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON)) {
            gameState.mode = GameState::Mode::HELP_MENU;
            return;
        }
    }

    void helpMenu(GameState& gameState, const Controller& controller) {
        if (gamestate::buttonHelpMenuBack.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON)) {
            gameState.mode = GameState::Mode::MAIN_MENU;
            return;
        }
    }

    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs) {
        for (std::vector<Apple>& appleRow : gameState.play.apples) {
            for (Apple& apple : appleRow) {
                apple.animate(timeMs - gameState.previousTimeMs);
            }
        }

        INT popCount = 0;
        if (controller.keyJustDown(VK_SPACE) && popCount < gameState.appleCountX * gameState.appleCountY) {
            while (true) {
                if (popCount++ > 10000) { break; }
                INT x = std::uniform_int_distribution(0, gameState.appleCountX - 1)(rng);
                INT y = std::uniform_int_distribution(0, gameState.appleCountY - 1)(rng);
                if (!gameState.play.apples[x][y].popped) {
                    gameState.play.apples[x][y].pop();
                    break;
                }
            }
        }
    }
}

void gameLogic::free() {

}
