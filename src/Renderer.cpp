#include "Renderer.h"
#include <MiniFB.h>
#include "Common.h"
#include <thread>
#include <vector>

#include "SceneObject.h"
#include "SceneXmlLoader.h"

Renderer::Renderer(int w, int h, int minDepth, int maxDepth, int samplePerPixel, const std::string& scenePath)
	: mViewportWidth(w), mViewportHeight(h), mMinDepth(minDepth), mMaxDepth(maxDepth)
{
	mCurrentPixelIndex = 0;
	SamplePerPixel = samplePerPixel;
	mScene = LoadSceneFromXml(scenePath, w, h).release();
}

Renderer::~Renderer()
{
	if (mScene)
	{
		delete mScene;
	}
}

void Renderer::Run()
{
	struct mfb_window *window = mfb_open_ex("MyRenderer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
	if (window == NULL)
		return;
	//视口上每个像素的颜色，以32位整数表示
	mBuffer = (uint32_t*)malloc(mViewportWidth * mViewportHeight * 4);
	
	std::thread renderThread(&Renderer::RunRenderThread, this); 
	renderThread.detach();
	
	int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> renderThreads(numThreads);
	for (int i = 0; i < numThreads; i++)
	{
		renderThreads[i] = std::thread(&Renderer::RunRenderThread, this);
		renderThreads[i].detach();
	}
	
	
	//Present:
	mfb_update_state state;
	do {
		// TODO: add some fancy rendering to the buffer of size mViewportWidth * mViewportWidth

		state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);

		if (state != MFB_STATE_OK)
			break;

	} while(mfb_wait_sync(window));
	
	
	free(mBuffer);
	mBuffer = NULL;
	window = NULL;
	
}

Color Renderer::RenderPixel(int x, int y)
{
	//SSAA
	const int N = SamplePerPixel; //每个像素采样的次数
	Color resultColor = Color(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < N; ++i)
	{
		//（x, y） - （x + 1, y + 1）范围内随机采样一个点
		float px = x + Random01();
		float py = y + Random01();
		
		Color color = RenderSubPixel(px, py);
		resultColor += color / (float)N;
	}
	return resultColor; //平均值
}

Color Renderer::RenderSubPixel(float x, float y)
{
	Ray ray = mScene->GetCamera().GetRay(x, y);
	Color color = GetRadiance(ray, 0, false);
	return color;
}

/*
Color Renderer::GetIrradiance(const Ray& ray)
{
	Intersection isect;
	if (!mScene->Intersect(ray, isect))
		return Color(0.0f, 0.0f, 0.0f);
	Color E = Color(0.0f, 0.0f, 0.0f);
	//E(p)
	for (Light* pLight : mScene->GetLights())
	{
		Vector3f sourcePos;
		Color L = pLight->GetRadiance(isect.position, sourcePos, TODO);
		
		Ray shadowRay;
		shadowRay.origin = isect.position;
		shadowRay.direction = glm::normalize(sourcePos - isect.position);
		shadowRay.mint = 1e-4f;
		shadowRay.maxt =glm::length(sourcePos - isect.position);
		
		if (mScene->Intersect(shadowRay, isect))
			continue;
		float cosTheta = glm::dot(isect.normal, shadowRay.direction);
		E += L * glm::max(cosTheta, 0.0f);
	}
	return E;
}
*/

Color Renderer::GetRadiance(const Ray& ray, int depth, bool bPrevIsSpecular)
{
	if (depth > mMaxDepth)
		return Color(0.0f, 0.0f, 0.0f);
	
	//俄罗斯轮盘
	static const float SurvivalProbability = 0.8f;
	float rewardFactor = 1.0f;
	if (depth >= mMinDepth)
	{
		float K = Random01();
		if (K > SurvivalProbability)
		{
			return Color(0.0f, 0.0f, 0.0f);
		}
		rewardFactor = 1.0f / SurvivalProbability;
	}
	
	Intersection isect;
	SceneObject* pSceneObject = mScene->Intersect(ray, isect);
	if (pSceneObject == nullptr)
		return Color(0.0f, 0.0f, 0.0f);
	
	Material* pMaterial = pSceneObject->GetMaterial();
	
	Color Lo = Color(0.0f, 0.0f, 0.0f);
	
	Matrix3x3 localToWorld = MakeCoordinateSystem(isect.normal);
	Matrix3x3 worldToLocal = glm::inverse(localToWorld);
	Vector3f wo = worldToLocal * (-ray.direction);
	
	//自发光
	Color Le = pSceneObject->GetEmissive();
	if (bPrevIsSpecular || depth == 0)
	{
		Lo += Le;
	}

	
	//直接光照
	if (!pMaterial->IsSpecular())
	{
		for (Light* pLight : mScene->GetLights())
		{
			Vector3f sourcePos;
			float pdf;
			Color L = pLight->GetRadiance(isect.position, sourcePos, pdf);
		
			Ray shadowRay;
			shadowRay.origin = isect.position;
			shadowRay.direction = glm::normalize(sourcePos - isect.position);
			shadowRay.mint = 1e-4f;
			shadowRay.maxt =glm::length(sourcePos - isect.position) - 1e-5f;
			
			Intersection shadowIsect;
			if (mScene->Intersect(shadowRay, shadowIsect))
				continue;
			Vector3f wi = worldToLocal * shadowRay.direction;
			float cosTheta = glm::dot(isect.normal, shadowRay.direction);
			Color brdf = pMaterial->BRDF(isect.uv, wo, wi);
			Lo += brdf * L * glm::max(cosTheta, 0.0f) / std::max(1e-4f, pdf);
		}
	}
	//间接光照
	//反射
	{
		float pdf;
		Vector3f wi = pMaterial->SampleWi(isect.uv, wo, pdf);
	
		Color brdf = pMaterial->BRDF(isect.uv, wo, wi);
		Ray r;
		r.direction = localToWorld * wi;
		r.origin = isect.position;
		r.mint = 1e-4f;
		Color Li = GetRadiance(r, depth + 1, pMaterial->IsSpecular());
		Lo += brdf * Li * std::fabs(wi.z) / std::max(1e-4f, pdf);
	}
	//折射
	{
		Vector3f wi;
		if (pMaterial->SampleWt(wo, wi))
		{
			Color btdf = pMaterial->BTDF(wo, wi);
			Ray r;
			r.direction = localToWorld * wi;
			r.origin = isect.position;
			r.mint = 1e-4f;
			Color Li = GetRadiance(r, depth + 1, pMaterial->IsSpecular());
			Lo += btdf * Li * std::fabs(wi.z);
		}
	}
	return Lo * rewardFactor;
}

//渲染线程入口函数
void Renderer::RunRenderThread()
{
	//读取当前屏幕的下一个像素
	while (true)
	{
		int pixelIndex = mCurrentPixelIndex.fetch_add(1);
		if (pixelIndex >= mViewportWidth * mViewportHeight) break;
		
		int x = pixelIndex % mViewportWidth;
		int y = pixelIndex / mViewportWidth;
		
		Color color = RenderPixel(x, y);
		uint32_t r = glm::clamp((uint32_t)std::round(color.r * 255), 0u, 255u);
		uint32_t g = glm::clamp((uint32_t)std::round(color.g * 255), 0u, 255u);
		uint32_t b = glm::clamp((uint32_t)std::round(color.b * 255), 0u, 255u);
		mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | (b);
	}
}
 
