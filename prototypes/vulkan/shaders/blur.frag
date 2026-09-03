#version 450

layout(binding = 0) uniform sampler2D inputImage;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform BlurParams {
    vec2 direction;
    float radius;
} params;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(inputImage, 0));
    float sigma = max(params.radius * 0.5, 0.001);
    float twoSigmaSq = 2.0 * sigma * sigma;
    
    vec4 result = vec4(0.0);
    float weightSum = 0.0;
    
    // 9-tap Gaussian blur
    for (int i = -4; i <= 4; i++) {
        float weight = exp(-float(i * i) / twoSigmaSq);
        vec2 offset = params.direction * texelSize * float(i) * params.radius;
        result += texture(inputImage, fragUV + offset) * weight;
        weightSum += weight;
    }
    
    outColor = result / weightSum;
}
