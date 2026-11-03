#include "Sphere.h"

Sphere::Sphere(const Vector3f& center, const float radius)
    :mRadius(radius)
{
    mObjectToWorld = MakeWorldTransform(center, Vector3f(0.0f, 0.0f, 0.0f), 1.0f);
    mWorldToObject = glm::inverse(mObjectToWorld);
}


bool Sphere::Intersect(Ray ray, Intersection& isect) const
{
    //ray转到球体的局部空间
    Ray r = mWorldToObject * ray;
    
    float A = glm::dot(r.direction, r.direction);
    float B = 2.0f * glm::dot(r.direction, r.origin);
    float C = glm::dot(r.origin, r.origin) - mRadius * mRadius;
    float delta = B * B - 4.0f * A * C;
    if (delta < 0.0f)
        return false;
    float sqrtDelta = sqrt(delta);
    float t1 = (-B - sqrtDelta) / (2.0f * A);
    float t2 = (-B + sqrtDelta) / (2.0f * A);
    //t1 < t2
    if (t2 < r.mint)
        return false;
    if (t1 > r.maxt)
        return false;
    float t = t1;
    if (t < r.mint)
    {
        t = t2;
        if (t > r.maxt)
            return false;
    }
    
    Vector3f p = r.origin + t * r.direction; //交点位置（局部空间）
    Vector3f n = glm::normalize(p);
    
    isect.position = Vector3f(mObjectToWorld * Vector4f(p , 1.0f)); //交点位置世界坐标   
    isect.normal = Vector3f(mObjectToWorld * Vector4f(n , 0.0f));
    
    isect.t = t;
    return true;
}
