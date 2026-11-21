#pragma once
#include <complex>

#include "Ray.h"

class Material
{
public:
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const = 0;
    virtual Color BTDF(const Vector3f& wi, const Vector3f& wo) const {return Color(0.0f, 0.0f, 0.0f);}
    virtual bool IsSpecular() const {return false;}
    virtual bool SampleWt(const Vector3f& wo, Vector3f& wt) const  {return false;}
    virtual ~Material() = default;
    virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const {return Vector3f(0.0f, 0.0f, 0.0f);};
};

//兰伯特
class LambertMaterial : public Material
{
public:
    LambertMaterial(const Color& albedo) : mAlbedo(albedo) {}
    Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual  Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;
private:
    Color mAlbedo;
};

//导体镜面材质
class ConductorSpecularMaterial : public Material
{
public:
    ConductorSpecularMaterial(const Color& eta, const Color& absorptionCoef, const Color& reflectionColor)
        : mEta(eta), mAbsorptionCoef(absorptionCoef), mReflectionColor(reflectionColor) {}
    bool IsSpecular() const override {return true;}
    Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;
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
    virtual Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Color BTDF(const Vector3f& wi, const Vector3f& wt) const override;
    virtual bool SampleWt(const Vector3f& wi, Vector3f& wt) const override;
    virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;
private:
    static float Fresnel(float etaI, float etaT, float cosThetaI, float cosThetaT);
    float mEta;
    Color mTransmissionColor;
};

//Oren-Nayar
class OrenNayarMaterial : public Material
{
public:
    OrenNayarMaterial(const Color& albedo, float sigma)
        : mAlbedo(albedo), mSigma(sigma) {}
    Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;
private:
    Color mAlbedo;
    float mSigma;
};

//Torrance-Sparrow
class TorranceSparrowMaterial : public Material
{
public:
    TorranceSparrowMaterial(float e, const Color& eta, const Color& absorptionCoef, const Color& reflectionColor)
        : mExponent(e), mEta(eta), mAbsorptionCoef(absorptionCoef), mReflectionColor(reflectionColor) {}
    bool IsSpecular() const override {return true;}
    Color BRDF(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Vector3f SampleWi(const Vector3f& wo, float& pdf) const override;
private:
    float mExponent;
    Color mEta;
    Color mAbsorptionCoef;
    Color mReflectionColor;
};
