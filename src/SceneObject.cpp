#include "SceneObject.h"

bool SceneObject::Intersect(Ray ray, Intersection& isect) const
{
    bool bHit = false;
    for (const auto& primitive : mPrimitives)
    {
        if (primitive->Intersect(ray, isect))
        {
            ray.maxt = isect.t;
            bHit = true;
        }
    }
    return bHit;
}

SceneObject::~SceneObject()
{
    for (const auto& primitive : mPrimitives)
    {
        delete primitive;
    }
}
