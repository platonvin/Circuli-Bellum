const float Pi = 3.1415926;

// vec2(clip, depth)
vec2 world2shadowClip(vec2 world_pos){
    vec2 center = ubo.light_pos;    
    // POINTERS ON GPU ALARM
    vec2 pointer = world_pos - center;
    vec2 direction = normalize(pointer);

    //[-pi, +pi]
    float angle = atan(direction.y, direction.x);
    //[-1, +1]
    float clip = (angle) / (1.0 * Pi); 

    // float clip = remapped_angle;

    // clip = (gl_VertexIndex % 2)*2.0 - 1.0;
    
    const float MAX_DEPTH = 33.0;
    float depth = length(pointer) / MAX_DEPTH; 

    return vec2(clip, depth);
}