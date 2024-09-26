#version 450 

// shape is constructed in fragment, vertex only creates bounding box
// this is to decrease state chagnes and improve quality by smoothing

layout(location = 0) in uvec3 coloring_info;
layout(location = 1) in uint shape_type;
layout(location = 2) in vec2 local_pos;
layout(location = 3) in float value_1; // used to make shape size
layout(location = 4) in float value_2; // used to make shape size

layout(location = 0) flat out uvec3 out_coloring_info;
layout(location = 1) flat out uint out_shape_type;
layout(location = 2) out vec2 out_local_pos;
layout(location = 3) out float out_value_1; // used to make shape size
layout(location = 4) out float out_value_2; // used to make shape size

layout(binding = 0, set = 0) uniform restrict readonly UniformBufferObject {
    vec2 camera_pos;
    vec2 camera_scale;
} ubo;

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;

// Shape is either circle, capsule or square (for now)
vec2 get_shape_shift(){
    vec2 shift;
    
    if (shape_type == Circle) {
        // value_1 is radius
        shift = vec2(value_1);
    }
    else if (shape_type == Square) {
        // value_1 is width/2,  value_2 is height/2
        shift = vec2(value_1, value_2); // Bounding box extends half the width/height from center
    }
    else if (shape_type == Capsule) {
        // value_1 is radius, value_2 is length/2
        shift = vec2(value_2 + value_1, value_1); // Horizontal length + radius at both ends
    }

    return shift;
}

//TODO: opt like grass
vec2 get_vertex_pos(){
    vec2 shift = get_shape_shift();
    vec2 position;
    if ((gl_VertexIndex == 0) || (gl_VertexIndex == 3)) {
        position = vec2(-shift.x, -shift.y); // Bottom-left
    }
    else if ((gl_VertexIndex == 1)) {
        position = vec2( shift.x, -shift.y); // Bottom-right
    }
    else if ((gl_VertexIndex == 2) || (gl_VertexIndex == 4)) {
        position = vec2( shift.x,  shift.y); // Top-right
    }
    else if ((gl_VertexIndex == 5)) {
        position = vec2(-shift.x,  shift.y); // Top-left
    }
    return position;
}

void main() 
{
    vec2 world_pos = local_pos + get_vertex_pos();
    
    out_coloring_info = coloring_info;
    out_shape_type = shape_type;
    out_local_pos = local_pos;
    out_value_1 = value_1;
    out_value_2 = value_2;

    vec2 relative_pos = world_pos - ubo.camera_pos;
    // vec2 relative_pos = world_pos;
    vec2 clip = relative_pos / ubo.camera_scale;
    // vec2 clip = relative_pos;
    gl_Position = vec4(clip, 0, 1);

    // vec2 outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    // gl_Position = vec4(outUV * 2.0f + -1.0f, 0.0f, 1.0f);
}