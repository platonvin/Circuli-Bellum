#version 450

precision highp float;
precision highp int;

layout(location = 0) out vec4 outColor;

#include "common/ubo.glsl"

layout(set = 0, binding = 1) uniform sampler2D rendered_frame;
layout(set = 0, binding = 2) uniform sampler2D bloom_result;

#include "common/map_shadow.glsl"

void main(){
    // vec4 rendered_color = subpassLoad(rendered_frame);
    vec2 size = ubo.resolution;
    vec2 uv = gl_FragCoord.xy/size;
    uv *= 1.0;
/* rgb shifts match regular tirangle vertices
   R
   o
G     B
*/  
    vec2 chrom_abb = ubo.chrom_abb;
    vec2 uv_r_shift = vec2(0,1)*chrom_abb;
    vec2 uv_g_shift = vec2(-cos(radians(30)),sin(radians(30)))*chrom_abb;
    vec2 uv_b_shift = vec2(+cos(radians(30)),sin(radians(30)))*chrom_abb;

    vec2 clip = uv*2.0 - 1.0;
    float dist_to_center = length(clip);

    uv_r_shift *= dist_to_center;
    uv_g_shift *= dist_to_center;
    uv_b_shift *= dist_to_center;

    vec3 rendered_color;
        rendered_color.r = textureLod(rendered_frame, uv+uv_r_shift, 0).r;
        rendered_color.g = textureLod(rendered_frame, uv+uv_g_shift, 0).g;
        rendered_color.b = textureLod(rendered_frame, uv+uv_b_shift, 0).b;
    vec3 bloom_color;
        bloom_color.r = textureLod(bloom_result, uv+uv_r_shift, 0).r;
        bloom_color.g = textureLod(bloom_result, uv+uv_g_shift, 0).g;
        bloom_color.b = textureLod(bloom_result, uv+uv_b_shift, 0).b;

    // vec2 world_pos = clip*ubo.camera_scale + ubo.camera_pos;
    // vec2 clip_depth = world2shadowClip(world_pos);
    // float this_depth = clip_depth.y;
    // // not U-V, but just easier to read this way
    // float shadow_uv = (clip_depth.x + 1.0) / 2.0;
    // float test_depth = textureLod(shadowmap, shadow_uv, 0).x;
    // float light = (test_depth < this_depth)? 0.5 : 1.0; 
        
    outColor = vec4((rendered_color + bloom_color*4.0) * 1.0, 1);
    // outColor = rendered_color;
    // outColor = bloom_color;
}
