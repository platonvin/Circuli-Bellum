layout(location = 0) flat in uvec3 coloring_info;
layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) flat in float extra_sdf_radius;
layout(location = 4) flat in float value_1;
layout(location = 5) flat in float value_2;
layout(location = 6) flat in float value_3;

layout(location = 0) out vec4 outColor;

#include "ubo.glsl"

#include "sdf_functions.glsl"