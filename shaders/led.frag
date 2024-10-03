#version 450

#include "common/header.glsl"
#include "noise/perlin.glsl"

vec2 distortUV(vec2 uv) {
    uv += 0.75 * perlinNoise2d(uv * 4.0 + float(ubo.time) + perlinNoise2d(uv * 8.0 - float(ubo.time) / 1.5) / 2.0) / 2.0;
    return uv;
}

float circularGridPattern(vec2 fragCoord, float cellSize, float radius) {
    vec2 gridIndex = floor(fragCoord / cellSize);
    vec2 cellCenter = (gridIndex + 0.5) * cellSize;

    float distanceToCenter = length(fragCoord - cellCenter);

    float smoothness = 0.45;  // controls edge smoothness
    float alpha = smoothstep(radius + smoothness, radius - smoothness, distanceToCenter);

    return alpha;
}

vec3 draw(in vec2 fragCoord) {
    vec2 uv = fragCoord / float(ubo.resolution.y);
    uv = distortUV(uv);

    // float grid = checkeredGridPattern(fragCoord, vec2(ubo.resolution.xy), gridSpacing);
    float grid = circularGridPattern(fragCoord, 8, 3.1);
    
    vec3 ledColor = vec3(0.86, 0.16, 0.24);
    vec3 color = mix(vec3(0), ledColor, 5.0 * vec3(pow(1.0 - perlinNoise2d(uv * 4.0 - vec2(0.0, float(ubo.time))), 5.0)));

    color *= grid;
    // color = pow(color, vec3(1.0 / 2.2)); // Apply gamma correction

    return color;
}

void main() {
    // vec2 local_frag_coord = (local_pos*vec2(ubo.resolution)) / ubo.camera_scale * 2.0;
    // vec3 color = draw(local_frag_coord.xy);  // Call render function

    vec4 color = vec4(draw(gl_FragCoord.xy), getAlphaFromShape());

    outColor = color;
} 