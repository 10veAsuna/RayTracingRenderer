#include "Material.h"

Vector3f LambertMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    float e1 = Random01();
    float e2 = Random01();
    float cosTheta = glm::sqrt(1.0f - e1);
    float sinTheta = glm::sqrt(e1);
    float phi = 2.0f * PI * e2;
    Vector3f wi;
    wi.x = sinTheta * cos(phi);
    wi.y = sinTheta * sin(phi);
    wi.z = cosTheta;
    pdf = cosTheta * INV_PI;
    return wi;
}

Color ConductorSpecularMaterial::BRDF(const Vector3f& wi, const Vector3f& wo) const
{
    if (!fabs(wi.x + wo.x) <= 1e-6f
        && fabs(wi.y + wo.y) <= 1e-6f
        && fabs(wi.z - wo.z) <= 1e-6f)
    {
        return Color(0.0f, 0.0f, 0.0f);
    }
    float cosTheta = glm::max(wi.z, 1e-4f);
    Color k = mAbsorptionCoef;
    Color r1 = ((mEta * mEta + k * k) * cosTheta * cosTheta - 2.0f * mEta * cosTheta + 1.0f)
        / ((mEta * mEta + k * k) * cosTheta * cosTheta + 2.0f * mEta * cosTheta + 1.0f);

    Color r2 = (mEta * mEta + k * k - 2.0f * mEta * cosTheta + cosTheta * cosTheta)
        / (mEta * mEta + k * k + 2.0f * mEta * cosTheta + cosTheta * cosTheta);

    Color Fr = (r1 + r2) * 0.5f;

    return Fr * mReflectionColor / std::max(std::fabs(cosTheta), 1e-4f);
}

Vector3f ConductorSpecularMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;
    return wi;
}

Color DielectricSpecularMaterial::BRDF(const Vector3f& wi, const Vector3f& wo) const
{
    float cosThetaI = fabs(wi.z);
    float etaI = 1.0f;
    float etaT = 1.0f;
    if (wi.z > 1e-4f)
    {
        etaI = 1.0f;
        etaT = mEta;
    }
    else
    {
        etaI = mEta;
        etaT = 1.0f;
    }
    Vector3f wt;
    float Fr = 1.0f;
    if (ComputeRefractVector(wi, etaI, etaT, wt))
    {
        float cosThetaT = fabs(wt.z);
        Fr = Fresnel(etaI, etaT, cosThetaI, cosThetaT);
    }
    return Color(Fr) / std::max(cosThetaI, 1e-4f);
    
}

Color DielectricSpecularMaterial::BTDF(const Vector3f& wi, const Vector3f& wt) const
{
    float cosThetaI = fabs(wi.z);
    float cosThetaT = fabs(wt.z);
    float etaI = 1.0f;
    float etaT = 1.0f;
    if (wi.z > 1e-4f)
    {
        etaI = 1.0f;
        etaT = mEta;
    }
    else
    {
        etaI = mEta;
        etaT = 1.0f;
    }
    
    float Fr = Fresnel(etaI, etaT, cosThetaI, cosThetaT);
    return mTransmissionColor * (1.0f - Fr) * etaT * etaT / (etaI * etaI * std::max(cosThetaI, 1e-4f));
}

bool DielectricSpecularMaterial::SampleWt(const Vector3f& wi, Vector3f& wt) const
{
    float cosThetaI = fabs(wi.z);
    float etaI = 1.0f;
    float etaT = 1.0f;
    if (wi.z > 1e-4f)
    {
        etaI = 1.0f;
        etaT = mEta;
    }
    else
    {
        etaI = mEta;
        etaT = 1.0f;
    }
    return ComputeRefractVector(wi, etaI, etaT, wt);
}

Vector3f DielectricSpecularMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;
    return wi;
}

float DielectricSpecularMaterial::Fresnel(float etaI, float etaT, float cosThetaI, float cosThetaT)
{
    float r1 = (etaI * cosThetaI - etaT * cosThetaT) / (etaI * cosThetaI + etaT * cosThetaT);
    float r2 = (etaT * cosThetaI - etaI * cosThetaT) / (etaT * cosThetaI + etaI * cosThetaT);
    return 0.5f * (r1 * r1 + r2 * r2);
}


