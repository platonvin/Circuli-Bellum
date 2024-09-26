#version 450

precision highp float;
precision highp int;

layout(location = 0) flat in uvec3 coloring_info;
layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) in float value_1; // used to make shape size
layout(location = 4) in float value_2; // used to make shape size

layout(location = 0) out vec4 outColor;

// layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
//     vec2 camera_pos;
//     vec2 camera_scale;
// } ubo;

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;

void main() {
    vec4 color = vec4(vec3(255)/255.0, 1);
    
    outColor = color;
} 