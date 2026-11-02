#include "Renderer.h"
#include <MiniFB.h>
#include "Common.h"
#include <thread>
#include <vector>

Renderer::Renderer(int w, int h)
	: mViewportWidth(w), mViewportHeight(h)
{
	mCurrentPixelIndex = 0;
	mCamera.Initialize(
		Vector3f(10.0f, 5.0f, 20.0f),
		Vector3f(10.0f, 10.0f, 30.0f),
		Vector3f(0.0f, 1.0f, 0.0f),
		glm::radians(60.0f),
		0.1f,
		1000.0f,
		w,
		h
	);
}

void Renderer::Run()
{
	struct mfb_window *window = mfb_open_ex("MyRenderer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
	if (window == NULL)
		return;
	//视口上每个像素的颜色，以32位整数表示，
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

	renderThread.join();
	
	free(mBuffer);
	mBuffer = NULL;
	window = NULL;
	
}

Color Renderer::RenderPixel(int x, int y)
{
	Ray ray = mCamera.getRay(x, y);
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	return Color(1.0f, 0.3f, 0.8f);
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
 
