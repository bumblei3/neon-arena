// renderer.h - OpenGL 3.3+ Core Profile renderer with bloom
#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <vector>
#include "shader.h"
#include "math.h"
#include "particle.h"

struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec3 color;
    Vertex(Vec3 p = {}, Vec3 n = {}, Vec3 c = {}) : pos(p), normal(n), color(c) {}
};

class Renderer {
public:
    bool init(SDL_Window* window, int width, int height);
    void shutdown();

    void resize(int width, int height);

    void beginFrame();
    void endFrame();

    void drawLineLoop(const Vertex* verts, int count, const Vec3& color);
    void drawTriangles(const Vertex* verts, int count, const Vec3& color);
    void drawQuad(const Vertex* verts);
    void drawParticles(const Particle* particles, int count);

    void clear(float r, float g, float b, float a = 1.0f);

    void setProjection(const Mat4& proj) { projection = proj; }
    void setView(const Mat4& view) { this->view = view; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

private:
    void setupVBOs();
    void setupBloom();

    int width_, height_;
    Mat4 projection;
    Mat4 view;

    // VBOs
    unsigned int VAOVertex = 0, VBOVertex = 0;
    unsigned int VAOQuad = 0, VBOQuad = 0;

    // Shaders
    Shader* lineShader = nullptr;
    Shader* triangleShader = nullptr;
    Shader* hudShader = nullptr;
    Shader* brightPassShader = nullptr;
    Shader* blurShader = nullptr;
    Shader* compositeShader = nullptr;

    // Bloom framebuffer
    unsigned int fbo = 0;
    unsigned int fboTexture = 0;
    unsigned int brightFbo = 0;
    unsigned int brightTexture = 0;
    unsigned int blurFbo[2] = {0, 0};
    unsigned int blurTexture[2] = {0, 0};
};
