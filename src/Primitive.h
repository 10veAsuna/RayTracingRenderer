#pragma once
#include "Ray.h"

class SceneObject;

class Primitive
{
public:
    Primitive(SceneObject* pSceneObject) : m_pSceneObject(pSceneObject) {};
    virtual bool Intersect(Ray ray, Intersection& isect) const = 0;
    virtual ~Primitive() {}
    
protected:
    SceneObject* m_pSceneObject = nullptr;
};
