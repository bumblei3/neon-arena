// shader.cpp - OpenGL shader wrapper implementation
#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::~Shader() {
    if (id) {
        glDeleteProgram(id);
    }
}

bool Shader::load(const std::string& vertexSrc, const std::string& fragSrc) {
    unsigned int vert = compileShader(GL_VERTEX_SHADER, vertexSrc);
    unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);

    int success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(id, 512, nullptr, log);
        std::cerr << "Shader link error: " << log << std::endl;
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragPath) {
    return load(readFile(vertexPath), readFile(fragPath));
}

void Shader::use() const {
    glUseProgram(id);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value ? 1 : 0);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string& name, const float* mat) const {
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, mat);
}

unsigned int Shader::compileShader(GLenum type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error (" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "): " << log << std::endl;
        return 0;
    }
    return shader;
}

std::string Shader::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
