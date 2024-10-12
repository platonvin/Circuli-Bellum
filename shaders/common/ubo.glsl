layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
    ivec2 resolution;
    vec2 chrom_abb;
    vec2 light_pos;
    float time;
} ubo;