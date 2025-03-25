#pragma once

#include "Globals.h"

struct ConstantBuffer {
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
};