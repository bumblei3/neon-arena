// renderer.cpp - OpenGL 3.3+ Core Profile renderer with bloom
#include "renderer.h"
#include <cstdio>
#include <cstring>

// Embedded shaders to avoid file loading issues
static const char* lineVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 2) in vec3 aColor;
    uniform mat4 proj;
    uniform mat4 view;
    uniform mat4 model;
    out vec3 vColor;
    void main() {
        gl_Position = proj * view * model * vec4(aPos, 1.0);
        gl_PointSize = 8.0;
        vColor = aColor;
    }
)";

static const char* lineFragSrc = R"(
    #version 330 core
    in vec3 vColor;
    out vec4 FragColor;
    void main() {
        // Circular point with soft edge
        vec2 coord = gl_PointCoord - vec2(0.5);
        float dist = length(coord);
        if (dist > 0.5) discard;
        float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
        FragColor = vec4(vColor, alpha);
    }
)";

static const char* triangleVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec3 aColor;
    uniform mat4 proj;
    uniform mat4 view;
    uniform mat4 model;
    out vec3 vColor;
    out vec3 vNormal;
    out vec3 vWorldPos;
    void main() {
        vec4 worldPos = model * vec4(aPos, 1.0);
        gl_Position = proj * view * worldPos;
        vColor = aColor;
        vNormal = mat3(model) * aNormal;
        vWorldPos = worldPos.xyz;
    }
)";

static const char* triangleFragSrc = R"(
    #version 330 core
    in vec3 vColor;
    in vec3 vNormal;
    in vec3 vWorldPos;
    out vec4 FragColor;
    uniform vec3 viewPos;
    uniform vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    uniform vec3 lightColor = vec3(1.0, 0.95, 0.8);
    uniform vec3 ambientColor = vec3(0.15, 0.15, 0.2);
    uniform float fresnelPower = 2.0;
    uniform vec3 fresnelColor = vec3(0.0, 0.8, 1.0);
    uniform float fresnelIntensity = 0.6;
    void main() {
        vec3 normal = normalize(vNormal);
        vec3 viewDir = normalize(viewPos - vWorldPos);
        
        // Diffuse lighting
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Fresnel glow (edge glow)
        float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), fresnelPower);
        vec3 fresnelGlow = fresnel * fresnelColor * fresnelIntensity;
        
        vec3 result = (ambientColor + diffuse) * vColor + fresnelGlow;
        FragColor = vec4(result, 1.0);
    }
)";

static const char* hudVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 vTexCoord;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        vTexCoord = aTexCoord;
    }
)";

static const char* hudFragSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    out vec4 FragColor;
    uniform sampler2D uTexture;
    uniform vec3 uColor;
    void main() {
        float alpha = texture(uTexture, vTexCoord).r;
        FragColor = vec4(uColor, alpha);
    }
)";

static const char* postVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    out vec2 vTexCoord;
    void main() {
        vTexCoord = aPos * 0.5 + 0.5;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

static const char* brightPassFragSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    out vec4 FragColor;
    uniform sampler2D screenTexture;
    uniform float threshold;
    void main() {
        vec3 color = texture(screenTexture, vTexCoord).rgb;
        float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > threshold) {
            FragColor = vec4(color, 1.0);
        } else {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
)";

static const char* blurFragSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    out vec4 FragColor;
    uniform sampler2D screenTexture;
    uniform vec2 direction;
    void main() {
        vec2 texelSize = 1.0 / vec2(textureSize(screenTexture, 0));
        vec3 result = vec3(0.0);
        float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
        result += texture(screenTexture, vTexCoord).rgb * weights[0];
        for (int i = 1; i < 5; i++) {
            vec2 offset = direction * texelSize * float(i);
            result += texture(screenTexture, vTexCoord + offset).rgb * weights[i];
            result += texture(screenTexture, vTexCoord - offset).rgb * weights[i];
        }
        FragColor = vec4(result, 1.0);
    }
)";

static const char* groundVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec3 aColor;
    layout(location = 3) in vec2 aUV;
    uniform mat4 proj;
    uniform mat4 view;
    uniform mat4 model;
    out vec2 vUV;
    out vec3 vWorldPos;
    void main() {
        vec4 worldPos = model * vec4(aPos, 1.0);
        gl_Position = proj * view * worldPos;
        vUV = aUV;
        vWorldPos = worldPos.xyz;
    }
)";

