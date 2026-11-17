#pragma once
#include "Material.h"
#include "Primitive.h"
#include "Ray.h"

class  SceneObject
{
public:
    SceneObject(const Vector3f& position, const Vector3f& euler, float scale)
    {
        mObjectToWorld = MakeWorldTransform(position, euler, scale);
        mWorldToObject = glm::inverse(mObjectToWorld);
    }
    bool Intersect(Ray ray, Intersection& isect) const;
    
    void AddPrimitive(Primitive* primitive) { mPrimitives.push_back(primitive); }
    
    template<typename T, typename... Args>
    T* CreatePrimitive(Args&&... args)
    {
        T* primitive = new T(this, std::forward<Args>(args)...);
        mPrimitives.push_back(primitive);
        return primitive;
    }
    
    virtual ~SceneObject();
    
    Matrix4x4 GetObjectToWorld() const  { return mObjectToWorld; }
    Matrix4x4 GetWorldToObject() const  { return mWorldToObject; }
    
    void SetMaterial(Material* material) {m_pMaterial = material;}
    Material* GetMaterial() const { return m_pMaterial; }
private:
    Matrix4x4 mObjectToWorld;
    Matrix4x4 mWorldToObject;
    
    Material* m_pMaterial = nullptr;
    
    std::vector<Primitive*> mPrimitives;
};
