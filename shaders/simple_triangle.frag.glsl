#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 v_fragColor;
layout(location = 1) in vec2 v_UV;

void main() {
    outColor = vec4(v_fragColor, 1.0);

    float d = 1 - length(v_UV);
    d = step(0.0, d);

    outColor *= d;

}