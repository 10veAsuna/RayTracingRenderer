#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "Common.h"

class Texture
{
public:
    Texture(int width, int height, int channels, uint8_t* data)
        : mWidth(width), mHeight(height), mChannels(channels)
    {
        mData.resize(width * height * channels);
        memcpy(mData.data(), data, width * height * channels);
        
        Texture* pTexture = nullptr;
    }
    
    static Texture* CreateFromFile(const std::string& filePath);

    template <typename T>
    T GetPixel(int x, int y)
    {
        int x = glm::clamp(x, 0, mWidth - 1);
        int y = glm::clamp(y, 0, mHeight - 1);
        int index = y * mWidth + x * mChannels;
        
        if constexpr (std::is_same_v<T, float>)
        {
            return mData[index] / 255.0f;
        }
        else
        {
            T result(0);
            int count = std::min(T::length(), mChannels);
            for (int i = 0; i < count; i++)
            {
                //result[0] == result.x == result.r
                result[i] = mData[index + i] / 255.0f;
            }
            return result;
        }
    }
private:
    int mWidth;
    int mHeight;
    int mChannels;
    std::vector<uint8_t> mData;
};
