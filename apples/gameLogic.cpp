#include "gameLogic.h"

#include<random>
#include "helper.h"

using gamestate::GameState;
using gamestate::Apple;

namespace {
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

    bool titleMenu(GameState& gameState, const Controller& controller);
    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs);
    void helpMenu(GameState& gameState, const Controller& controller);
    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs);

} // namespace

void gameLogic::init(UINT64 timeMs, GameState& gameState) {
    rng.seed(timeMs);

    gameState.mode = GameState::Mode::TITLE_MENU;

    gameState.appleCountX = gamestate::DEFAULT_APPLES_X;
    gameState.appleCountY = gamestate::DEFAULT_APPLES_Y;
    gameState.playTime = gamestate::DEFAULT_PLAY_TIME_SECONDS;

    gameState.previousTimeMs = timeMs;

    gameState.highScore = 0;
}

bool gameLogic::processFrame(const Controller& controller, GameState& gameState, UINT64 timeMs) {
    gameState.currentTimeMs = timeMs;

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
    case GameState::Mode::TITLE_MENU:
        return titleMenu(gameState, controller);

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

    return false;
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
    if (posY > 30000.0f) { return; }

    FLOAT deltaTimeSec = deltaTimeMs / 1000.0f;

    posX += velX * deltaTimeSec;

    posY += velY * deltaTimeSec;
    velY += accY * deltaTimeSec;

    angle += velAngular * deltaTimeSec;
}

namespace {
    void initPlaying(GameState& gameState, UINT64 timeMs) {
        gameState.play.timesOver = false;
        gameState.play.startTimeMs = timeMs;
        gameState.play.score = 0;
        gameState.play.inDrag = false;

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
        INT antiLockProtection = 10000; // in very impropable case that only 1s are on the apples
        while (valueSum % 10 != 0 && antiLockProtection-- > 0) {
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

    bool titleMenu(GameState& gameState, const Controller& controller) {
        if (controller.keyJustDown(VK_LBUTTON)) {
            gameState.mode = GameState::Mode::MAIN_MENU;
        }

        if (controller.keyJustDown(VK_ESCAPE)) {
            return true;
        }

        return false;
    }


    void mainMenu(GameState& gameState, const Controller& controller, UINT64 timeMs) {
        if (controller.keyJustDown(VK_ESCAPE)) {
            gameState.mode = GameState::Mode::TITLE_MENU;
            return;
        }

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

        if (gamestate::buttonMainMenuReset.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON)) {
            gameState.appleCountX = gamestate::DEFAULT_APPLES_X;
            gameState.appleCountY = gamestate::DEFAULT_APPLES_Y;
            gameState.playTime = gamestate::DEFAULT_PLAY_TIME_SECONDS;
            return;
        }

        INT settingMenuSelected = -1;
        for (int i = 0; i < 6; i++) {
            if (gamestate::mainMenuSettingsButtons[i].hoverOver(gameState.logicalMouseX, gameState.logicalMouseY)) {
                settingMenuSelected = i;
                break;
            }
        }

        if (settingMenuSelected != -1 && controller.keyJustDown(VK_LBUTTON)) {
            switch (settingMenuSelected) {
            case 0:
                if (gameState.appleCountX < 32) { gameState.appleCountX++; }
                break;
            case 3:
                if (gameState.appleCountX > 4) { gameState.appleCountX--; }
                break;
            case 1:
                if (gameState.appleCountY < 20) { gameState.appleCountY++; }
                break;
            case 4:
                if (gameState.appleCountY > 4) { gameState.appleCountY--; }
                break;
            case 2:
                if (gameState.playTime < 900 ) { gameState.playTime += 5; }
                break;
            case 5:
                if (gameState.playTime > 5) { gameState.playTime -= 5; }
                break;
            }
            return;
        }
    }

    void helpMenu(GameState& gameState, const Controller& controller) {
        if (controller.keyJustDown(VK_ESCAPE) ||
            (gamestate::buttonHelpMenuBack.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON))) {
            gameState.mode = GameState::Mode::MAIN_MENU;
            return;
        }
    }

