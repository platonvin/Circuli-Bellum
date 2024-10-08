layout(location = 0) flat in uvec3 coloring_info;
layout(location = 1) flat in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) flat in float extra_sdf_radius;
layout(location = 4) flat in float value_1;
layout(location = 5) flat in float value_2;
layout(location = 6) flat in float value_3;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
    ivec2 resolution;
    float time;
} ubo;

const int SHAPE_CIRCLE = 0;
const int SHAPE_SQUARE = 1;
const int SHAPE_CAPSULE = 2;
const int SHAPE_TRAPEZOID = 3;

//TODO: dynamic pixel dependend
const float SOFT_EDGE_RADIUS = 0.02;

//sdf means "signed distance field"
//basically, it returnes smallest distance to shape in a given point
//if inside a shape, then it will be zero/negative

float sdf_circle(vec2 position, float radius) {
    return length(position) - radius;
}

float sdf_square(vec2 position, vec2 half_size) {
    vec2 distance_from_edges = abs(position) - half_size;
    float inside_distance = min(max(distance_from_edges.x, distance_from_edges.y), 0.0);
    return inside_distance + length(max(distance_from_edges, 0.0));
}

float sdf_capsule(vec2 position, float radius, float half_length) {
    vec2 distance_from_edges = abs(position) - vec2(half_length, 0.0);
    return length(max(distance_from_edges, 0.0)) - radius;
}

//precise
float sdf_trapezoid_1(vec2 position, float bottom_hw, float bottom_tw, float half_height) {
    position.x = abs(position.x);
    position.x -= 0.5 * (bottom_tw + bottom_hw);
    vec2 e = vec2(0.5 * (bottom_tw - bottom_hw), half_height);
    vec2 q = position - e * clamp(dot(position, e) / dot(e, e), -1.0, 1.0);
    float d = length(q);
    if (q.x > 0.0) {
        return d;
    }
    return max(-d, abs(position.y) - half_height);
}

//approx
float sdf_trapezoid_2(vec2 position, float bottom_hw, float top_hw, float half_height) {
    float width = mix(bottom_hw, top_hw, (position.y + half_height) / (2.0 * half_height));
    vec2 distance_from_edges = abs(position) - vec2(width, half_height);
    float inside_distance = min(max(distance_from_edges.x, distance_from_edges.y), 0.0);
    return inside_distance + length(max(distance_from_edges, 0.0));
}

float getAlphaFromShape() {
    float shape_distance = 0.0;

    float param_1 = abs(value_1);
    float param_2 = abs(value_2);
    float param_3 = abs(value_3);

    float sdf_radius = abs(extra_sdf_radius);
    
    switch (shape_type) {
        case SHAPE_CIRCLE:
            shape_distance = sdf_circle(local_pos, param_1);
            break;
        case SHAPE_SQUARE:
            param_1 -= sdf_radius;
            param_2 -= sdf_radius;
            param_3 -= sdf_radius;
            shape_distance = sdf_square(local_pos, vec2(param_1, param_2));
            shape_distance -= sdf_radius;
            break;
        case SHAPE_CAPSULE:
            shape_distance = sdf_capsule(local_pos, param_1, param_2);
            break;
        case SHAPE_TRAPEZOID:
            // shape_distance = sdf_trapezoid_2(local_pos, param_1, param_2, param_3);
            shape_distance = sdf_trapezoid_1(local_pos, param_1, param_2, param_3);
            break;
    }
    return 1.0 - smoothstep(-SOFT_EDGE_RADIUS, SOFT_EDGE_RADIUS, shape_distance);
}
