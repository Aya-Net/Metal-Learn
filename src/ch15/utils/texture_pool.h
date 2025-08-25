//
// Created by GUO XIUYI on 2025/8/22.
//

#ifndef MYMETALAPP_TEXTURE_POOL_H
#define MYMETALAPP_TEXTURE_POOL_H

#include <Metal/Metal.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>

#include <iostream>
#include <map>




class TexturePool {

  private:
    std::vector<MTL::Texture*> textures_;
    std::map<std::string, unsigned int> texture_map_; // Map to store loaded textures with their file paths as keys

  public:
    unsigned int loadTexture(const std::string &imagePath, MTL::Device* device, bool needFlip = false) {
        std::string normalizedPath = imagePath;
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/'); // Normalize path for consistency
        stbi_set_flip_vertically_on_load(needFlip);
        if (texture_map_.find(normalizedPath) != texture_map_.end()) {
            std::cout << "Texture already loaded: " << normalizedPath << std::endl;
            return texture_map_[normalizedPath];
        }
        std::cout << "Loading texture: " << normalizedPath << std::endl;
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(normalizedPath.c_str(), &texWidth, &texHeight, &texChannels, 0);
        std::cout << "Texture size: " << texWidth << "x" << texHeight << ", Channels: " << texChannels << std::endl;
        if (!pixels) {
            std::cerr << "Failed to load texture image: " << normalizedPath << std::endl;
            return -1;
        }
        if (texChannels == 3) {
            std::cout << "Texture loaded with RGB channels, converting to RGBA." << std::endl;
            stbi_uc* newPixels = (stbi_uc*) malloc(texWidth * texHeight * 4);
            for (int i = 0; i < texWidth * texHeight; ++i) {
                newPixels[i * 4] = pixels[i * 3];
                newPixels[i * 4 + 1] = pixels[i * 3 + 1];
                newPixels[i * 4 + 2] = pixels[i * 3 + 2];
                newPixels[i * 4 + 3] = 255; // Set alpha to fully opaque
            }
            stbi_image_free(pixels);
            pixels = newPixels;
        } else if (texChannels != 4) {
            std::cerr << "Unsupported texture format: " << texChannels << " channels." << std::endl;
            stbi_image_free(pixels);
            return -1; // Return an invalid index
        }

        MTL::TextureDescriptor* texDesc = MTL::TextureDescriptor::texture2DDescriptor(
                MTL::PixelFormatRGBA8Unorm,
                texWidth,
                texHeight,
                false
        );
        texDesc->setUsage(MTL::TextureUsageShaderRead);
        MTL::Texture* texture = device->newTexture(texDesc);
        MTL::Region region = MTL::Region(0, 0, 0, texWidth, texHeight, 1);
        texture->replaceRegion(region, 0, pixels, texWidth * 4);

        textures_.push_back(texture);
        texture_map_[normalizedPath] = textures_.size() - 1;

        texDesc->release();
        stbi_image_free(pixels);

        return textures_.size() - 1;
    }
    MTL::Texture* getTexture(unsigned int index) {
        if (index >= textures_.size()) {
            std::cerr << "Texture index out of bounds: " << index << std::endl;
            return nullptr;
        }
        return textures_[index];
    }

    ~TexturePool() {
        for (auto texture : textures_) {
            if (texture) {
                texture->release();
            }
        }
    }
};

#endif //MYMETALAPP_TEXTURE_POOL_H
