// text.h - Bitmap font text renderer
#pragma once
#include "renderer.h"
#include "math.h"
#include <string>

class TextRenderer {
public:
    TextRenderer() = default;

    // Draw text at screen position (normalized -1 to 1)
    void drawText(Renderer* renderer, const std::string& text, float x, float y, float scale, Vec3 color);

    // Draw centered text
    void drawTextCentered(Renderer* renderer, const std::string& text, float y, float scale, Vec3 color);

    // Measure text width (for centering)
    float getTextWidth(const std::string& text, float scale);

private:
    // 5x7 bitmap font (ASCII 32-90)
    static const unsigned char font[59][7];
};
