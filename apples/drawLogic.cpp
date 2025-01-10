#include "drawLogic.h"

#include "helper.h"

using D2D1::Point2F;
using D2D1::ColorF;
using D2D1::Matrix3x2F;
using help::hCheck;
using gamestate::GameState;
using gamestate::Apple;

static_assert(gamestate::LOGICAL_WINDOW_SIZE_X == 1920.0f);
static_assert(gamestate::LOGICAL_WINDOW_SIZE_Y == 1080.0f);

namespace {
    const ColorF BG_COLOR = ColorF(1.0f, 0.8f, 0.4f);

    IDWriteTextFormat* textFormatComicSans = nullptr;
    IDWriteTextFormat* textFormatVCR = nullptr;

    ID2D1SolidColorBrush* solidBrush = nullptr;

    // universal arguments to helper functions (no point in typing them for each helper function):
    const MyD2DObjectCollection* p_myd2d;
    const GameState* p_gameState;
    Matrix3x2F finalTransform;

    void mainMenu();
    void helpMenu();
    void playing();
} // namespace

void drawLogic::init(const MyD2DObjectCollection& myd2d, rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        myd2d.write_factory->CreateTextFormat(
            L"Comic Sans MS", nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            192.0f, L"en-us", &textFormatComicSans);


        IDWriteFontSetBuilder1* font_set_builder = nullptr;
        IDWriteFontFile* font_file = nullptr;
        IDWriteFontSet* font_set = nullptr;
        IDWriteFontCollection1* font_collection = nullptr;
        hCheck(myd2d.write_factory->CreateFontSetBuilder(&font_set_builder));
        hCheck(myd2d.write_factory->CreateFontFileReference(L"assets/fonts/VCR_OSD_MONO_1.001.ttf", NULL, &font_file));
        hCheck(font_set_builder->AddFontFile(font_file));
        hCheck(font_set_builder->CreateFontSet(&font_set));
        hCheck(myd2d.write_factory->CreateFontCollectionFromFontSet(font_set, &font_collection));

        help::SafeRelease(font_set_builder);
        help::SafeRelease(font_file);
        help::SafeRelease(font_set);

        myd2d.write_factory->CreateTextFormat(
            L"VCR OSD Mono", font_collection,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            64.0f, L"en-us", &textFormatVCR);
        textFormatVCR->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        textFormatVCR->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        help::SafeRelease(font_collection);
    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        auto d2drt = myd2d.d2d_render_target;
        hCheck(d2drt->CreateSolidColorBrush(ColorF(ColorF::Black), &solidBrush));
        
    }
}

void drawLogic::drawFrame(const MyD2DObjectCollection& myd2d, const GameState& gameState) {
    myd2d.d2d_render_target->Clear(BG_COLOR);

    finalTransform = Matrix3x2F::Scale(gameState.graphicalScale, gameState.graphicalScale)
        * Matrix3x2F::Translation(gameState.graphicalOffsetX, gameState.grpahicalOffsetY);
    myd2d.d2d_render_target->SetTransform(finalTransform);

    // draw border:
    solidBrush->SetColor(ColorF(ColorF::Green));
    myd2d.d2d_render_target->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::Rect(30.0f, 30.0f, 1890.0f, 1050.0f), 30.0f, 30.0f),
        solidBrush, 40.0f);

    // test mouse positon tracking:
    myd2d.d2d_render_target->FillRectangle(
        D2D1::Rect(gameState.logicalMouseX - 5.0f, gameState.logicalMouseY - 5.0f,
            gameState.logicalMouseX + 5.0f, gameState.logicalMouseY + 5.0f),
        solidBrush
    );

    p_myd2d = &myd2d;
    p_gameState = &gameState;
    switch (gameState.mode) {
    case GameState::Mode::MAIN_MENU:
        mainMenu();
        break;

    case GameState::Mode::HELP_MENU:
        helpMenu();
        break;

    case GameState::Mode::PLAYING:
        playing();
        break;
    }
    p_myd2d = nullptr;
    p_gameState = nullptr;
}


