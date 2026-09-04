// text.cpp - Bitmap font text renderer with texture atlas
#include "text.h"
#include <cmath>
#include <cstring>

// 5x7 bitmap font (ASCII 32-90: Space to Z)
const unsigned char TextRenderer::font[59][7] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, // !
    {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}, // #
    {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04}, // $
    {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}, // %
    {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D}, // &
    {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, // (
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}, // )
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00}, // *
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08}, // ,
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}, // .
    {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}, // /
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, // 2
    {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E}, // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
    {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00}, // :
    {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x08}, // ;
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}, // <
    {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00}, // =
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, // >
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}, // ?
    {0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E}, // @
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
    {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}, // D
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // F
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}, // G
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}, // N
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
    {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E}, // S
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
    {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04}, // V
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}, // W
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
};

// Embedded text shader
static const char* textVertSrc = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aUV;
    layout(location = 2) in vec3 aColor;
    uniform mat4 proj;
    out vec2 vUV;
    out vec3 vColor;
    void main() {
        gl_Position = proj * vec4(aPos, 0.0, 1.0);
        vUV = aUV;
        vColor = aColor;
    }
)";

static const char* textFragSrc = R"(
    #version 330 core
    in vec2 vUV;
    in vec3 vColor;
    out vec4 FragColor;
    uniform sampler2D uTexture;
    void main() {
        float alpha = texture(uTexture, vUV).r;
        FragColor = vec4(vColor, alpha);
    }
)";

TextRenderer::TextRenderer() {
    memset(glyphU_, 0, sizeof(glyphU_));
    memset(glyphV_, 0, sizeof(glyphV_));
    memset(glyphW_, 0, sizeof(glyphW_));
    memset(glyphH_, 0, sizeof(glyphH_));
}

TextRenderer::~TextRenderer() {
    shutdown();
}

void TextRenderer::shutdown() {
    if (atlasTexture_) {
        glDeleteTextures(1, &atlasTexture_);
        atlasTexture_ = 0;
    }
    if (textVAO_) {
        glDeleteVertexArrays(1, &textVAO_);
        glDeleteBuffers(1, &textVBO_);
        textVAO_ = 0;
        textVBO_ = 0;
    }
    delete textShader_;
    textShader_ = nullptr;
}

bool TextRenderer::init() {
    buildAtlas();

    // Create text shader
    textShader_ = new Shader();
    if (!textShader_->load(textVertSrc, textFragSrc)) {
        fprintf(stderr, "Failed to load text shader\n");
        return false;
    }

    // Create persistent VAO/VBO for text quads
    glGenVertexArrays(1, &textVAO_);
    glGenBuffers(1, &textVBO_);
    glBindVertexArray(textVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO_);
    // Max 256 chars * 4 verts * 8 floats (pos.xy, uv.xy, color.rgb, padding)
    glBufferData(GL_ARRAY_BUFFER, 256 * 4 * 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

void TextRenderer::buildAtlas() {
    // Pack all 59 glyphs into a single texture
    int cols = 8;
    int glyphW = 6;  // 5 + 1 padding
    int glyphH = 8;  // 7 + 1 padding
    atlasWidth_ = cols * glyphW;
    int rows = (59 + cols - 1) / cols;
    atlasHeight_ = rows * glyphH;

    // Create pixel data (R8 format - single channel)
    unsigned char* data = new unsigned char[atlasWidth_ * atlasHeight_];
    memset(data, 0, atlasWidth_ * atlasHeight_);

    for (int c = 0; c < 59; c++) {
        int col = c % cols;
        int row = c / cols;
        int baseX = col * glyphW;
        int baseY = row * glyphH;

        // Copy glyph pixels
        for (int gy = 0; gy < 7; gy++) {
            for (int gx = 0; gx < 5; gx++) {
                if (font[c][gy] & (1 << (4 - gx))) {
                    data[(baseY + gy) * atlasWidth_ + (baseX + gx)] = 255;
                }
            }
        }

        // Calculate UV coords
        float u0 = (float)baseX / atlasWidth_;
        float v0 = (float)baseY / atlasHeight_;
        float u1 = (float)(baseX + 5) / atlasWidth_;
        float v1 = (float)(baseY + 7) / atlasHeight_;
        glyphU_[c + 32] = u0;
        glyphV_[c + 32] = v0;
        glyphW_[c + 32] = u1 - u0;
        glyphH_[c + 32] = v1 - v0;
    }

    // Upload to GPU
    glGenTextures(1, &atlasTexture_);
    glBindTexture(GL_TEXTURE_2D, atlasTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlasWidth_, atlasHeight_, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    delete[] data;
}

void TextRenderer::drawText(const std::string& text, float x, float y, float scale, Vec3 color) {
    if (text.empty() || !atlasTexture_) return;

    float charW = scale * 0.04f;
    float charH = scale * 0.06f;
    float spacing = charW * 1.5f;

    int len = (int)text.size();
    if (len > 256) len = 256;

    // Build vertex data: 4 verts per char, 8 floats per vert
    float verts[256 * 4 * 8];
    int vertCount = 0;

    for (int i = 0; i < len; i++) {
        char c = text[i];
        if (c < 32 || c > 90) continue;

        float cx = x + i * spacing;
        float u = glyphU_[(int)c];
        float v = glyphV_[(int)c];
        float uw = glyphW_[(int)c];
        float vh = glyphH_[(int)c];

        // 4 vertices: TL, TR, BR, BL
        float vx[4] = {cx, cx + charW, cx + charW, cx};
        float vy[4] = {y, y, y + charH, y + charH};
        float uu[4] = {u, u + uw, u + uw, u};
        float vv[4] = {v, v, v + vh, v + vh};

        for (int j = 0; j < 4; j++) {
            float* vptr = &verts[vertCount * 8];
            vptr[0] = vx[j];
            vptr[1] = vy[j];
            vptr[2] = uu[j];
            vptr[3] = vv[j];
            vptr[4] = color.x;
            vptr[5] = color.y;
            vptr[6] = color.z;
            vptr[7] = 1.0f;
            vertCount++;
        }
    }

    if (vertCount == 0) return;

    // Upload and draw
    glBindVertexArray(textVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertCount * 8 * sizeof(float), verts);

    // Bind atlas texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTexture_);

    // Set up ortho projection
    Mat4 proj = Mat4::ortho(-1, 1, -1, 1, -1, 1);

    textShader_->use();
    textShader_->setMat4("proj", proj.ptr());
    textShader_->setInt("uTexture", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_QUADS, 0, vertCount);
    glBindVertexArray(0);
}

void TextRenderer::drawTextCentered(const std::string& text, float y, float scale, Vec3 color) {
    float width = getTextWidth(text, scale);
    drawText(text, -width / 2.0f, y, scale, color);
}

float TextRenderer::getTextWidth(const std::string& text, float scale) {
    return text.size() * scale * 0.04f * 1.5f;
}
