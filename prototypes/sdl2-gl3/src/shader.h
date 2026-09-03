// shader.h - OpenGL shader wrapper
#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>

class Shader {
public:
    unsigned int id;

    Shader() = default;
    ~Shader();

    bool load(const std::string& vertexSrc, const std::string& fragSrc);
    bool loadFromFile(const std::string& vertexPath, const std::string& fragPath);

    void use() const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setMat4(const std::string& name, const float* mat) const;

private:
    unsigned int compileShader(GLenum type, const std::string& source);
    std::string readFile(const std::string& path);
};
