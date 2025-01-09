#include "drawLogic.h"

#include "helper.h"

using D2D1::Point2F;
using D2D1::ColorF;
using D2D1::Matrix3x2F;
using help::hCheck;
using gamestate::GameState;

static_assert(gamestate::LOGICAL_WINDOW_SIZE_X == 1920.0f);
static_assert(gamestate::LOGICAL_WINDOW_SIZE_Y == 1080.0f);

namespace {
    const ColorF BG_COLOR = ColorF(1.0f, 0.8f, 0.4f);

    IDWriteTextFormat* textFormatComicSans = nullptr;
    IDWriteTextFormat* textFormatVCR = nullptr;

    ID2D1SolidColorBrush* solidBrush = nullptr;


    Matrix3x2F finalTransform;
} // namespace

void drawLogic::init(const MyD2DObjectCollection& myd2d, rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        myd2d.write_factory->CreateTextFormat(
            L"Comic Sans MS", nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            64.0f, L"en-us", &textFormatComicSans);


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
}

/*void DrawableButton::draw() {
    solidBrush->SetColor(hover ? ColorF(ColorF::Blue) : ColorF(ColorF::BlueViolet));
    currentd2d->d2d_render_target->FillRoundedRectangle(
        D2D1::RoundedRect(D2D1::Rect(left, top, right, bottom), 5.0f, 5.0f),
        solidBrush);

    solidBrush->SetColor(ColorF(ColorF::White));
    textFormatVCR->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    textFormatVCR->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    currentd2d->d2d_render_target->DrawTextW(
        text.data(), text.size(),
        textFormatVCR,
        D2D1::Rect(left, top, right, bottom),
        solidBrush);
}*/

void drawLogic::free(rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        help::SafeRelease(textFormatComicSans);
        help::SafeRelease(textFormatVCR);
    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        help::SafeRelease(solidBrush);
    }
}
