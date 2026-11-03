#pragma once
#include "Common.h"
#include <atomic>
#include "Camera.h"
#include "Disk.h"
#include "Sphere.h"
#include "Triangle.h"

class Renderer
{
public:
    Renderer(int w, int h);
	virtual ~Renderer() {}
	void Run();
	
private:
	Color RenderPixel(int x, int y);
	void RunRenderThread();
	
	int mViewportWidth;
	int mViewportHeight;
	uint32_t *mBuffer = nullptr;
	
	std::atomic<int> mCurrentPixelIndex = 0;
	
	Camera mCamera;
	
	Sphere* mSphere;
	Disk* mDisk;
	Triangle* mTriangle;
};
