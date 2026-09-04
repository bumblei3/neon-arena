// text.h - Bitmap font text renderer with texture atlas
#pragma once
#include "math.h"
#include "shader.h"
#include <string>

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();

    bool init();
    void shutdown();

    // Draw text at screen position (normalized -1 to 1)
    void drawText(const std::string& text, float x, float y, float scale, Vec3 color);

    // Draw centered text
    void drawTextCentered(const std::string& text, float y, float scale, Vec3 color);

    // Measure text width (for centering)
    float getTextWidth(const std::string& text, float scale);

private:
    // 5x7 bitmap font (ASCII 32-90)
    static const unsigned char font[59][7];

    // Texture atlas
    unsigned int atlasTexture_ = 0;
    int atlasWidth_ = 0;
    int atlasHeight_ = 0;

    // Per-glyph UV coords
    float glyphU_[91];
    float glyphV_[91];
    float glyphW_[91];
    float glyphH_[91];

    // Persistent VAO/VBO for text quads
    unsigned int textVAO_ = 0;
    unsigned int textVBO_ = 0;

    // Text shader
    Shader* textShader_ = nullptr;

    void buildAtlas();
};
