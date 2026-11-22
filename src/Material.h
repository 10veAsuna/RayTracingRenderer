#pragma once
#include <complex>

#include "Ray.h"
#include "Texture.h"

class Material
{
public:
    virtual Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual Color BTDF(const Vector3f& wi, const Vector3f& wo) const {return Color(0.0f, 0.0f, 0.0f);}
    virtual bool IsSpecular() const {return false;}
    virtual bool SampleWt(const Vector3f& wo, Vector3f& wt) const  {return false;}
    virtual ~Material() = default;
    virtual Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const {return Vector3f(0.0f, 0.0f, 0.0f);};
};

//兰伯特
class LambertMaterial : public Material
{
public:
    LambertMaterial(const Color& albedo, Texture* pAlbedoMap) : mAlbedo(albedo), pAlbedoMap(pAlbedoMap) {}
    Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const override;
    virtual  Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const override;
private:
    Color mAlbedo;
    Texture* pAlbedoMap = nullptr;
};

//导体镜面材质
class ConductorSpecularMaterial : public Material
{
public:
    ConductorSpecularMaterial(const Color& eta, const Color& absorptionCoef, const Color& reflectionColor)
        : mEta(eta), mAbsorptionCoef(absorptionCoef), mReflectionColor(reflectionColor) {}
    bool IsSpecular() const override {return true;}
    Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const override;
    virtual Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const override;
private:
    Color mEta;
    Color mAbsorptionCoef;
    Color mReflectionColor;
};

//绝缘体镜面材质
class DielectricSpecularMaterial : public Material
{
public:
    DielectricSpecularMaterial(float eta, Color transmissionColor) :
        mEta(eta), mTransmissionColor(transmissionColor){}
    virtual bool IsSpecular() const override {return true;}
    virtual Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const override;
    virtual Color BTDF(const Vector3f& wi, const Vector3f& wt) const override;
    virtual bool SampleWt(const Vector3f& wi, Vector3f& wt) const override;
    virtual Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const override;
private:
    static float Fresnel(float etaI, float etaT, float cosThetaI, float cosThetaT);
    float mEta;
    Color mTransmissionColor;
};

//Oren-Nayar
class OrenNayarMaterial : public Material
{
public:
    OrenNayarMaterial(const Color& albedo, Texture* pAlbedoMap, float sigma)
        : mAlbedo(albedo), mSigma(sigma), pAlbedoMap(pAlbedoMap) {}
    Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const override;
    Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const override;
private:
    Color mAlbedo;
    Texture* pAlbedoMap = nullptr;
    float mSigma;
};

//Torrance-Sparrow
class TorranceSparrowMaterial : public Material
{
public:
    TorranceSparrowMaterial(float e, const Color& eta, const Color& absorptionCoef, const Color& reflectionColor)
        : mExponent(e), mEta(eta), mAbsorptionCoef(absorptionCoef), mReflectionColor(reflectionColor) {}
    bool IsSpecular() const override {return true;}
    Color BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const override;
    virtual Vector3f SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const override;
private:
    float mExponent;
    Color mEta;
    Color mAbsorptionCoef;
    Color mReflectionColor;
};
