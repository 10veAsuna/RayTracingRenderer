#include "Disk.h"

Disk::Disk(const Vector3f& center, const Vector3f& euler, float radius)
    : mRadius(radius)
{
    mObjectToWorld = MakeWorldTransform(center, euler, 1.0f);
    mWorldToObject = glm::inverse(mObjectToWorld);
}

bool Disk::Intersect(Ray ray, Intersection& isect) const
{
    //ray转到局部坐标系
    Ray r = mWorldToObject * ray;
    
    //t
    if (fabs(r.direction.z) <= 1e-6f) //平行无交点
        return false;
    
    float t = -r.origin.z / r.direction.z; //圆盘所在平面的z = 0
    
    if (t < r.mint || t > r.maxt)
        return false;
    
    Vector3f p = r.origin + t * r.direction; //交点位置（局部空间）
    
    if (glm::dot(p, p) > mRadius * mRadius)
        return false;
    
    isect.position = Vector3f(mObjectToWorld * Vector4f(p , 1.0f)); //交点位置世界坐标   
    isect.normal = glm::normalize(Vector3f(mObjectToWorld * Vector4f(0.0f, 0.0f, 1.0f, 0.0f)));
    isect.t = t;
    return true;
}
