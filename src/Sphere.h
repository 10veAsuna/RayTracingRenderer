#pragma once
#include "Primitive.h"

class Sphere : public Primitive
{
public:
    Sphere(SceneObject* pSceneObject, const float radius);
    virtual bool Intersect(Ray ray, Intersection& isect) const override; 
    
private:
    float mRadius;
};