namespace {
    void drawButton(const gamestate::Button& buttonData) {
        D2D1_RECT_F thisRect = D2D1::Rect(buttonData.left, buttonData.top,
            buttonData.right, buttonData.bottom);


        solidBrush->SetColor(buttonData.hoverOver(p_gameState->logicalMouseX, p_gameState->logicalMouseY) ?
            ColorF(ColorF::Blue) : ColorF(ColorF::Red));
        p_myd2d->d2d_render_target->FillRoundedRectangle(
            D2D1::RoundedRect(thisRect, 5.0f, 5.0f),
            solidBrush);


        FLOAT centerX = (thisRect.left + thisRect.right) / 2.0f;
        FLOAT centerY = (thisRect.top + thisRect.bottom) / 2.0f;
        FLOAT textScale = (thisRect.bottom - thisRect.top) / 80.0f; // text is scalled with button height
        p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(textScale, textScale, D2D1::Point2F(centerX, centerY)) *
            finalTransform);

        solidBrush->SetColor(ColorF(ColorF::White));
        p_myd2d->d2d_render_target->DrawTextW(
            buttonData.text.data(), buttonData.text.size(),
            textFormatVCR,
            thisRect,
            solidBrush);

        p_myd2d->d2d_render_target->SetTransform(finalTransform);
    }

    void drawApple(const Apple& apple, bool popped) {
        if (apple.popped != popped) { return; }
        if (apple.posY > 25000.0f) { return; }

        D2D1_RECT_F thisRect = D2D1::Rect(-150.0f, -150.0f, 150.0f, 150.0f);

        Matrix3x2F appleTransform = Matrix3x2F::Scale(p_gameState->appleSize / 400.0f, p_gameState->appleSize / 400.0f) * 
            Matrix3x2F::Rotation(apple.angle) * 
            Matrix3x2F::Translation(apple.posX, apple.posY) * 
            finalTransform;
        p_myd2d->d2d_render_target->SetTransform(appleTransform);


        solidBrush->SetColor(ColorF(ColorF::Red));
        p_myd2d->d2d_render_target->FillRectangle(thisRect, solidBrush);

        solidBrush->SetColor(apple.inDrag ? ColorF(ColorF::Goldenrod) : ColorF(ColorF::SaddleBrown));
        p_myd2d->d2d_render_target->DrawRectangle(thisRect, solidBrush, 40.0f);

        p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(4.0f, 4.0f) * appleTransform);
        std::wstring text = std::to_wstring(apple.value);
        solidBrush->SetColor(ColorF(ColorF::White));
        p_myd2d->d2d_render_target->DrawTextW(
            text.data(), text.size(),
            textFormatVCR,
            thisRect,
            solidBrush);

        p_myd2d->d2d_render_target->SetTransform(finalTransform);
    }


    void mainMenu() {
        // draw name of the game:
        {
            D2D1_RECT_F textRect = D2D1::Rect(230.0f, 120.0f, 1920.0f, 1080.0f);

            std::wstring text = L"Apple";
            solidBrush->SetColor(ColorF(ColorF::MediumVioletRed));
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatComicSans, textRect, solidBrush);

            textRect.left = 820.0f;
            text = L"Container";
            solidBrush->SetColor(ColorF(ColorF::Khaki));
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatComicSans, textRect, solidBrush);
        }

        drawButton(gamestate::buttonMainMenuStart);
        drawButton(gamestate::buttonMainMenuHelp);
    }

    void helpMenu() {
        drawButton(gamestate::buttonHelpMenuBack);
    }

    void playing() {
        solidBrush->SetColor(ColorF(0.75f, 0.6f, 0.3f));
        p_myd2d->d2d_render_target->DrawRectangle(gamestate::APPLES_PLAY_AREA,
            solidBrush, 2.0f);

        for (bool popped : {false, true}) {
            for (const std::vector<Apple>& appleRow : p_gameState->play.apples) {
                for (const Apple& apple : appleRow) {
                    drawApple(apple, popped);
                }
            }
        }

        if (p_gameState->play.inDrag) {
            solidBrush->SetColor(ColorF(ColorF::Black));
            p_myd2d->d2d_render_target->DrawRectangle(D2D1::Rect(
                p_gameState->logicalMouseX, p_gameState->logicalMouseY,
                p_gameState->play.dragStartX, p_gameState->play.dragStartY),
                solidBrush, 6.0f);
        }
    }
} // namespace

void drawLogic::free(rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        help::SafeRelease(textFormatComicSans);
        help::SafeRelease(textFormatVCR);
    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        help::SafeRelease(solidBrush);
    }
}
