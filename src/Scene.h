#pragma once
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Camera.h"
#include "Light.h"
#include "SceneObject.h"

class Scene
{
public:
    void SetCamera(const Camera& camera) {mCamera = camera;}
    const Camera& GetCamera() const {return mCamera;}
    
    SceneObject* Intersect(Ray ray, Intersection& isect) const;
    
    SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);
    
    template<typename T, typename... Args>
    Light* CreateLight(Args&&...args)
    {
        Light* light = new T(std::forward<Args>(args)...);
        mLights.push_back(light);
        return (T*)light;
    }
    
    template<typename T, typename... Args>
    T* CreateMaterial(const std::string& name, Args&&...args)
    {
        if (mMaterials.find(name) != mMaterials.end())
        {
            throw std::invalid_argument("duplicate material: " + name);
        }
        std::unique_ptr<T> material = std::make_unique<T>(std::forward<Args>(args)...);
        T* result = material.get();
        mMaterials.emplace(name, result);
        material.release();
        return result;
    }

    Material* GetMaterial(const std::string& name) const
    {
        auto it = mMaterials.find(name);
        return it == mMaterials.end() ? nullptr : it->second;
    }
    
    const std::vector<Light*>& GetLights() const {return mLights;}
    
    ~Scene();
    
private:
    Camera mCamera;
    
    std::vector<SceneObject*> mSceneObjects;
    
    std::vector<Light*> mLights;
    
    std::map<std::string, Material*> mMaterials;
};
