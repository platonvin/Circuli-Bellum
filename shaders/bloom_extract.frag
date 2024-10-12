#version 450

precision highp float;
precision highp int;

layout(location = 0) out vec4 outColor;

#include "common/ubo.glsl"

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
