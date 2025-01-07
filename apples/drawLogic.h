#pragma once

#include "myD2D.h"
#include "drawOrders.h"

namespace drawLogic {
    void init(const MyD2DObjectCollection& myd2d, rtd rtdv);
    void processOrders(const MyD2DObjectCollection& myd2d, DrawOrders& drawOders);
    void free(rtd rtdv);
} // namespace drawLogic
