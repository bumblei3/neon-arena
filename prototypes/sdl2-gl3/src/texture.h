// texture.h - OpenGL texture loader using stb_image
#pragma once
#include <GL/glew.h>
#include <string>

class Texture {
public:
    Texture();
    ~Texture();

    bool loadFromFile(const std::string& path);
    void bind(int unit = 0) const;
    void unbind() const;

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    GLuint getHandle() const { return handle_; }

private:
    GLuint handle_ = 0;
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
};
