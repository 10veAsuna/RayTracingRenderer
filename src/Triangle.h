#pragma once
#include "Primitive.h"

class Triangle : public Primitive
{
public:
    Triangle(SceneObject* pSceneObject, const Vector3f& v0, const Vector3f& v1, const Vector3f& v2);
    virtual bool Intersect(Ray ray, Intersection& isect) const override;
    virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;
private:
    //局部坐标系下三角形顶点位置和法线
    Vector3f mVertices[3];
    Vector3f mNormal;
    float mArea;
};
