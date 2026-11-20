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

void SceneObject::Sample(Vector3f& p, Vector3f& normal, float& pdf)
{
    //随机取一个primitive
    int index = RandomInt(0, mPrimitives.size() - 1);
    mPrimitives[index]->Sample(p, normal, pdf);
    pdf /= mPrimitives.size();
}
