#version 450

//darkenes pixels in shadow

precision highp float;
precision highp int;

layout(location = 0) out vec4 outColor;

#include "common/ubo.glsl"
layout(set = 0, binding = 1) uniform sampler1D shadowmap;

#include "common/map_shadow.glsl"

void main(){
    vec2 size = ubo.resolution;
    vec2 uv = gl_FragCoord.xy/size;
    vec2 clip = uv*2.0 - 1.0;

    vec2 world_pos = clip*ubo.camera_scale + ubo.camera_pos;
    vec2 clip_depth = world2shadowClip(world_pos);
    float this_depth = clip_depth.y;
    // not U-V, but just way easier to read this way
    float shadow_uv = (clip_depth.x + 1.0) / 2.0;
    float test_depth = textureLod(shadowmap, shadow_uv, 0).x;
    float light = (test_depth < this_depth)? 0.8 : 1.0; 
        
    //mix with dark color for shading
    outColor = vec4(vec3(0.0), 1.0-light);
    // outColor = vec4(vec3(0.5), 0.0);
}
