#version 450

precision highp float;
precision highp int;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
    ivec2 resolution;
    float time;
} ubo;
layout(input_attachment_index = 0, set = 0, binding = 1) uniform subpassInput rendered_frame;

void main(){
    vec4 rendered_color = subpassLoad(rendered_frame);
    if(any(greaterThan(rendered_color.xyz, vec3(.6)))){
        outColor = vec4(rendered_color.rgb-.6,1);
    }
    else {
        outColor = vec4(0);
    }
}
