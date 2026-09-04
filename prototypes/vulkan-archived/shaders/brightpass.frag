#version 450

layout(binding = 0) uniform sampler2D sceneImage;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

const float threshold = 0.6;
const float softness = 0.2;

void main() {
    vec4 color = texture(sceneImage, fragUV);
    // Bright-pass: extract areas above threshold with soft knee
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    float knee = threshold * softness;
    float soft = brightness - threshold + knee;
    soft = clamp(soft / (2.0 * knee + 0.0001), 0.0, 1.0);
    float contribution = max(soft * soft * 4.0, brightness - threshold);
    outColor = vec4(color.rgb * contribution / max(brightness, 0.0001), 1.0);
}
