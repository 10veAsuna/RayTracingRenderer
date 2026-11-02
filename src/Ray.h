#pragma once
#include "Common.h"

struct Ray
{
    Vector3f origin;
    Vector3f direction;
    
    float mint = 0.f;
    float maxt = FLT_MAX;
};
