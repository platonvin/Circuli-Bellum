#version 450

#include "common/header.glsl"
#include "noise/random.glsl"
#include "noise/fbm.glsl"

// SPECIAL - coloring_info.w means unique id for hashing

const int NUM_WEDGES = 16;
const float PI = 3.141592653589793238;
const float TWO_PI = 6.28318530718;
const float ANGLE_STEP = TWO_PI / float(NUM_WEDGES); // step between wedges
const float INNER_RADIUS = 0.4;  // inner empty radius
const float CIRCLE_RADIUS = 0.7; // opaque body radius
const float WEDGE_LENGTH = 0.3;
const int NUM_SAMPLE = 8;

const float SEGENDS_SMOOTHNESS = 0.1;
const float SEGSIDE_SMOOTHNESS = 0.1;

float sawBladeSDFSample(in vec2 uv, float angleShift) {
    float angle = atan(uv.y, uv.x) + angleShift;
    angle += ubo.time * 7.73 / 1.0;
    // angle += ubo.time * 7.73 / 10.0;
    float radius = length(uv);

    //[-PI, +PI]
    angle = mod(angle+PI, TWO_PI)-PI;
    
    //curve radius so inside is curved too
    // radius += fbm(sin(angle), 10, .9)/64.0;


    // inside body
    if (radius < CIRCLE_RADIUS) {
        float inside = smoothstep(
             - 0.02,
             + 0.02,
            CIRCLE_RADIUS-radius
        );

        float segment_f = radius / ((CIRCLE_RADIUS - INNER_RADIUS) / 8.0);
        int segment = int(segment_f);
        float segment_blend = 
            (smoothstep(float(segment) - SEGSIDE_SMOOTHNESS, float(segment) + SEGSIDE_SMOOTHNESS, segment_f))
            *(1.0 -smoothstep(float(segment+1) - SEGSIDE_SMOOTHNESS, float(segment+1) + SEGSIDE_SMOOTHNESS, segment_f))
            ;

        float angular_shift = TWO_PI * hash_u1f1(segment + coloring_info.w);
        angular_shift += ubo.time * float(segment) / 1.0;

        float seg_angular_length = 0.7 + 1.0 * hash_u1f1(segment * 128 + coloring_info.w);

        // Normalize the start and end to [-Pi, Pi]
        float start = mod(angular_shift - seg_angular_length + PI, TWO_PI) - PI;
        float end = mod(angular_shift + seg_angular_length + PI, TWO_PI) - PI;

        float smooth_val = 0.0;

        // wraps around from -Pi to Pi
        if (start < end) {
            // normal case: angle is between start and end
            smooth_val = smoothstep(start - SEGENDS_SMOOTHNESS, start, angle) *
                        (1.0 - smoothstep(end, end + SEGENDS_SMOOTHNESS, angle));
        } else {
            // wrapped case: angle is either in [start, Pi] or [-Pi, end]
            smooth_val = smoothstep(start - SEGENDS_SMOOTHNESS, start, angle) +
                        (1.0 - smoothstep(end, end + SEGENDS_SMOOTHNESS, angle));
        }

        // combine smoothness alphas
        float alpha = 1;
            // alpha*=inside;
            alpha*=smooth_val;
            // alpha*=min(smooth_val,inside);
            alpha*=segment_blend;
        return alpha;
    }
    
    float alpha = 1;

    alpha *= smoothstep(
            CIRCLE_RADIUS - 0.02,
            CIRCLE_RADIUS + 0.02,
            radius
        );

    // add rotation and inward curve
    angle += pow(radius + INNER_RADIUS, 2.0) * 0.5;
    // float rnd = hash_f2f1(angle, radius);

    // relative angle in wedge
    // float wedgeIndex = mod(angle, ANGLE_STEP);
    float wedgeIndex = mod(angle, ANGLE_STEP);
    float wedgeOffset = abs(wedgeIndex - ANGLE_STEP / 2.0);

    // vary wedge width using sin
    float wedgeWeirdness = 0.5 + 0.5 * sin(wedgeIndex * 200.0);
    wedgeWeirdness = 1;

    float maxAllowedOffset = (1.0 - (radius - CIRCLE_RADIUS) / WEDGE_LENGTH) * (ANGLE_STEP / 2.0) * wedgeWeirdness;
    maxAllowedOffset *= 2.0; // make wedges thicker

    // check if inside a wedge but smooth (aka sdf multisampling) 
    alpha *= 1.0 - smoothstep(
        maxAllowedOffset-0.15,
        maxAllowedOffset+0.0,
        wedgeOffset);
    
    return alpha;
}

// sofware multisampled. Not used atm
float sawBladeSDF(in vec2 uv) {
    float total = 0.0;
    float angleStep = .15 / float(NUM_SAMPLE);  // step size

    for (int i = 0; i < NUM_SAMPLE; ++i) {
        float angleShift = angleStep * float(i);  // shift angle for each sample
        total += sawBladeSDFSample(uv, angleShift);
    }

    // average samples
    return total / float(NUM_SAMPLE);
}

vec4 draw(in vec2 uv) {
    float sdf = sawBladeSDFSample(uv, 0);
    return vec4(vec3(sdf != 0), sdf);
    // return vec4(sawBladeSDF(uv)); // for software msaa
}

void main() {
    vec2 uv = local_pos / value_1;  // normalize uv
    vec4 baseColor = draw(uv * 1.0);
    baseColor *= vec4(coloring_info.xyz/255.0, 1);
    outColor = baseColor;
}
