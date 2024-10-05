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
layout(set = 0, binding = 2 ) uniform sampler2D bloom_result;

void main(){
    vec4 rendered_color = subpassLoad(rendered_frame);
    vec2 size = ubo.resolution;
    vec4 bloom_color = textureLod(bloom_result, (gl_FragCoord.xy/size), 0);
    outColor = (rendered_color + bloom_color*4.0) / 1.0;
    // outColor = rendered_color;
    // outColor = bloom_color;
}
