#pragma once
#include "Common.h"

struct Ray
{
    Vector3f origin;
    Vector3f direction;
    
    float mint = 0.0f;
    float maxt = FLT_MAX;
};

inline Ray operator*(const Matrix4x4& m, const Ray& ray)
{
    Ray result;
    result.origin =Vector3f(m * Vector4f(ray.origin, 1.0f));
    result.direction =Vector3f(m * Vector4f(ray.direction, 0.0f));
    result.mint = ray.mint;
    result.maxt = ray.maxt;
    return result;
}

struct Intersection
{
    Vector3f position;
    Vector3f normal;
    float t;
};

