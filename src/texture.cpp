/*
 * Created by Brett Terpstra 6920201 on 27/03/23.
 * Copyright (c) Brett Terpstra 2023 All Rights Reserved
 */
#define GLAD_GL_IMPLEMENTATION
#include <texture.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>


texture* loadTexture(const std::string& path) {
    constexpr int channel_count = 4;
    int width, height, channels;
    auto* data = stbi_load(path.c_str(), &width, &height, &channels, channel_count);
    return new texture(data, width, height, channel_count);
}
