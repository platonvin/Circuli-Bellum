#version 450

#include "common/default_vertex.glsl"

layout(location = 0) flat out uvec4 out_coloring_info;
layout(location = 1) flat out uint out_shape_type;
layout(location = 2) out vec2 out_local_pos;
layout(location = 3) flat out float out_radius;
layout(location = 4) flat out float out_value_1;
layout(location = 5) flat out float out_value_2;
layout(location = 6) flat out float out_value_3;

void main() {
    vec2 local_pos = get_vertex_pos();
    vec2 rotation = vec2(cos(rotation_angle),sin(rotation_angle));
    mat2 mrot = mat2(rotation.x, rotation.y, -rotation.y, rotation.x);
    vec2 world_pos = global_pos + local_pos * mrot;
    
    out_coloring_info = coloring_info;
    out_shape_type = shape_type;
    out_local_pos = local_pos;
    out_value_1 = value_1;
    out_value_2 = value_2;
    out_value_3 = value_3;
    out_radius = radius;

    vec2 relative_pos = world_pos - ubo.camera_pos;
    vec2 clip = relative_pos / ubo.camera_scale;
    gl_Position = vec4(clip, 0, 1);
}
