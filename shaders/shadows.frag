#version 450

/*
this is not sdf
*/

layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 world_pos;
layout(location = 3) flat in float extra_sdf_radius;
layout(location = 4) flat in float value_1;
layout(location = 5) flat in float value_2;
layout(location = 6) flat in float value_3;
layout(location = 7) in vec2 local_pos;

// layout(location = 0) out vec4 outColor;
#include "common/ubo.glsl"
#include "common/map_shadow.glsl"

void main(){
    // float sdf = getAlphaFromShape();
    vec2 clip_depth = world2shadowClip(world_pos);
    gl_FragDepth = clip_depth.y;
    // if(sdf > 0) {
    // } else {
    //     gl_FragDepth = 1;
    // }
    
}