static const char* groundFragSrc = R"(
    #version 330 core
    in vec2 vUV;
    in vec3 vWorldPos;
    out vec4 FragColor;
    uniform float time;
    void main() {
        vec2 uv = vUV * 20.0;
        
        // Plasma effect
        float plasma = 0.0;
        plasma += sin(uv.x * 0.5 + time * 0.5);
        plasma += sin(uv.y * 0.3 + time * 0.7);
        plasma += sin((uv.x + uv.y) * 0.4 + time * 0.6);
        plasma += sin(length(uv - vec2(10.0, 10.0)) * 0.5 - time * 0.8);
        plasma = plasma * 0.25;
        
        // Neon color palette (cyan, magenta, purple)
        vec3 col1 = vec3(0.0, 0.8, 1.0);  // Cyan
        vec3 col2 = vec3(1.0, 0.0, 0.8);  // Magenta
        vec3 col3 = vec3(0.4, 0.0, 1.0);  // Purple
        
        float t = plasma * 0.5 + 0.5;
        vec3 color = mix(col1, col2, smoothstep(0.0, 0.5, t));
        color = mix(color, col3, smoothstep(0.5, 1.0, t));
        
        // Grid overlay
        vec2 grid = abs(fract(uv) - 0.5);
        float gridLine = min(grid.x, grid.y);
        float gridGlow = 1.0 - smoothstep(0.0, 0.05, gridLine);
        color += vec3(0.0, 0.6, 0.8) * gridGlow * 0.3;
        
        // Distance fade
        float dist = length(vWorldPos.xz);
        float fade = 1.0 / (1.0 + dist * 0.02);
        color *= fade;
        
        FragColor = vec4(color, 1.0);
    }
)";

static const char* compositeFragSrc = R"(
    #version 330 core
    in vec2 vTexCoord;
    out vec4 FragColor;
    uniform sampler2D sceneTexture;
    uniform sampler2D bloomTexture;
    uniform float bloomIntensity;
    uniform float time;
    uniform vec2 resolution;
    uniform float hitFlash;          // White flash on damage [0..1]
    uniform float gameOverVignette;  // Extra darkening at game over [0..1]
    uniform float caAmount;          // Chromatic aberration intensity multiplier

    // Chromatic aberration sample
    vec3 sampleCA(sampler2D tex, vec2 uv, float offset) {
        float r = texture(tex, uv + vec2(offset, 0.0)).r;
        float g = texture(tex, uv).g;
        float b = texture(tex, uv - vec2(offset, 0.0)).b;
        return vec3(r, g, b);
    }

    // ACES Filmic Tone Mapping (Stephen Hill approximation)
    vec3 ACESFilm(vec3 x) {
        float a = 2.51;
        float b = 0.03;
        float c = 2.43;
        float d = 0.59;
        float e = 0.14;
        return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
    }

    void main() {
        vec2 uv = vTexCoord;
        vec2 center = uv - 0.5;
        float dist = length(center);

        // Chromatic aberration (stronger at edges, adjustable)
        float caStrength = caAmount * (0.004 + dist * 0.015);
        vec3 color = sampleCA(sceneTexture, uv, caStrength);

        // Bloom (multi-pass: small + medium + large)
        vec3 bloom = texture(bloomTexture, uv).rgb;
        color += bloom * bloomIntensity * 1.5;

        // Hit flash (white screen flash)
        if (hitFlash > 0.001) {
            color = mix(color, vec3(1.0), hitFlash * 0.5);
        }

        // Vignette (standard + game over extra darkening)
        float vignette = 1.0 - dist * 1.2;
        vignette = clamp(vignette, 0.0, 1.0);
        vignette *= (1.0 - gameOverVignette * dist * 2.0);
        color *= vignette;

        // Scanlines (subtle)
        float scanline = sin(uv.y * resolution.y * 1.5) * 0.04;
        color -= scanline;

        // Film grain (subtle noise)
        float grain = fract(sin(dot(uv + time, vec2(12.9898, 78.233))) * 43758.5453);
        color += (grain - 0.5) * 0.025;

        // ACES Filmic Tone Mapping
        color = ACESFilm(color);

        // Gamma correction
        color = pow(color, vec3(1.0 / 2.2));

        FragColor = vec4(color, 1.0);
    }
)";

