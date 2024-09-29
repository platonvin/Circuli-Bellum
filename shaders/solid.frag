#version 450

precision highp float;
precision highp int;

layout(location = 0) flat in uvec3 coloring_info;
layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) in float value_1; // used to make shape size
layout(location = 4) in float value_2; // used to make shape size

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
} ubo;

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;

// distance field smooth_step with smooth edges instead of discard 
// literally math-driven MSAA
float get_alpha_from_shape(){
    switch (shape_type){
        case Circle:{
            float radius = value_1;
            float dist = distance(local_pos, vec2(0));
            return 1.0 - smoothstep(radius*0.98,radius*1.02,dist);
        }
        case Square:{
            return 1.0;
        }
    }
}

void main() {
    vec4 color = vec4(vec3(coloring_info)/255.0, get_alpha_from_shape());
    
    outColor = color;
} 