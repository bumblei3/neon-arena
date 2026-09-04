#version 450

layout(binding = 0) uniform sampler2D sceneImage;
layout(binding = 1) uniform sampler2D bloomImage;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 scene = texture(sceneImage, fragUV);
    vec4 bloom = texture(bloomImage, fragUV);
    // Additive bloom blend
    outColor = scene + bloom;
}
