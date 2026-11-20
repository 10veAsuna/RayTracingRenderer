#include "Light.h"

#include "SceneObject.h"

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
    sourcePos = p - mDirection * 100000.0f; //假设光源在无限远处，沿着光线方向
    pdf = 1.0f;
    return mRadiance;
}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
    sourcePos = mPosition;
    float R = glm::length(p - mPosition);
    float attenuation = 1.0f / (mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z);
    pdf = 1.0f;
    return attenuation * mIntensity;
}

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
    sourcePos = mPosition;
    //距离衰减
    float R = glm::length(p - mPosition);
    float k1 = 1.0f / (mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z);
        
    //角度衰减
    Vector3f L = glm::normalize(p - mPosition);
    float cosTheta = glm::dot(L, mDirection);
    float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle);
    pdf = 1.0f;
    return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
        
}

Color AreaLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
    Vector3f A;
    Vector3f normalA;
    float pdfA;
    pSceneObject->Sample(A, normalA, pdfA);
    
    sourcePos = A;
    Vector3f d = p - A;
    float R = glm::length(d);
    d /= glm::max(1e-5f, R);
    float cosThetaA = glm::dot(normalA, d);
    if (cosThetaA < 1e-5f)
    {
        pdf = 0.0f;
        return Color(0.0f);
    }
    pdf = R * R * pdfA / cosThetaA;
    
    return pSceneObject->GetEmissive();
}
