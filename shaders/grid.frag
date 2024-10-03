#version 450

#include "common/header.glsl"
#include "common/colors.glsl"
#include "noise/perlin.glsl"

//TODO: pulse size and color

const float gridCellSize = 10.0;  
const float quadSize = 8.0;       
const float pulsationSpeed = 5.5; 

vec3 shiftColor(vec3 color, float randomShift) {
    vec3 hsv = rgb2hsv(color);

    float hueShift = fract(hsv.x + randomShift * 0.3);
    float saturationShift = clamp(hsv.y + (randomShift - 0.5) * 0.4, 0.0, 1.0);
    float brightnessShift = clamp(hsv.z + (randomShift - 0.5) * 0.4, 0.0, 1.0);

    vec3 shiftedHsb = vec3(hueShift, saturationShift, brightnessShift);

    return hsv2rgb(shiftedHsb);
}

vec4 draw(vec2 fragCoord) {
    vec2 gridPosition = floor(fragCoord / gridCellSize) * gridCellSize;
    vec2 localPosition = fragCoord - gridPosition;  // Local position within the grid cell

    if ((localPosition.x < quadSize) && (localPosition.y < quadSize)) {
        float pulseOffset = 100.0*randomValue(gridPosition);
        float colorOffset = randomValue(gridPosition*2.0);
        float pulse = 0.6 + 0.4*sin(ubo.time + pulseOffset);

        vec3 color = vec3(coloring_info)/255.0;
        color = shiftColor(color, colorOffset / 10.0);
        color *= pulse;
        return vec4(color, 1);
    }

    return vec4(0);
}

void main() {
    vec4 color = draw(gl_FragCoord.xy);
    color.a *= getAlphaFromShape();
    outColor = color;
}