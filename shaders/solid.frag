#version 450

#include "common/header.glsl"

void main() {
    vec4 color = vec4(coloring_info)/255.0;
    color.a *= getAlphaFromShape();
    outColor = color;
} 