#include "Disk.h"

#include "SceneObject.h"

Disk::Disk(SceneObject* pSceneObject, float radius)
    : Primitive(pSceneObject), mRadius(radius)
{
}

bool Disk::Intersect(Ray ray, Intersection& isect) const
{
    //ray转到局部坐标系
    Ray r = m_pSceneObject->GetWorldToObject() * ray;
    
    //t
    if (fabs(r.direction.z) <= 1e-6f) //平行无交点
        return false;
    
    float t = -r.origin.z / r.direction.z; //圆盘所在平面的z = 0
    
    if (t < r.mint || t > r.maxt)
        return false;
    
    Vector3f p = r.origin + t * r.direction; //交点位置（局部空间）
    
    if (glm::dot(p, p) > mRadius * mRadius)
        return false;
    
    float phi = std::atan2f(p.y, p.x);
    if (phi < 0.0f)
        phi += 2.0f * PI;
    isect.uv.x = phi * INV_PI * 0.5f;
    isect.uv.y = glm::length(p) / mRadius;
    
    isect.position = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p , 1.0f)); //交点位置世界坐标
    isect.normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(0.0f, 0.0f, 1.0f, 0.0f)));
    isect.t = t;
    return true;
}

void Disk::Sample(Vector3f& p, Vector3f& normal, float& pdf) const
{
    Vector2f p2 = UniformRandomDisk(mRadius);
    p = Vector3f(p2.x, p2.y, 0.0f);
    //转成世界空间
    p = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p, 1.0f));
    normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(0.0f, 0.0f, 1.0f, 0.0f)));
    pdf = 1.0f / (PI * mRadius * mRadius);
}
