#version 450
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;
layout(set = 0, binding = 0) uniform texture2D tTexture;
layout(set = 0, binding = 1) uniform sampler sSampler;
layout(location = 0) out vec4 fColor;
void main() {
    fColor = vColor * texture(sampler2D(tTexture, sSampler), vUV);
}
