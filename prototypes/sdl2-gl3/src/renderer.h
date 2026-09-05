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
    float u, v;
    Vertex(Vec3 p = {}, Vec3 n = {}, Vec3 c = {}, float u = 0, float v = 0)
        : pos(p), normal(n), color(c), u(u), v(v) {}
};

class Renderer {
public:
    bool init(SDL_Window* window, int width, int height);
    void shutdown();

    void resize(int width, int height);

    void beginFrame();
    void endFrame();
    void beginOverlay();

    void drawLineLoop(const Vertex* verts, int count, const Vec3& color);
    void drawTriangles(const Vertex* verts, int count, const Vec3& color);
    void drawQuad(const Vertex* verts);
    void drawParticles(const Particle* particles, int count);
    void drawParticlesInstanced(const Particle* particles, int count);
    void drawParticlesECS(const float* data, int count);

    void drawGround(float time);

    void clear(float r, float g, float b, float a = 1.0f);

    void setProjection(const Mat4& proj) { projection = proj; }
    void setView(const Mat4& view) { this->view = view; }
    void setViewPos(const Vec3& pos) { viewPos = pos; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Post-processing setters
    void setHitFlash(float intensity) { hitFlashIntensity = intensity; }
    void setGameOverVignette(float amount) { gameOverVignette = amount; }
    void setChromaticAberration(float amount) { chromaticAberrationAmount = amount; }
    void setBloomThreshold(float t) { bloomThreshold = t; }
    void setBloomIntensity(float i) { bloomIntensity = i; }
    void setupBloom();

private:
    void setupVBOs();
    void setupFullscreenQuad();
    void drawFullscreenQuad();

    int width_, height_;
    Mat4 projection;
    Mat4 view;
    Vec3 viewPos;

    // VBOs
    unsigned int VAOVertex = 0, VBOVertex = 0;
    unsigned int VAOQuad = 0, VBOQuad = 0;

    // Persistent fullscreen quad VAO/VBO (for post-processing)
    unsigned int fsQuadVAO = 0, fsQuadVBO = 0;

    // Instanced rendering
    unsigned int instanceVBO = 0;
    unsigned int particleVAO = 0;
    unsigned int particleVBO = 0;

    // Shaders
    Shader* lineShader = nullptr;
    Shader* triangleShader = nullptr;
    Shader* hudShader = nullptr;
    Shader* brightPassShader = nullptr;
    Shader* blurShader = nullptr;
    Shader* compositeShader = nullptr;
    Shader* groundShader = nullptr;

    // Bloom framebuffer
    unsigned int fbo = 0;
    unsigned int fboTexture = 0;
    unsigned int fboDepth = 0;
    unsigned int brightFbo = 0;
    unsigned int brightTexture = 0;
    unsigned int blurFbo[3] = {0, 0, 0};      // 3 blur stages
    unsigned int blurTexture[3] = {0, 0, 0};  // small, medium, large
    unsigned int bloomResultFbo = 0;
    unsigned int bloomResultTexture = 0;

public:
    // Post-processing parameters (public for game logic access)
    float hitFlashIntensity = 0.0f;
    float gameOverVignette = 0.0f;
    float chromaticAberrationAmount = 0.0f;
    float bloomThreshold = 0.6f;
    float bloomIntensity = 0.8f;
};
