#include "drawLogic.h"

#include "helper.h"

using D2D1::Point2F;
using D2D1::ColorF;
using D2D1::Matrix3x2F;
using help::hCheck;

namespace {
    const ColorF BG_COLOR = ColorF(1.0f, 0.8f, 0.4f);





    ID2D1SolidColorBrush* solidBrush = nullptr;

    Matrix3x2F finalTransform;
    const MyD2DObjectCollection* currentd2d = nullptr;
} // namespace

void drawLogic::init(const MyD2DObjectCollection& myd2d, rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {

    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        auto d2drt = myd2d.d2d_render_target;
        hCheck(d2drt->CreateSolidColorBrush(ColorF(ColorF::Black), &solidBrush));
        
    }
}

void drawLogic::processOrders(const MyD2DObjectCollection& myd2d, DrawOrders& drawOrders) {
    myd2d.d2d_render_target->Clear(BG_COLOR);

    finalTransform = Matrix3x2F::Scale(drawOrders.finalScaleFactor, drawOrders.finalScaleFactor)
        * Matrix3x2F::Translation(drawOrders.finalOffsetX, drawOrders.finalOffsetY);
    myd2d.d2d_render_target->SetTransform(finalTransform);

    currentd2d = &myd2d;
    while (!drawOrders.drawQueue.empty()) {
        drawOrders.drawQueue.front()->draw();
        delete drawOrders.drawQueue.front();
        drawOrders.drawQueue.pop();
    }
    currentd2d = nullptr;
}

void DrawableBorder::draw() {
    solidBrush->SetColor(ColorF(ColorF::Green));
    currentd2d->d2d_render_target->DrawRoundedRectangle(
        D2D1::RoundedRect(D2D1::Rect(30.0f, 30.0f, 1890.0f, 1050.0f), 10.0f, 10.0f),
        solidBrush, 40.0f);
}

void drawLogic::free(rtd rtdv) {
    if (rtdv == rtd::NO_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {

    }

    if (rtdv == rtd::ONLY_RENDER_TARGET_DEPENDENT || rtdv == rtd::ALL) {
        help::SafeRelease(solidBrush);
    }
}