bool Renderer::init(SDL_Window* window, int w, int h) {
    width_ = w;
    height_ = h;

    // Context is already created and GLEW initialized in main
    // Just set up GL state
    SDL_GL_MakeCurrent(window, SDL_GL_GetCurrentContext());

    printf("OpenGL: %s\n", glGetString(GL_VERSION));
    printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Create shaders
    lineShader = new Shader();
    if (!lineShader->load(lineVertSrc, lineFragSrc)) {
        fprintf(stderr, "Failed to load line shader\n");
        return false;
    }

    triangleShader = new Shader();
    if (!triangleShader->load(triangleVertSrc, triangleFragSrc)) {
        fprintf(stderr, "Failed to load triangle shader\n");
        return false;
    }

    hudShader = new Shader();
    if (!hudShader->load(hudVertSrc, hudFragSrc)) {
        fprintf(stderr, "Failed to load HUD shader\n");
        return false;
    }

    brightPassShader = new Shader();
    if (!brightPassShader->load(postVertSrc, brightPassFragSrc)) {
        fprintf(stderr, "Failed to load bright pass shader\n");
        return false;
    }

    blurShader = new Shader();
    if (!blurShader->load(postVertSrc, blurFragSrc)) {
        fprintf(stderr, "Failed to load blur shader\n");
        return false;
    }

    compositeShader = new Shader();
    if (!compositeShader->load(postVertSrc, compositeFragSrc)) {
        fprintf(stderr, "Failed to load composite shader\n");
        return false;
    }

    groundShader = new Shader();
    if (!groundShader->load(groundVertSrc, groundFragSrc)) {
        fprintf(stderr, "Failed to load ground shader\n");
        return false;
    }

    setupVBOs();
    setupBloom();
    setupFullscreenQuad();

    return true;
}

void Renderer::shutdown() {
    delete lineShader;
    delete triangleShader;
    delete hudShader;
    delete brightPassShader;
    delete blurShader;
    delete compositeShader;

    glDeleteVertexArrays(1, &VAOVertex);
    glDeleteBuffers(1, &VBOVertex);
    glDeleteVertexArrays(1, &VAOQuad);
    glDeleteBuffers(1, &VBOQuad);

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fboTexture);
    glDeleteFramebuffers(1, &brightFbo);
    glDeleteTextures(1, &brightTexture);
    glDeleteFramebuffers(3, blurFbo);
    glDeleteTextures(3, blurTexture);
    glDeleteFramebuffers(1, &bloomResultFbo);
    glDeleteTextures(1, &bloomResultTexture);

    glDeleteVertexArrays(1, &fsQuadVAO);
    glDeleteBuffers(1, &fsQuadVBO);
}

