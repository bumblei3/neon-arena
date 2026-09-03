// texture.cpp - OpenGL texture loader using stb_image
#include "texture.h"
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

Texture::Texture() : handle_(0), width_(0), height_(0), channels_(0) {}

Texture::~Texture() {
    if (handle_) {
        glDeleteTextures(1, &handle_);
    }
}

bool Texture::loadFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width_, &height_, &channels_, 0);
    if (!data) {
        fprintf(stderr, "Failed to load texture: %s\n", path.c_str());
        return false;
    }

    GLenum format = GL_RGB;
    if (channels_ == 4) format = GL_RGBA;
    else if (channels_ == 1) format = GL_RED;

    glGenTextures(1, &handle_);
    glBindTexture(GL_TEXTURE_2D, handle_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width_, height_, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return true;
}

void Texture::bind(int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, handle_);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
