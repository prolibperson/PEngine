#pragma once

#include "Globals.h"

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 texCoord;
    UINT materialIndex;
};