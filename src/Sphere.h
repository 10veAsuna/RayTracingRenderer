#pragma once
#include "Primitive.h"

class Sphere : public Primitive
{
public:
    Sphere(SceneObject* pSceneObject, const float radius);
    virtual bool Intersect(Ray ray, Intersection& isect) const override; 
    virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;
private:
    float mRadius;
};
