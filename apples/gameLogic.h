#pragma once

#include "controller.h"
#include "drawOrders.h"

namespace gameLogic {
    void init(UINT64 timeMs);
    void processFrame(const Controller& controller, DrawOrders& drawOrders, UINT64 timeMs);
    void free();
} // namespaace gameLogic
