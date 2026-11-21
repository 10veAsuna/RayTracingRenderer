#include "Material.h"

Color LambertMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    if (wo.z <= 1e-4f || wi.z <= 1e-4f)
        return Color(0.0f, 0.0f, 0.0f);
    return mAlbedo * INV_PI;
}

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

Color ConductorSpecularMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    if (!fabs(wo.x + wi.x) <= 1e-6f
        && fabs(wo.y + wi.y) <= 1e-6f
        && fabs(wo.z - wi.z) <= 1e-6f)
    {
        return Color(0.0f, 0.0f, 0.0f);
    }
    float cosTheta = glm::max(wo.z, 1e-4f);
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

Color DielectricSpecularMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    float cosThetaI = fabs(wo.z);
    float etaI = 1.0f;
    float etaT = 1.0f;
    if (wo.z > 1e-4f)
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
    if (ComputeRefractVector(wo, etaI, etaT, wt))
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

Color OrenNayarMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    if (wi.z <= 1e-4f || wo.z <= 1e-4f)
        return Color(0.0f, 0.0f, 0.0f);
    
    float sigma2 = mSigma * mSigma;
    float A = 1.0f - (sigma2 / 2.0f * (sigma2 + 0.33f));
    float B = 0.45f * sigma2 / (sigma2 + 0.09f);
    
    float cosThetaI = fabs(wi.z);
    float sinThetaI = std::sqrt(glm::max(0.0f, 1.0f - cosThetaI * cosThetaI));
    float cosPhiI = sinThetaI > 1e-6f ? wi.x / sinThetaI : 0.0f;
    float sinPhiI = sinThetaI > 1e-6f ? wi.y / sinThetaI : 0.0f;
    float tanThetaI = sinThetaI / std::max(cosThetaI, 1e-6f);
    
    float cosThetaO = fabs(wo.z);
    float sinThetaO = std::sqrt(glm::max(0.0f, 1.0f - cosThetaO * cosThetaO));
    float cosPhiO = sinThetaO > 1e-6f ? wo.x / sinThetaO : 0.0f;
    float sinPhiO = sinThetaO > 1e-6f ? wo.y / sinThetaO : 0.0f;
    float tanThetaO = sinThetaO / std::max(cosThetaO, 1e-6f);
    
    float sinAltha = glm::max(sinThetaI, sinThetaO);
    float tanBeta = glm::min(tanThetaI, tanThetaO);
    float cosDeltaPhi = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
    
    return mAlbedo * INV_PI * (A + B * glm::max(0.0f, cosDeltaPhi) * sinAltha * tanBeta);
}

Vector3f OrenNayarMaterial::SampleWi(const Vector3f& wo, float& pdf) const
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

Color TorranceSparrowMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    if (wo.z <= 1e-4f || wi.z <= 1e-4f)
        return Color(0.0f, 0.0f, 0.0f);
    
    Vector3f wh = glm::normalize(wo + wi);
    
    //D
    float D = (mExponent + 2) * glm::pow(wh.z, mExponent) / (2 * PI);
    //G
    float deno = glm::max(1e-5f, glm::dot(wo, wh));
    float G = glm::min(1.0f, glm::min(2 * wh.z * wo.z / deno, 2 * wh.z * wi.z / deno));
    //F
    float cosTheta = glm::max(glm::dot(wh, wi), 1e-4f);
    Color k = mAbsorptionCoef;
    Color r1 = ((mEta * mEta + k * k) * cosTheta * cosTheta - 2.0f * mEta * cosTheta + 1.0f)
        / ((mEta * mEta + k * k) * cosTheta * cosTheta + 2.0f * mEta * cosTheta + 1.0f);

    Color r2 = (mEta * mEta + k * k - 2.0f * mEta * cosTheta + cosTheta * cosTheta)
        / (mEta * mEta + k * k + 2.0f * mEta * cosTheta + cosTheta * cosTheta);

    Color F = (r1 + r2) * 0.5f;
    
    return mReflectionColor * D * F * G / (4 * wo.z * wi.z);
}

Vector3f TorranceSparrowMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    float e1 = Random01();
    float e2 = Random01();
    float cosThetaH = glm::pow(e1, 1.0f / (mExponent + 1));
    float sinThetaH = glm::sqrt(std::max(0.0f, 1.0f - cosThetaH * cosThetaH));
    float phiH = 2.0f * PI * e2;
    
    Vector3f wh;
    wh.x = sinThetaH * cos(phiH);
    wh.y = sinThetaH * sin(phiH);
    wh.z = cosThetaH;
    
    Vector3f wi = 2.0f * glm::dot(wo, wh) * wh - wo;
    float pdfH = (mExponent + 1) * (glm::pow(cosThetaH, mExponent)) / (2 * PI);
    float pdfI = pdfH / glm::max(1e-4f, 4 * glm::dot(wo, wh));
    pdf = pdfI;
    return wi;
}


