#pragma once

#include <queue>

class Drawable {
public:
    virtual void draw() = 0;
};

class DrawableBorder : public Drawable {
public:
    void draw();
};

class DrawOrders {
public:
    FLOAT finalScaleFactor = 1.0f;
    FLOAT finalOffsetX = 0.0f;
    FLOAT finalOffsetY = 0.0f;
    std::queue<Drawable*> drawQueue;
};