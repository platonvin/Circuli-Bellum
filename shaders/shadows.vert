#version 450

/*
so what is actually happening?
we get shapes as vertices
they are in 2d
then we project them into 1d
and draw
this could potentially be moved to cpu, cause whole image fits into L1
gpu-version was just easier to implement and potentially more scalable

literally just rasterizes depth to an image
1d shadow maps work the same way as 2d
*/

// literally same inputs. Some of them can be removed

layout(location = 0) in uvec3 coloring_info;
layout(location = 1) in uint shape_type;
layout(location = 2) in vec2 world_center;
// layout(location = 3) in vec2 rotation;
layout(location = 3) in float rotation_angle;
layout(location = 4) in float extra_sdf_radius;
layout(location = 5) in float value_1; // used to make shape size
layout(location = 6) in float value_2; // used to make shape size
layout(location = 7) in float value_3; // used to make shape size

#include "common/ubo.glsl"

const int Circle = 0;
const int Square = Circle+1;
const int Capsule = Square+1;
const int Trapezoid = Capsule+1;

//padding for smooth edges
const float SIZE_MULTIPLIER = 1.02;
const float SIZE_ADDER = 0.02;

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

const int SHAPE_CIRCLE = 0;
const int SHAPE_SQUARE = 1;
const int SHAPE_CAPSULE = 2;
const int SHAPE_TRAPEZOID = 3;

//TODO: dynamic pixel dependend
const float SOFT_EDGE_RADIUS = 0.02;

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

vec4 findTangentContactPoints(vec2 circle_center, float circle_radius, vec2 some_point) {
    vec2 point2circle = circle_center-some_point;
    vec2 direction = normalize(point2circle);

    float angle = asin(circle_radius / length(point2circle));

    float cos_angle = cos(angle);
    float sin_angle = sin(angle);

    float len = sqrt(dot(point2circle,point2circle) - circle_radius*circle_radius);

    vec2 point2contact_cw = vec2(
        direction.x * cos_angle - direction.y * sin_angle,
        direction.x * sin_angle + direction.y * cos_angle
    ) * len;

    vec2 point2contact_ccw = vec2(
        direction.x * cos_angle + direction.y * sin_angle,
        -direction.x * sin_angle + direction.y * cos_angle
    ) * len;

    return vec4(point2contact_cw, point2contact_ccw);
}

#define _CW zw
#define _CCW xy
vec4 findExtremeTangentAngles(vec4 res1, vec4 res2, vec4 res3, vec4 res4) {
    vec4 res;
    {
        // Extract tangent points from results
        vec2 cw1 = res1._CW;
        vec2 cw2 = res2._CW;
        vec2 cw3 = res3._CW;
        vec2 cw4 = res4._CW;

        // Compute angles for each clockwise tangent point
        float angle1 = atan(cw1.y, cw1.x);
        float angle2 = atan(cw2.y, cw2.x);
        float angle3 = atan(cw3.y, cw3.x);
        float angle4 = atan(cw4.y, cw4.x);

        // Initialize max and min angle variables
        float min_angle = angle1;

        vec2 most_cw_contact = cw1;

        // Compare angles to find the max and min
        if (angle2 < min_angle) {
            min_angle = angle2;
            most_cw_contact = cw2;
        }
        if (angle3 < min_angle) {
            min_angle = angle3;
            most_cw_contact = cw3;
        }
        if (angle4 < min_angle) {
            min_angle = angle4;
            most_cw_contact = cw4;
        }
        res._CW = most_cw_contact;
    }
    {
        // Extract tangent points from results
        vec2 ccw1 = res1._CCW;
        vec2 ccw2 = res2._CCW;
        vec2 ccw3 = res3._CCW;
        vec2 ccw4 = res4._CCW;

        // Compute angles for each clockwise tangent point
        float angle1 = atan(ccw1.y, ccw1.x);
        float angle2 = atan(ccw2.y, ccw2.x);
        float angle3 = atan(ccw3.y, ccw3.x);
        float angle4 = atan(ccw4.y, ccw4.x);

        // Initialize max and min angle variables
        float max_angle = angle1;

        vec2 most_ccw_contact = ccw4;

        // Compare angles to find the max and min
        if (angle2 > max_angle) {
            max_angle = angle2;
            most_ccw_contact = ccw2;
        }
        if (angle3 > max_angle) {
            max_angle = angle3;
            most_ccw_contact = ccw3;
        }
        if (angle4 > max_angle) {
            max_angle = angle4;
            most_ccw_contact = ccw4;
        }
        res._CCW = most_ccw_contact;
    }

    return res;
}

vec4 findExtremeTangentAngles(vec4 res1, vec4 res2) {
    vec4 res;

    // Extract tangent points for clockwise comparisons
    vec2 cw1 = res1.zw; // Clockwise point from res1
    vec2 cw2 = res2.zw; // Clockwise point from res2

    // Compute angles for each clockwise tangent point
    float angle1 = atan(cw1.y, cw1.x);
    float angle2 = atan(cw2.y, cw2.x);

    // Initialize min angle variable and most clockwise contact point
    float min_angle = angle1;
    vec2 most_cw_contact = cw1;

    // Compare angles to find the most clockwise
    if (angle2 < min_angle) {
        min_angle = angle2;
        most_cw_contact = cw2;
    }

    res.zw = most_cw_contact; // Store the most clockwise contact point

    // Extract tangent points for counterclockwise comparisons
    vec2 ccw1 = res1.xy; // Counterclockwise point from res1
    vec2 ccw2 = res2.xy; // Counterclockwise point from res2

    // Compute angles for each counterclockwise tangent point
    float angle3 = atan(ccw1.y, ccw1.x);
    float angle4 = atan(ccw2.y, ccw2.x);

    // Initialize max angle variable and most counterclockwise contact point
    float max_angle = angle3;
    vec2 most_ccw_contact = ccw1;

    // Compare angles to find the most counterclockwise
    if (angle4 > max_angle) {
        max_angle = angle4;
        most_ccw_contact = ccw2;
    }

    res.xy = most_ccw_contact; // Store the most counterclockwise contact point

    return res; // Return both contact points
}

