#pragma once
#include "Ray.h"




class Sphere
{
public:
    Sphere(const Vector3f& center, const float radius);
    bool Intersect(Ray ray, Intersection& isect) const; 
    
private:
    float mRadius;
    Matrix4x4 mObjectToWorld; //局部空间到世界空间的变换矩阵
    Matrix4x4 mWorldToObject; 
};
