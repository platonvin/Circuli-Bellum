#version 450

#include "common/header.glsl"
#include "noise/fbm.glsl"

// fBm-based color calculation
// vec3 pos for future effect
vec4 calculatePlainColor(vec3 position, vec4 attenuation) {
    vec4 resultColor = vec4(0.0);
    float noiseValue = fbm(position);
    resultColor = vec4(noiseValue * attenuation.rgb, 1.0);  // Apply attenuation to RGB channels
    return resultColor;
}

vec4 calculateAttenuation(float time) {
    // daynamic attenuation based on time, with variation in color channels
    return vec4(1.0, 0.9, 0.3, 0.15) + vec4(0.06 * cos(time * 0.1), 0.02 * sin(time * 0.15), 0.08 * cos(time * 0.3), 0.14 * cos(time * 0.23));
}

vec3 calculatePulseColor(vec2 uvCoords, float time) {
    vec3 finalColor = vec3(0.0);
    vec4 attenuation = calculateAttenuation(time);

    // float scaleFactor = sin(time);  // size pulse
    float scaleFactor = 10;  // size pulse
    return calculatePlainColor(vec3(uvCoords * scaleFactor, time), attenuation).xyz;
}

vec3 renderFragment(vec2 fragCoord, vec2 screenSize, float time, vec3 colorMultiplier) {
    vec2 uv = (fragCoord - screenSize * 0.5) / screenSize.y;
    vec3 baseColor = calculatePulseColor(uv, time);
    baseColor *= colorMultiplier;  // apply coloring info
    return pow(baseColor, vec3(0.77));
    return baseColor;
}

vec3 draw(vec2 fragCoord) {
    vec3 colorMultiplier = (vec3(coloring_info)/255.0);  // treat as сolor multiplier

    return renderFragment(fragCoord, ubo.resolution.xy, ubo.time/5.0, colorMultiplier);
}

void main() {
    vec3 color = draw(gl_FragCoord.xy);
    outColor = vec4(color, getAlphaFromShape());
}