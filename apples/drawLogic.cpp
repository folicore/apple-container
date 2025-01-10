#include "drawLogic.h"

#include "helper.h"
#include "bitmapFileLoader.h"

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
    ID2D1PathGeometry* appleGeometry = nullptr;
    ID2D1PathGeometry* leafGeometry = nullptr;
    ID2D1RadialGradientBrush* appleGradientBrush = nullptr;
    ID2D1Bitmap* mainMenuBgBitmap = nullptr;
    ID2D1Bitmap* dragBitmap = nullptr;
    ID2D1Bitmap* tutorialBitmap = nullptr;

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
        // load Comic Sans:
        myd2d.write_factory->CreateTextFormat(
            L"Comic Sans MS", nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            192.0f, L"en-us", &textFormatComicSans);

        // load VCR OSD Mono:
        {
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
    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        hCheck(myd2d.d2d_render_target->CreateSolidColorBrush(ColorF(ColorF::Black), &solidBrush));
        

        // create apple geometry:
        {
            ID2D1GeometrySink* sink = nullptr;

            hCheck(myd2d.d2d_factory->CreatePathGeometry(&appleGeometry));
            appleGeometry->Open(&sink);
            sink->BeginFigure(Point2F(0, -100), D2D1_FIGURE_BEGIN_FILLED);
            sink->AddBezier(D2D1::BezierSegment(Point2F(40, -140), Point2F(180, -120), Point2F(160, 10)));
            sink->AddBezier(D2D1::BezierSegment(Point2F(170, 70), Point2F(70, 130), Point2F(0, 110)));
            sink->AddBezier(D2D1::BezierSegment(Point2F(-70, 130), Point2F(-170, 70), Point2F(-160, 10)));
            sink->AddBezier(D2D1::BezierSegment(Point2F(-180, -120), Point2F(-40, -140), Point2F(0, -100)));
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            hCheck(sink->Close());
        }

        // create leaf geometry:
        {
            ID2D1GeometrySink* sink = nullptr;

            hCheck(myd2d.d2d_factory->CreatePathGeometry(&leafGeometry));
            leafGeometry->Open(&sink);
            sink->BeginFigure(Point2F(30, -150), D2D1_FIGURE_BEGIN_FILLED);
            sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(Point2F(10, -200), Point2F(-70, -180)));
            sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(Point2F(20, -110), Point2F(30, -150)));
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            hCheck(sink->Close());
        }

        // create apple gradient:
        {
            ID2D1GradientStopCollection* stops = nullptr;
            D2D1_GRADIENT_STOP stopsData[2];
            stopsData[0] = { .position = 0.0f, .color = ColorF(ColorF::IndianRed) };
            stopsData[1] = { .position = 1.0f, .color = ColorF(ColorF::Red) };

            hCheck(myd2d.d2d_render_target->CreateGradientStopCollection(stopsData, 2, &stops));

            hCheck(myd2d.d2d_render_target->CreateRadialGradientBrush(
                D2D1::RadialGradientBrushProperties(Point2F(180, -70), Point2F(0, 0), 200, 180),
                stops, &appleGradientBrush));

            help::SafeRelease(stops);

        }

        // load images:
        mainMenuBgBitmap = LoadBitmapFromFile(myd2d.d2d_render_target, myd2d.imaging_factory,
            L"assets/images/bigTree.png");
        tutorialBitmap = LoadBitmapFromFile(myd2d.d2d_render_target, myd2d.imaging_factory,
            L"assets/images/tutorial.png");

        // create bitmap for dragging over apples:
        {
            const BYTE BMP_DATA[4] = {0x00, 0x30, 0x40, 0x40};

            hCheck(myd2d.d2d_render_target->CreateBitmap(
                D2D1::SizeU(1, 1),
                BMP_DATA, 1,
                D2D1::BitmapProperties(D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_PREMULTIPLIED)),
                &dragBitmap));
        }

    }
}

