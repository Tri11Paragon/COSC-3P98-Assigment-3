/*
 * Created by Brett Terpstra 6920201 on 27/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */
#include <texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


texture* loadTexture(const std::string& path) {
    constexpr int channel_count = 4;
    int width, height, channels;
    auto* data = stbi_load(path.c_str(), &width, &height, &channels, channel_count);
    channels = channel_count;
    return new texture(data, width, height, channels);
}
