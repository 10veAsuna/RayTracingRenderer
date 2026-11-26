#include "Material.h"

#include <cmath>


Color LambertMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
{
    if (wo.z <= 1e-4f || wi.z <= 1e-4f)
        return Color(0.0f, 0.0f, 0.0f);

    Vector3f albedo = mAlbedo;
    if (pAlbedoMap)
    {
        albedo *= pAlbedoMap->Sample<Color>(uv);
    }
    return albedo * INV_PI;
}

Vector3f LambertMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color ConductorSpecularMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

Vector3f ConductorSpecularMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;
    return wi;
}

Color DielectricSpecularMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

Vector3f DielectricSpecularMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color OrenNayarMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

    Vector3f albedo = mAlbedo;
    if (pAlbedoMap)
    {
        albedo *= pAlbedoMap->Sample<Color>(uv);
    }

    return albedo * INV_PI * (A + B * glm::max(0.0f, cosDeltaPhi) * sinAltha * tanBeta);
}

Vector3f OrenNayarMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color TorranceSparrowMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

Vector3f TorranceSparrowMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

float CookTorranceMaterial::DistributionGGX(float roughness, const Vector3f& wh)
{
	float a = roughness * roughness;
    float a2 = a * a;
    float denom = wh.z * wh.z * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / std::max(denom, 1e-8f);
}

Color CookTorranceMaterial::FresnelSchlick(float metallic, const Vector3f& albedo,
	const Vector3f& wo, const Vector3f& wh)
{
	Color F0 = glm::mix(Color(0.04f), albedo, metallic);
	float cos_h_v = glm::clamp(glm::dot(wo, wh), 0.0f, 1.0f);
	return F0 + (Color(1.0f) - F0) * std::pow(1.0f - cos_h_v, 5.0f);
}

float CookTorranceMaterial::GeometrySmith(float roughness, const Vector3f& wi, const Vector3f& wo)
{
	float k = (roughness + 1) * (roughness + 1) / 8.0f;
	return GeometrySchlickGGX(k, std::abs(wi.z)) * GeometrySchlickGGX(k, std::abs(wo.z));
}

float CookTorranceMaterial::GeometrySchlickGGX(float k, float NdotV)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / std::max(denom, 1e-5f);
}

Color CookTorranceMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
{
	static const float epsilon = 1e-5f;
	if (wo.z <= epsilon || wi.z <= epsilon)
		return Color(0);

	Color albedo = mAlbedo;
	if (m_pAlbedoMap)
		albedo *= m_pAlbedoMap->Sample<Color>(uv);

	float roughness = mRoughness;
	if (m_pRoughnessMap)
		roughness *= m_pRoughnessMap->Sample<float>(uv);

	// roughness一旦变为0会导致D项完全变为0，所以做一个clamp：
	roughness = glm::clamp(roughness, 0.03f, 1.0f);

	float metallic = mMetallic;
	if (m_pMetallicMap)
		metallic *= m_pMetallicMap->Sample<float>(uv);

	Vector3f wh = glm::normalize(wi + wo);

	// D:
	float D = DistributionGGX(roughness, wh);

	// F:
	Color F = FresnelSchlick(metallic, albedo, wo, wh);

    // G:
    float G = GeometrySmith(roughness, wi, wo);

	Color Kd = (Color(1.0f) - F) * (1.0f - metallic);
	return Kd * albedo / PI + F * G * D / (4.0f * wo.z * wi.z);
}

Vector3f CookTorranceMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
{
	float roughness = mRoughness;
	if (m_pRoughnessMap)
		roughness *= m_pRoughnessMap->Sample<float>(uv);
	roughness = glm::clamp(roughness, 0.03f, 1.0f);

	float metallic = mMetallic;
	if (m_pMetallicMap)
		metallic *= m_pMetallicMap->Sample<float>(uv);

	float a = roughness * roughness;
	float a2 = a * a;
	Vector3f wi;

	float e1 = Random01();
	float e2 = Random01();

	float weight = 0.5f;

	float p1 = 0, p2 = 0;

	if (Random01() < weight)
	{
		float cos_theta_h = std::sqrt(std::max(0.0f, (1.0f - e1) /
			std::max(1e-5f, e1 * (a2 - 1.0f) + 1.0f)));
		float sin_theta_h = std::sqrt(std::max(0.0f, 1.0f - cos_theta_h * cos_theta_h));
		float phi_h = e2 * 2.0f * PI;

		Vector3f wh;
		wh.x = std::cos(phi_h) * sin_theta_h;
		wh.y = std::sin(phi_h) * sin_theta_h;
		wh.z = cos_theta_h;

		float woDotWh = glm::dot(wo, wh);
		if (woDotWh <= 0.0f)
		{
			pdf = 0.0f;
			return Vector3f(0.0f, 0.0f, 1.0f);
		}

		wi = 2.0f * woDotWh * wh - wo;
		if (wi.z <= 0.0f)
		{
			pdf = 0.0f;
			return wi;
		}

		// p(wi) = p(wh) / (4 * dot(wo,wh))
		p2 = DistributionGGX(roughness, wh) * cos_theta_h / (4.0f * woDotWh);
		p1 = wi.z * INV_PI;
	}
	else
	{
		float cosTheta = sqrt(1.0f - e1);
		float sinTheta = sqrt(e1);
		float phi = 2.0f * PI * e2;

		wi.x = sinTheta * cos(phi);
		wi.y = sinTheta * sin(phi);
		wi.z = cosTheta;

		p1 = cosTheta * INV_PI;
		Vector3f wh = glm::normalize(wo + wi);
		float woDotWh = glm::dot(wo, wh);
		if (woDotWh > 0 && wh.z > 0)
		{
			p2 = DistributionGGX(roughness, wh) * wh.z / (4.0f * woDotWh);
		}
	}

	pdf = p1 * (1 - weight) + p2 * weight;
	return wi;
}
