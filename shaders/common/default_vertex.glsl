// #version 450

// shape is constructed in fragment, vertex only creates bounding box
// this is to decrease state chagnes and improve quality by smoothing

layout(location = 0) in uvec4 coloring_info;
layout(location = 1) in uint shape_type;
layout(location = 2) in vec2 global_pos;
// layout(location = 3) in vec2 rotation;
layout(location = 3) in float rotation_angle;
layout(location = 4) in float radius;
layout(location = 5) in float value_1; // used to make shape size
layout(location = 6) in float value_2; // used to make shape size
layout(location = 7) in float value_3; // used to make shape size

#include "ubo.glsl"

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;
const int Trapezoid = Capsule+1;

//padding for smooth edges
const float SIZE_MULTIPLIER = 1.02;
const float SIZE_ADDER = 0.02;

vec2 get_shape_shift() {
    vec2 shift;
    
    // float param_1 = abs(value_1);
    // float param_2 = abs(value_2);
    // float param_3 = abs(value_3);

    switch (shape_type){
        case (Circle): {
            // value_1 is radius
            shift = vec2(value_1);
            // shift *= SIZE_MULTIPLIER; //padding for smooth edges
            shift += sign(value_1)*SIZE_ADDER; //padding for smooth edges
            break;
        }
        case (Square): {
            // value_1 is width/2,  value_2 is height/2
            shift = vec2(value_1, value_2); // Bounding box extends half the width/height from center
            // shift *= SIZE_MULTIPLIER; //padding for smooth edges
            shift += vec2(sign(value_1),sign(value_2))*SIZE_ADDER; //padding for smooth edges
            break;
        }
        case (Capsule): {
            // value_1 is radius, value_2 is length/2
            shift = vec2(value_2 + value_1, value_1); // Horizontal length + radius at both ends
            // shift *= SIZE_MULTIPLIER;
            shift += vec2(sign(value_2),sign(value_1))*SIZE_ADDER; //padding for smooth edges
            break;
        }
        case (Trapezoid): {
            // an 'isosceles' one, which means vertically symmetrical 
            // Trapezoid is made almost like a square, but 
            int effective_index = (gl_VertexIndex >= 3)? (gl_VertexIndex-2) : (gl_VertexIndex);
            if(effective_index < 2){
                //bottom
                shift.x = value_1;
            } else {
                //top
                shift.x = value_2;
            }
            shift.y = +value_3;
            //TODO:
            // shift *= SIZE_MULTIPLIER;
            shift += vec2(sign(value_2),sign(value_3))*SIZE_ADDER; //padding for smooth edges
            break;
        }
    }
    return shift;
}

//TODO: opt like grass
vec2 get_vertex_pos(){
    vec2 shift = get_shape_shift();
    vec2 position;
    vec2 corner; // specifies shift direction
    int effective_index = (gl_VertexIndex >= 3)? (gl_VertexIndex-2) : (gl_VertexIndex);
    corner.x = float((effective_index%2) == 0); 
    corner.y = float((effective_index/2)); 
    //0/1 -> -1/+1
    corner = corner*2.0 - 1.0;

    position = corner * shift;
    return position;
}