void drawLogic::drawFrame(const MyD2DObjectCollection& myd2d, const GameState& gameState) {
    myd2d.d2d_render_target->Clear(BG_COLOR);

    p_myd2d = &myd2d;
    p_gameState = &gameState;
    finalTransform = Matrix3x2F::Scale(gameState.graphicalScale, gameState.graphicalScale)
        * Matrix3x2F::Translation(gameState.graphicalOffsetX, gameState.grpahicalOffsetY);
    myd2d.d2d_render_target->SetTransform(finalTransform);


    if (gameState.mode == GameState::Mode::TITLE_MENU ||
        gameState.mode == GameState::Mode::MAIN_MENU) {
        p_myd2d->d2d_render_target->DrawBitmap(mainMenuBgBitmap,
            D2D1::Rect(30.0f, 30.0f, 1890.0f, 1050.0f), 1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

        D2D1_RECT_F textRect = D2D1::Rect(230.0f, 120.0f, 1920.0f, 1080.0f);

        std::wstring text = L"Apple";
        solidBrush->SetColor(ColorF(ColorF::DarkRed));
        p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
            textFormatComicSans, textRect, solidBrush);

        textRect.left = 820.0f;
        text = L"Container";
        solidBrush->SetColor(ColorF(ColorF::LawnGreen));
        p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
            textFormatComicSans, textRect, solidBrush);
    }

    // draw border:
    solidBrush->SetColor(ColorF(ColorF::Green));
    myd2d.d2d_render_target->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::Rect(30.0f, 30.0f, 1890.0f, 1050.0f), 30.0f, 30.0f),
        solidBrush, 40.0f);


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
            ColorF(0.2f, 0.8f, 0.2f) : ColorF(0.05f, 0.20f, 0.05f));
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

    void drawAppleGeometry(D2D1::ColorF lineColor) {
        solidBrush->SetColor(ColorF(ColorF::ForestGreen));
        p_myd2d->d2d_render_target->FillGeometry(leafGeometry, solidBrush);

        solidBrush->SetColor(ColorF(0.17f, 0.05f, 0.05f));
        p_myd2d->d2d_render_target->DrawLine(Point2F(0, -95), Point2F(40, -170), solidBrush, 24.0f);

        solidBrush->SetColor(ColorF(ColorF::Red));
        p_myd2d->d2d_render_target->FillGeometry(appleGeometry, appleGradientBrush);

        solidBrush->SetColor(lineColor);
        p_myd2d->d2d_render_target->DrawGeometry(appleGeometry, solidBrush, 24.0f);
    }

    void drawApple(const Apple& apple, bool popped, bool withText = true) {
        if (apple.popped != popped) { return; }
        if (apple.posY > 25000.0f) { return; }

        D2D1_RECT_F thisRect = D2D1::Rect(-150.0f, -150.0f, 150.0f, 150.0f);
        
        Matrix3x2F appleTransform = Matrix3x2F::Scale(p_gameState->appleSize / 400.0f, p_gameState->appleSize / 400.0f) * 
            Matrix3x2F::Rotation(apple.angle) * 
            Matrix3x2F::Translation(apple.posX, apple.posY) * 
            finalTransform;
        p_myd2d->d2d_render_target->SetTransform(appleTransform);

        drawAppleGeometry(apple.inDrag ? ColorF(ColorF::Goldenrod) : ColorF(ColorF::SaddleBrown));


        if (withText) {
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
    }


    void mainMenu() {
        drawButton(gamestate::buttonMainMenuStart);
        drawButton(gamestate::buttonMainMenuHelp);

        solidBrush->SetColor(ColorF(ColorF::Black));

        // high score display:
        {
            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(1.0f, 1.0f) *
                Matrix3x2F::Translation(300.0f, 450.0f) *
                finalTransform);

            D2D1_RECT_F textRect = D2D1::Rect(-1000.0f, -1000.0f, 1000.0f, 1000.0f);
            std::wstring text = L"High Score:";
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);

            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(2.0f, 2.0f) *
                Matrix3x2F::Translation(300.0f, 550.0f) *
                finalTransform);

            text = std::to_wstring(p_gameState->highScore);
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);

            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(0.3f, 0.3f) *
                Matrix3x2F::Translation(300.0f, 490.0f) *
                finalTransform);

            text = std::wstring(L"(") + std::to_wstring(gamestate::DEFAULT_APPLES_X) +
                L" x " + std::to_wstring(gamestate::DEFAULT_APPLES_Y) +
                L" x " + std::to_wstring(gamestate::DEFAULT_PLAY_TIME_SECONDS) + L"s only)";
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);

            p_myd2d->d2d_render_target->SetTransform(finalTransform);
        }

        // game settings:
        {
            for (INT i = 0; i < 6; i++) {
                drawButton(gamestate::mainMenuSettingsButtons[i]);
            }
            drawButton(gamestate::buttonMainMenuReset);

            for (int i = 0; i < 3; i++) {
                D2D1_RECT_F rect = D2D1::Rect(
                    gamestate::mainMenuSettingsButtons[i].left - 40.0f,
                    gamestate::mainMenuSettingsButtons[i].bottom + 10.0f,
                    gamestate::mainMenuSettingsButtons[i].right + 40.0f,
                    gamestate::mainMenuSettingsButtons[i].bottom + 150.0f);

                solidBrush->SetColor(ColorF(ColorF::Wheat));
                p_myd2d->d2d_render_target->FillRoundedRectangle(
                    D2D1::RoundedRect(rect, 10.0f, 10.0f), solidBrush);

                solidBrush->SetColor(ColorF(ColorF::Black));
                std::wstring text;
                if (i == 0) { text = std::to_wstring(p_gameState->appleCountX); }
                if (i == 1) { text = std::to_wstring(p_gameState->appleCountY); }
                if (i == 2) { text = std::to_wstring(p_gameState->playTime) + L"s"; }

                p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                    textFormatVCR, rect, solidBrush);
            }
        }
    }

    void helpMenu() {
        drawButton(gamestate::buttonHelpMenuBack);

        FLOAT imgLeft = 1260.0f;
        FLOAT imgTop = 270.0f;
        p_myd2d->d2d_render_target->DrawBitmap(tutorialBitmap,
            D2D1::Rect(imgLeft, imgTop, imgLeft + 550.0f, imgTop + 475.0f), 1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

        solidBrush->SetColor(ColorF(ColorF::Black));

        p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(0.7f, 0.7f) *
            Matrix3x2F::Translation(100.0f, 0.0f) *
            finalTransform);

        D2D1_RECT_F textRect = D2D1::Rect(130.0f, 70.0f, 19200.0f, 10800.0f);
        std::wstring text = L"How to play";
        p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
            textFormatComicSans, textRect, solidBrush);

        p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(0.25f, 0.25f) *
            Matrix3x2F::Translation(70.0f, 250.0f) *
            finalTransform);

        text = L"The goal of the game is to clear as many apples\n"
            L"as possible. Apples can be cleared by dragging\n"
            L"mouse over them so sum of their values euqals 10.\n"
            L"You get 1 point for each apple cleared, regardless\n"
            L"of its value.\n\n"
            L"Keybinds:\nEsc: previous menu\nR: reset game";
        p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
            textFormatComicSans, textRect, solidBrush);

        p_myd2d->d2d_render_target->SetTransform(finalTransform);
    }

    void playing() {
        // draw score:
        {
            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(0.8f, 0.8f) *
                Matrix3x2F::Translation(230.0f, 250.0f) *
                finalTransform);

            drawAppleGeometry(ColorF(ColorF::SaddleBrown));

            solidBrush->SetColor(ColorF(ColorF::White));

            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(1.05f, 1.05f) *
                Matrix3x2F::Translation(233.0f, 130.0f) *
                finalTransform);

            D2D1_RECT_F textRect = D2D1::Rect(-400.0f, -50.0f, 400.0f, 150.0f);
            std::wstring text = L"Score";
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);

            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Scale(2.05f, 2.05f) *
                Matrix3x2F::Translation(231.0f, 165.0f) *
                finalTransform);

            text = std::to_wstring(p_gameState->play.score);
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);

            p_myd2d->d2d_render_target->SetTransform(finalTransform);
        }

        // draw timer: 
        {
            D2D1_POINT_2F clockCenter = Point2F(230.0f, 500.0f);
            FLOAT clockRadius = 100.0f;

            solidBrush->SetColor(ColorF(ColorF::ForestGreen));
            p_myd2d->d2d_render_target->FillEllipse(
                D2D1::Ellipse(clockCenter, clockRadius, clockRadius), solidBrush
            );

            FLOAT rotationAngle = 360.0f * static_cast<FLOAT>(p_gameState->currentTimeMs - p_gameState->play.startTimeMs) /
                1000.0f / static_cast<FLOAT>(p_gameState->playTime);
            p_myd2d->d2d_render_target->SetTransform(Matrix3x2F::Rotation(rotationAngle, clockCenter) *
                finalTransform);

            solidBrush->SetColor(ColorF(ColorF::DarkGreen));
            p_myd2d->d2d_render_target->DrawLine(clockCenter, D2D1::Point2F(clockCenter.x, clockCenter.y - clockRadius), solidBrush, 5.0f);

            p_myd2d->d2d_render_target->SetTransform(finalTransform);

            D2D1_RECT_F textRect = D2D1::Rect(clockCenter.x - clockRadius, clockCenter.y - clockRadius,
                clockCenter.x + clockRadius, clockCenter.y + clockRadius);

            // to prevent flashing digit when restting and for number to stop at 0:
            UINT64 endTime = p_gameState->play.startTimeMs + 1000 * p_gameState->playTime;
            INT displayedTime = (p_gameState->currentTimeMs < endTime) ? (min(p_gameState->playTime - 1, (endTime - p_gameState->currentTimeMs) / 1000)) : 0;
            solidBrush->SetColor(ColorF(ColorF::White));
            std::wstring text = std::to_wstring(displayedTime);
            p_myd2d->d2d_render_target->DrawTextW(text.data(), text.size(),
                textFormatVCR, textRect, solidBrush);
        }

        // draw buttons:
        drawButton(gamestate::buttonPlayingMenu);
        drawButton(gamestate::buttonPlayingReset);

        // draw play field:
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
            D2D1_RECT_F dragRect = D2D1::Rect(
                p_gameState->logicalMouseX, p_gameState->logicalMouseY,
                p_gameState->play.dragStartX, p_gameState->play.dragStartY);

            p_myd2d->d2d_render_target->DrawBitmap(dragBitmap,
                dragRect, 1.0f,
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);

            solidBrush->SetColor(ColorF(0.5f, 0.375f, 0.0f));
            p_myd2d->d2d_render_target->DrawRectangle(dragRect, solidBrush, 4.0f);
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
        help::SafeRelease(appleGeometry);
        help::SafeRelease(leafGeometry);
        help::SafeRelease(appleGradientBrush);
        help::SafeRelease(mainMenuBgBitmap);
        help::SafeRelease(dragBitmap);
    }
}
