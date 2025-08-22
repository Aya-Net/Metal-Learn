//
// Created by GUO XIUYI on 2025/8/21.
//

#ifndef MYMETALAPP_MATERIAL_H
#define MYMETALAPP_MATERIAL_H

#include <Metal/Metal.hpp>
#include <stb_image.h>
#include <glm.hpp>

#include <iostream>

class Material {
public:
    MTL::Texture *diffuseTexture;
    MTL::Texture *specularTexture;
    float shininess;

    Material(MTL::Device *device, const char* diffuseImagePath, const char* specularImagePath, float shine) {
        MTL::TextureDescriptor* texDesc;
        MTL::Region region;
        stbi_uc* pixels;
        int texWidth, texHeight, texChannels;
        // load diffuse texture
        pixels = stbi_load(diffuseImagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        texDesc = MTL::TextureDescriptor::texture2DDescriptor(
                MTL::PixelFormatRGBA8Unorm,
                texWidth,
                texHeight,
                false
        );
        texDesc->setUsage(MTL::TextureUsageShaderRead);
        diffuseTexture = device->newTexture(texDesc);
        region = MTL::Region(0, 0, 0, texWidth, texHeight, 1);
        diffuseTexture->replaceRegion(region, 0, pixels, texWidth * 4);

        texDesc->release();
        stbi_image_free(pixels);

        // load specular texture
        pixels = stbi_load(specularImagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        texDesc = MTL::TextureDescriptor::texture2DDescriptor(
                MTL::PixelFormatRGBA8Unorm,
                texWidth,
                texHeight,
                false
        );
        texDesc->setUsage(MTL::TextureUsageShaderRead);
        specularTexture = device->newTexture(texDesc);
        region = MTL::Region(0, 0, 0, texWidth, texHeight, 1);
        specularTexture->replaceRegion(region, 0, pixels, texWidth * 4);
        texDesc->release();
        stbi_image_free(pixels);

        shininess = shine;
    }



    ~Material() {
        if (diffuseTexture) {
            diffuseTexture->release();
        }
        if (specularTexture) {
            specularTexture->release();
        }
    }
};
#endif //MYMETALAPP_MATERIAL_H