    void playing(GameState& gameState, const Controller& controller, UINT64 timeMs) {
        if (controller.keyJustDown(VK_ESCAPE) ||
            (gamestate::buttonPlayingMenu.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON))) {
            gameState.mode = GameState::Mode::MAIN_MENU;
            endPlaying(gameState);
            return;
        }

        if (controller.keyJustDown('R') ||
            (gamestate::buttonPlayingReset.hoverOver(gameState.logicalMouseX, gameState.logicalMouseY) &&
            controller.keyJustDown(VK_LBUTTON))) {
            endPlaying(gameState);
            initPlaying(gameState, timeMs);
            return;
        }

        if (gameState.currentTimeMs > gameState.play.startTimeMs + gameState.playTime * 1000) {
            gameState.play.timesOver = true;
            gameState.play.inDrag = false;
            for (std::vector<Apple>& appleRow : gameState.play.apples) {
                for (Apple& apple : appleRow) {
                    apple.inDrag = false;
                }
            }

            if (gameState.play.score > gameState.highScore &&
                gameState.appleCountX == gamestate::DEFAULT_APPLES_X &&
                gameState.appleCountY == gamestate::DEFAULT_APPLES_Y &&
                gameState.playTime == gamestate::DEFAULT_PLAY_TIME_SECONDS) {
                gameState.highScore = gameState.play.score;
            }
        }

        for (std::vector<Apple>& appleRow : gameState.play.apples) {
            for (Apple& apple : appleRow) {
                apple.animate(timeMs - gameState.previousTimeMs);
            }
        }

        // start dragging:
        if (controller.keyJustDown(VK_LBUTTON) &&
            gameState.logicalMouseX > gamestate::APPLES_PLAY_AREA.left &&
            gameState.logicalMouseX < gamestate::APPLES_PLAY_AREA.right &&
            gameState.logicalMouseY > gamestate::APPLES_PLAY_AREA.top &&
            gameState.logicalMouseY < gamestate::APPLES_PLAY_AREA.bottom) {
            gameState.play.inDrag = true;
            gameState.play.dragStartX = gameState.logicalMouseX;
            gameState.play.dragStartY = gameState.logicalMouseY;
        }

        INT dragSum = 0;
        if (gameState.play.inDrag) {
            FLOAT dragAreaLeft =   min(gameState.logicalMouseX, gameState.play.dragStartX);
            FLOAT dragAreaRight =  max(gameState.logicalMouseX, gameState.play.dragStartX);
            FLOAT dragAreaTop =    min(gameState.logicalMouseY, gameState.play.dragStartY);
            FLOAT dragAreaBottom = max(gameState.logicalMouseY, gameState.play.dragStartY);

            for (std::vector<Apple>& appleRow : gameState.play.apples) {
                for (Apple& apple : appleRow) {
                    FLOAT d = 0.05f * gameState.appleSize;
                    if (!apple.popped &&
                        apple.posX + d >= dragAreaLeft && apple.posX - d <= dragAreaRight &&
                        apple.posY + d >= dragAreaTop  && apple.posY - d <= dragAreaBottom) {
                        apple.inDrag = true;
                        dragSum += apple.value;
                    } else {
                        apple.inDrag = false;
                    }
                }
            }
        }

        // finish dragging:
        if (gameState.play.inDrag && controller.keyJustUp(VK_LBUTTON)) {
            gameState.play.inDrag = false;

            if (dragSum == 10) {
                for (std::vector<Apple>& appleRow : gameState.play.apples) {
                    for (Apple& apple : appleRow) {
                        if (apple.inDrag) {
                            apple.pop();
                            gameState.play.score++;
                        }
                    }
                }
            }

            for (std::vector<Apple>& appleRow : gameState.play.apples) {
                for (Apple& apple : appleRow) {
                    apple.inDrag = false;
                }
            }
        }
    }
}

void gameLogic::free() {

}