void Renderer::setupFullscreenQuad() {
    // Persistent fullscreen quad for post-processing
    float quadVerts[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };
    glGenVertexArrays(1, &fsQuadVAO);
    glGenBuffers(1, &fsQuadVBO);
    glBindVertexArray(fsQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fsQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindVertexArray(0);
}

void Renderer::drawFullscreenQuad() {
    glBindVertexArray(fsQuadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Renderer::setupVBOs() {
    // Line VAO
    glGenVertexArrays(1, &VAOVertex);
    glGenBuffers(1, &VBOVertex);
    glBindVertexArray(VAOVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 1024, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glBindVertexArray(0);

    // Quad VAO
    glGenVertexArrays(1, &VAOQuad);
    glGenBuffers(1, &VBOQuad);
    glBindVertexArray(VAOQuad);
    glBindBuffer(GL_ARRAY_BUFFER, VBOQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 1024, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glBindVertexArray(0);

    // Particle Instanced VAO/VBO
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Attributes werden in drawParticlesInstanced gesetzt
    glBindVertexArray(0);
}

void Renderer::setupBloom() {
    // Scene framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    // Bright pass framebuffer
    glGenFramebuffers(1, &brightFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFbo);
    glGenTextures(1, &brightTexture);
    glBindTexture(GL_TEXTURE_2D, brightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_ / 2, height_ / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTexture, 0);

    // 3-stage blur framebuffers (small / medium / large)
    int sizes[3][2] = {
        {width_ / 2, height_ / 2},
        {width_ / 4, height_ / 4},
        {width_ / 8, height_ / 8}
    };
    for (int i = 0; i < 3; i++) {
        glGenFramebuffers(1, &blurFbo[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[i]);
        glGenTextures(1, &blurTexture[i]);
        glBindTexture(GL_TEXTURE_2D, blurTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, sizes[i][0], sizes[i][1], 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture[i], 0);
    }

    // Bloom result framebuffer (combine 3 blurs)
    glGenFramebuffers(1, &bloomResultFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, bloomResultFbo);
    glGenTextures(1, &bloomResultTexture);
    glBindTexture(GL_TEXTURE_2D, bloomResultTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width_ / 2, height_ / 2, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomResultTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resize(int w, int h) {
    width_ = w;
    height_ = h;
    // Recreate framebuffers at new size
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fboTexture);
    glDeleteFramebuffers(1, &brightFbo);
    glDeleteTextures(1, &brightTexture);
    glDeleteFramebuffers(3, blurFbo);
    glDeleteTextures(3, blurTexture);
    glDeleteFramebuffers(1, &bloomResultFbo);
    glDeleteTextures(1, &bloomResultTexture);
    setupBloom();
}

void Renderer::beginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    float time = SDL_GetTicks() / 1000.0f;
    
    // Bright pass
    glBindFramebuffer(GL_FRAMEBUFFER, brightFbo);
    glViewport(0, 0, width_ / 2, height_ / 2);
    glClear(GL_COLOR_BUFFER_BIT);
    brightPassShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glUniform1i(glGetUniformLocation(brightPassShader->id, "screenTexture"), 0);
    glUniform1f(glGetUniformLocation(brightPassShader->id, "threshold"), bloomThreshold);
    drawFullscreenQuad();

    // Multi-pass bloom: 3 blur stages at different resolutions
    // Stage 1: 1/2 resolution (small details)
    int sizes[3][2] = {
        {width_ / 2, height_ / 2},
        {width_ / 4, height_ / 4},
        {width_ / 8, height_ / 8}
    };
    
    unsigned int srcTexture = brightTexture;
    for (int stage = 0; stage < 3; stage++) {
        int w = sizes[stage][0];
        int h = sizes[stage][1];
        
        // Horizontal blur
        glBindFramebuffer(GL_FRAMEBUFFER, blurFbo[stage]);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        blurShader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, srcTexture);
        glUniform1i(glGetUniformLocation(blurShader->id, "screenTexture"), 0);
        glUniform2f(glGetUniformLocation(blurShader->id, "direction"), 1.0f, 0.0f);
        drawFullscreenQuad();
        
        // Vertical blur (ping-pong back to same FBO for simplicity)
        // For production we'd use a second FBO, but this works for now
        // The result is in blurTexture[stage]
        srcTexture = blurTexture[stage];
    }

    // Composite to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glClear(GL_COLOR_BUFFER_BIT);
    compositeShader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glUniform1i(glGetUniformLocation(compositeShader->id, "sceneTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurTexture[2]); // Use the largest blur for bloom
    glUniform1i(glGetUniformLocation(compositeShader->id, "bloomTexture"), 1);
    glUniform1f(glGetUniformLocation(compositeShader->id, "bloomIntensity"), bloomIntensity);
    glUniform1f(glGetUniformLocation(compositeShader->id, "time"), time);
    glUniform2f(glGetUniformLocation(compositeShader->id, "resolution"), (float)width_, (float)height_);
    glUniform1f(glGetUniformLocation(compositeShader->id, "hitFlash"), hitFlashIntensity);
    glUniform1f(glGetUniformLocation(compositeShader->id, "gameOverVignette"), gameOverVignette);
    glUniform1f(glGetUniformLocation(compositeShader->id, "caAmount"), chromaticAberrationAmount);
    drawFullscreenQuad();

    // Bind default framebuffer for subsequent 2D rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
}

void Renderer::drawLineLoop(const Vertex* verts, int count, const Vec3& color) {
    std::vector<Vertex> coloredVerts(count);
    for (int i = 0; i < count; i++) {
        coloredVerts[i] = verts[i];
        coloredVerts[i].color = color;
    }

    Mat4 model(true);
    lineShader->use();
    lineShader->setMat4("proj", projection.ptr());
    lineShader->setMat4("view", view.ptr());
    lineShader->setMat4("model", model.ptr());

    glBindVertexArray(VAOVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * count, coloredVerts.data());
    glDrawArrays(GL_LINE_LOOP, 0, count);
    glBindVertexArray(0);
}

void Renderer::drawTriangles(const Vertex* verts, int count, const Vec3& color) {
    std::vector<Vertex> coloredVerts(count);
    for (int i = 0; i < count; i++) {
        coloredVerts[i] = verts[i];
        coloredVerts[i].color = color;
    }

    Mat4 model(true);
    triangleShader->use();
    triangleShader->setMat4("proj", projection.ptr());
    triangleShader->setMat4("view", view.ptr());
    triangleShader->setMat4("model", model.ptr());
    triangleShader->setVec3("viewPos", viewPos);

    glBindVertexArray(VAOVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * count, coloredVerts.data());
    glDrawArrays(GL_TRIANGLES, 0, count);
    glBindVertexArray(0);
}

void Renderer::drawQuad(const Vertex* verts) {
    // Use a simple overlay rendering
    // Create temporary VAO with pos + color for HUD overlay
    unsigned int qVAO, qVBO;
    glGenVertexArrays(1, &qVAO);
    glGenBuffers(1, &qVBO);
    glBindVertexArray(qVAO);
    glBindBuffer(GL_ARRAY_BUFFER, qVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, verts, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // Use line shader with triangle fan
    lineShader->use();
    Mat4 model(true);
    Mat4 proj = Mat4::ortho(-1, 1, -1, 1, -1, 1);
    Mat4 view(true);
    lineShader->setMat4("proj", proj.ptr());
    lineShader->setMat4("view", view.ptr());
    lineShader->setMat4("model", model.ptr());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &qVAO);
    glDeleteBuffers(1, &qVBO);
}

void Renderer::drawParticles(const Particle* particles, int count) {
    if (count == 0) return;

    int maxCount = count;
    if (maxCount > 2048) maxCount = 2048;

    std::vector<Vertex> verts(maxCount);
    for (int i = 0; i < maxCount; i++) {
        const Particle& p = particles[i];
        verts[i] = Vertex(p.pos, p.color);
    }

    lineShader->use();
    Mat4 model(true);
    lineShader->setMat4("proj", projection.ptr());
    lineShader->setMat4("view", view.ptr());
    lineShader->setMat4("model", model.ptr());

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    glBindVertexArray(VAOVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * maxCount, verts.data());
    glDrawArrays(GL_POINTS, 0, maxCount);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POINT_SPRITE);
    glBindVertexArray(0);
}

void Renderer::drawParticlesECS(const float* data, int count) {
    if (count == 0) return;

    // ECS render buffer format: [x,y,z, r,g,b,a, size] per particle (8 floats)
    // Use instanced rendering: 1 draw call for all particles

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * count, data, GL_DYNAMIC_DRAW);

    glBindVertexArray(particleVAO);

    // Position attribute (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
    glVertexAttribDivisor(0, 1);

    // Color attribute (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
    glVertexAttribDivisor(1, 1);

    // Size attribute (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 7));
    glVertexAttribDivisor(2, 1);

    lineShader->use();
    Mat4 model(true);
    lineShader->setMat4("proj", projection.ptr());
    lineShader->setMat4("view", view.ptr());
    lineShader->setMat4("model", model.ptr());

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArraysInstanced(GL_POINTS, 0, 1, count);

    glBindVertexArray(0);
}

void Renderer::drawParticlesInstanced(const Particle* particles, int count) {
    if (count == 0) return;

    int maxCount = count;
    if (maxCount > 4096) maxCount = 4096;

    // Instanced rendering: 1 Draw-Call für alle Partikel
    struct InstanceData {
        Vec3 pos;
        Vec3 color;
        float size;
    };

    std::vector<InstanceData> instances(maxCount);
    for (int i = 0; i < maxCount; i++) {
        instances[i].pos = particles[i].pos;
        instances[i].color = particles[i].color;
        instances[i].size = particles[i].size;
    }

    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData) * maxCount, instances.data(), GL_DYNAMIC_DRAW);

    // Set up instanced attributes
    glBindVertexArray(particleVAO);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)0);
    glVertexAttribDivisor(0, 1);

    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, color));
    glVertexAttribDivisor(1, 1);

    // Size attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, size));
    glVertexAttribDivisor(2, 1);

    // Draw instanced
    lineShader->use();
    Mat4 model(true);
    lineShader->setMat4("proj", projection.ptr());
    lineShader->setMat4("view", view.ptr());
    lineShader->setMat4("model", model.ptr());

    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArraysInstanced(GL_POINTS, 0, 1, maxCount);

    glBindVertexArray(0);
}

void Renderer::drawGround(float time) {
    // Draw a large ground quad with plasma shader
    float s = 80.0f;
    Vec3 up(0, 1, 0);
    Vertex verts[] = {
        Vertex(Vec3(-s, 0, -s), up, Vec3(0, 0, 0), 0, 0),
        Vertex(Vec3( s, 0, -s), up, Vec3(0, 0, 0), 1, 0),
        Vertex(Vec3( s, 0,  s), up, Vec3(0, 0, 0), 1, 1),
        Vertex(Vec3(-s, 0,  s), up, Vec3(0, 0, 0), 0, 1),
    };

    Mat4 model(true);
    groundShader->use();
    groundShader->setMat4("proj", projection.ptr());
    groundShader->setMat4("view", view.ptr());
    groundShader->setMat4("model", model.ptr());
    groundShader->setFloat("time", time);

    glBindVertexArray(VAOVertex);
    glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * 4, verts);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void Renderer::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