// almost like bounding box, but angular
vec4 get_rad_vecs(vec2 center, mat2 mrot, vec2 light) {
    vec4 res;

    float param_1 = abs(value_1);
    float param_2 = abs(value_2);
    float param_3 = abs(value_3);

    float sdf_radius = abs(extra_sdf_radius);
    
    switch (shape_type) {
        case SHAPE_CIRCLE: {
            res = findTangentContactPoints(center, value_1, light);
            break; 
        }
        case SHAPE_SQUARE: {
            vec2 hsize = vec2(value_1, value_2);
            vec2 center1 = center + ((hsize-sdf_radius) * vec2(-1,-1)) * mrot;
            vec2 center2 = center + ((hsize-sdf_radius) * vec2(-1,+1)) * mrot;
            vec2 center3 = center + ((hsize-sdf_radius) * vec2(+1,-1)) * mrot;
            vec2 center4 = center + ((hsize-sdf_radius) * vec2(+1,+1)) * mrot;
            vec4 res1 = findTangentContactPoints(center1, sdf_radius, light);
            vec4 res2 = findTangentContactPoints(center2, sdf_radius, light);
            vec4 res3 = findTangentContactPoints(center3, sdf_radius, light);
            vec4 res4 = findTangentContactPoints(center4, sdf_radius, light);

            res = findExtremeTangentAngles(res1,res2,res3,res4);
            break;
        }
        case SHAPE_CAPSULE: {
            float hlen = value_2;
            float radius = value_1;
            vec2 center1 = center + (vec2(-(hlen),0)) * mrot;
            vec2 center2 = center + (vec2(+(hlen),0)) * mrot;
            vec4 res1 = findTangentContactPoints(center1, radius, light);
            vec4 res2 = findTangentContactPoints(center2, radius, light);

            res = findExtremeTangentAngles(res1,res2);
            break; 
        }
        case SHAPE_TRAPEZOID: {
            float bottom_hw   = value_1; 
            float top_hw      = value_2; 
            float half_height = value_3;
            vec2 center1 = center + (vec2(-bottom_hw,-half_height)) * mrot;
            vec2 center2 = center + (vec2(+bottom_hw,-half_height)) * mrot;
            vec2 center3 = center + (vec2(-top_hw,+half_height)) * mrot;
            vec2 center4 = center + (vec2(+top_hw,+half_height)) * mrot;
            vec4 res1 = findTangentContactPoints(center1, sdf_radius, light);
            vec4 res2 = findTangentContactPoints(center2, sdf_radius, light);
            vec4 res3 = findTangentContactPoints(center3, sdf_radius, light);
            vec4 res4 = findTangentContactPoints(center4, sdf_radius, light);

            res = findExtremeTangentAngles(res1,res2,res3,res4);
            break;
        }
    }
    return res;
}

vec2 get_shadow_wpos(){
    //TODO: determine actual cornerns that bound BEFORE
    vec2 rotation = vec2(cos(rotation_angle),sin(rotation_angle));
    mat2 mrot = mat2(rotation.x, rotation.y, -rotation.y, rotation.x);
    //can be computed via multiple different methods. Or precisely like this
    //surely not the best implementation, but does the job and not many vertices anyway
    // vec2 light_pos = ubo.light_pos;
    vec2 light_pos = vec2(-1,8);
    vec4 rads = get_rad_vecs(world_center, mrot, light_pos);
    vec2 rad1 = rads.rg + light_pos;
    vec2 rad2 = rads.ba + light_pos;
    
    vec2 p1 = rad1; 
    vec2 p2 = rad2; 

    if(gl_VertexIndex==0) return p1;
    else return p2;

    // return position;
}

#include "common/map_shadow.glsl"

layout(location = 1) flat out uint out_shape_type;
layout(location = 2) out vec2 world_pos;
layout(location = 3) flat out float out_radius;
layout(location = 4) flat out float out_value_1;
layout(location = 5) flat out float out_value_2;
layout(location = 6) flat out float out_value_3;
layout(location = 7) out vec2 local_pos;

void main(){
    world_pos = get_shadow_wpos();
    
    vec2 clip_depth = world2shadowClip(world_pos);

    out_shape_type = shape_type;
    // out_clip = clip_depth.x;
    // out_depth = clip_depth.y;
    out_value_1 = value_1;
    out_value_2 = value_2;
    out_value_3 = value_3;
    out_radius = extra_sdf_radius;

    // vec2 relative_pos = world_pos - ubo.camera_pos;
    // vec2 clip = relative_pos / ubo.camera_scale;

    // same as default.vert
    // now transform 2d -> 1d  


    //what a weird composition...
    gl_Position = vec4(clip_depth.x, 0, clip_depth.y, 1);
}