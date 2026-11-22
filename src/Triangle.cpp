#include "Triangle.h"

#include "SceneObject.h"

Triangle::Triangle(SceneObject* pSceneObject, const Vector3f positions[3], const Vector2f uvs[3])
    : Primitive(pSceneObject)
{
    Matrix4x4 worldMatrix = pSceneObject->GetObjectToWorld();
 
    for (int i = 0; i < 3; i++)
    {
        mPositions[i] = Vector3f(worldMatrix * Vector4f(positions[i], 1.0f));
        mTexCoords[i] = uvs[i];
    }
    
    Vector3f edge1 = mPositions[1] - mPositions[0];
    Vector3f edge2 = mPositions[2] - mPositions[0];
    mNormal = glm::normalize(glm::cross(edge1, edge2));
    mArea = 0.5f * glm::length(glm::cross(edge1, edge2));
}

bool Triangle::Intersect(Ray ray, Intersection& isect) const
{
    Ray r = m_pSceneObject->GetWorldToObject() * ray;

    Vector3f p0 = mPositions[0];
    Vector3f p1 = mPositions[1];
    Vector3f p2 = mPositions[2];
    
    Vector3f e1 = p1 - p0;
    Vector3f e2 = p2 - p0;
    Vector3f s = r.origin - p0;
    
    Vector3f s1 = glm::cross(r.direction, e2);
    Vector3f s2 = glm::cross(s, e1);
    
    float det = glm::dot(s1, e1);
    if (fabs(det) < 1e-6f) //平行
        return false;
    
    float invDet = 1.0f / det;
    float b1 = glm::dot(s1, s) * invDet;
    float b2 = glm::dot(s2, r.direction) * invDet;
    float t = glm::dot(s2, e2) * invDet;
    
    if (t < r.mint || t > r.maxt)
        return false;
    float b0 = 1.0f - b1 - b2;
    if (b0 <0.0f || b1 < 0.0f || b2 < 0.0f)
        return false;
    
    Vector3f p = r.origin + t * r.direction;
    isect.position = Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(p, 1.0f));
    isect.normal = glm::normalize(Vector3f(m_pSceneObject->GetObjectToWorld() * Vector4f(mNormal, 0.0f)));
    isect.uv = b0 * mTexCoords[0] + b1 * mTexCoords[1] + b2 * mTexCoords[2];
    isect.t = t;
    return true;
}

void Triangle::Sample(Vector3f& p, Vector3f& normal, float& pdf) const
{
    p = UniformRandomTriangle(mPositions[0], mPositions[1], mPositions[2]);
    normal = mNormal;
    pdf = 1.0f / mArea;
}
