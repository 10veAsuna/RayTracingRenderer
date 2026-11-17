#pragma once
#include "Common.h"
#include <atomic>
#include <string>
#include "Camera.h"
#include "Primitive.h"
#include "Sphere.h"
#include "Disk.h"
#include "Scene.h"
#include "Triangle.h"

class Renderer
{
public:
    Renderer(int w, int h, int minDepth, int maxDepth, int samplePerPixel, const std::string& scenePath);
	virtual ~Renderer();
	void Run();
	
private:
	Color RenderPixel(int x, int y);
	Color RenderSubPixel(float x, float y);
	Color GetIrradiance(const Ray& ray);
	Color GetRadiance(const Ray& ray, int depth);
	
	void RunRenderThread();
	
	
	int mViewportWidth;
	int mViewportHeight;
	//spp
	int SamplePerPixel = 64;
	int mMaxDepth = 10;
	int mMinDepth = 3;
	
	uint32_t *mBuffer = nullptr;
	
	std::atomic<int> mCurrentPixelIndex = 0;
	
	Scene* mScene = nullptr;
};
