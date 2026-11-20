#pragma once
#include "Ray.h"

class SceneObject;

class Light
{
public:
    //对于一点p，求L（p）
    virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const = 0;
    virtual ~Light() = default;
};

//平行光
class DirectionalLight : public Light
{
public:
    DirectionalLight(const Vector3f& direction, const Color& radiance)
        : mDirection(glm::normalize(direction)), mRadiance(radiance){}

    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
    Vector3f mDirection; //光线方向，单位向量
    Color mRadiance; //光照强度
    
};

//点光源
class PointLight : public Light
{
public:
    PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuations)
        : mPosition(position), mIntensity(intensity), mAttenuations(attenuations){}
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
    Vector3f mPosition;
    Vector3f mIntensity;
    Vector3f mAttenuations; //衰减系数
};

//聚光灯
class SpotLight : public Light
{
public:
    SpotLight(const Vector3f& position, const Vector3f& direction, const Color& intensity,
        float innerAngle, float outerAngle, const Vector3f& attenuations)
            : mPosition(position), mDirection(glm::normalize(direction)), mIntensity(intensity),
            mCosInnerAngle(cos(innerAngle)), mCosOuterAngle(cos(outerAngle)), mAttenuations(attenuations){}
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;

private:
    Vector3f mDirection;
    Vector3f mPosition;
    Color mIntensity;
    
    float mCosInnerAngle;
    float mCosOuterAngle;
    
    Vector3f mAttenuations;
};

//面光源
class AreaLight : public Light
{
public:
    AreaLight(SceneObject* pSceneObject)
        : pSceneObject(pSceneObject){}
    Color GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const override;
private:
    SceneObject* pSceneObject;
};
