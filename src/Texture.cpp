#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture* Texture::CreateFromFile(const std::string& filePath)
{
    int width, height, channels;
    uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
    if (data == nullptr)
        return nullptr;
    Texture* tex = new Texture(width, height, channels, data);
    stbi_image_free(data);
    return tex;
}
