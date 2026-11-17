#include "Light.h"

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = p - mDirection * 100000.0f; //假设光源在无限远处，沿着光线方向
    return mRadiance;
}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = mPosition;
    float R = glm::length(p - mPosition);
    float attenuation = 1.0f / (mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z);
    return attenuation * mIntensity;
}

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
    sourcePos = mPosition;
    //距离衰减
    float R = glm::length(p - mPosition);
    float k1 = 1.0f / (mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z);
        
    //角度衰减
    Vector3f L = glm::normalize(p - mPosition);
    float cosTheta = glm::dot(L, mDirection);
    float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle);
        
    return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
        
}
