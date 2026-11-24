#pragma once
#include "Primitive.h"

class Triangle : public Primitive
{
public:
    Triangle(SceneObject* pSceneObject, const Vector3f positions[3], const Vector2f uvs[3]);
    virtual bool Intersect(Ray ray, Intersection& isect) const override;
    virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;
private:
    //局部坐标系下三角形顶点位置和法线
    Vector3f mPositions[3];
    Vector2f mTexCoords[3];
    Vector3f mNormal;
    Vector3f mTangent;
    float mArea;
};
