precision highp float;
precision highp int;

layout(location = 0) flat in uvec3 coloring_info;
layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) flat in float value_1; // used to make shape size
layout(location = 4) flat in float value_2; // used to make shape size
layout(location = 5) flat in float value_3; // used to make shape size

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
    ivec2 resolution;
    float time;
} ubo;

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;
const int Trapezoid = Capsule+1;

// distance field smooth_step with smooth edges instead of discard 
// literally math-driven MSAA
float getAlphaFromShape(){
    switch (shape_type){
        case Circle:{
            // for cirlce, local pos is measured from center
            float radius = value_1;
            float dist = distance(local_pos, vec2(0));
            return 1.0 - smoothstep(radius*0.98,radius*1.02,dist);
        }
        case Square:{
            // for square, local pos is measured from center
            return 1.0;
        }
        case Trapezoid:{
            // for trapezoid, local pos is measured from center (it is not for triangle only i guess) 
            // it is also a 'isosceles' one, which means vertically symmetrical 
            // Trapezoid is made almost like a square
            return 1.0;
        }
    }
}