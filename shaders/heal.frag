#version 450

#include "common/header.glsl"
#include "noise/random.glsl"

// draws green crosses

const float CELL_SIZE = 18+0;
const int LAYER_COUNT = 1;
const float TWO_PI = 6.28318530718;

float time = ubo.time + 420.0;

// credits to IQ
float sdfCross(in vec2 p, in vec2 b, float r) {
    p = abs(p); p = (p.y>p.x) ? p.yx : p.xy;
    vec2  q = p - b;
    float k = max(q.y,q.x);
    vec2  w = (k>0.0) ? q : vec2(b.y-p.x,-k);
    return sign(k)*length(max(w,0.0)) + r;
}
mat2 rot2d(float a){
    return mat2(cos(a),-sin(a),
                sin(a),cos(a));
}

// damn thats a lot of hashing
float getLayer(vec2 frag_coords, int layerIndex) {
    ivec2 baseCellPos = ivec2(floor(frag_coords / CELL_SIZE));
    float result = 0.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            ivec2 cellPos = baseCellPos + ivec2(x, y);
            float uh = hash21(vec2(cellPos) * 41.152);
            vec2 h2 = hash22(vec2(cellPos) * 17.21) * 2.0 - 1.0;

            vec2 randomOffset = vec2(
                hash21(vec2(float(layerIndex) + uh, float(cellPos.x))),
                hash21(vec2(float(layerIndex) + uh, float(cellPos.y)))
            ) * CELL_SIZE * 0.5;

            randomOffset.x *= sin(time * 0.5 + h2.x);
            randomOffset.y *= sin(time * 0.6 + h2.y);

            vec2 cellCenter = (vec2(cellPos) + 0.5) * CELL_SIZE + randomOffset;
            vec2 local_pos = frag_coords - cellCenter;
            local_pos *= rot2d(hash31(vec3(float(layerIndex) * 36.234, vec2(cellPos) * 24.610)) * TWO_PI + time * 0.3);

            vec2 crossSize = vec2(CELL_SIZE * 0.4, CELL_SIZE * 0.15);
            float sizeModifier = 0.9 + 0.1 * sin(time*uh + float(layerIndex));
            crossSize *= sizeModifier;

            result += (sdfCross(local_pos, crossSize, 0.0) < 0.0) ? 1.0 : 0.0;
        }
    }

    return result;
}

vec4 draw(vec2 frag_coords) {
    float result = 0.0;

    for (int i = 0; i < LAYER_COUNT; i++) {
        result += getLayer(frag_coords, i);
    }

    // result /= LAYER_COUNT;

    return vec4(result); 
}

float getAlphaFromCircle() {
    float shape_distance = 0.0;

    float fillrate = abs(value_2);
    float radius = abs(value_1)*fillrate;

    float sdf_radius = abs(extra_sdf_radius);
    
    shape_distance = sdf_circle(local_pos, radius);
    if(sdf_radius != 0){
        if(shape_distance < 0) {
            shape_distance = abs(shape_distance) - sdf_radius;
        }
    }
    return 1.0 - smoothstep(-SOFT_EDGE_RADIUS, SOFT_EDGE_RADIUS, shape_distance);
}

void main() {
    // vec2 uv = local_pos / value_1;
    // vec4 baseColor = draw(uv * 2.0);
    vec4 baseColor = draw(gl_FragCoord.xy);
    // vec4 baseColor = draw(uv);
    baseColor *= vec4(coloring_info.xyz, 255) / 255.0;
    baseColor.a *= getAlphaFromCircle();
    outColor = baseColor;
}
