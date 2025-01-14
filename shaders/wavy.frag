#version 450

#include "common/header.glsl"
#include "noise/fbm.glsl"

// sine-based noise
float sineNoise(vec2 p, float velocityX, float velocityY, float time) {
    return sin(p.x + sin(p.y + time * velocityX)) * sin(p.y * p.x * 0.1 + time * velocityY);
}

vec2 computeField(vec2 p, float time) {
    const vec2 ep = vec2(0.05, 0.0);
    vec2 velocity = vec2(0.0);

    for (int i = 0; i < 16; i++) {  // reduce iterations for a smoother field
        float t0 = sineNoise(p, 0.15, 0.3, time);
        float t1 = sineNoise(p + ep.xy, 0.15, 0.3, time);
        float t2 = sineNoise(p + ep.yx, 0.15, 0.3, time);
        vec2 grad = vec2(t1 - t0, t2 - t0) / ep.x;
        vec2 tangent = vec2(-grad.y, grad.x);

        p += (30.0 * 0.01) * tangent + grad * (1.0 / 150.0);
        p.x += sin(time * 1.5 / 10.0) / 15.0;
        p.y += cos(time * 1.5 / 10.0) / 15.0;

        velocity = grad;
    }
    return velocity;
}

vec3 vel2color(vec2 vel, vec3 baseColor) {
    vec3 velocityColor = vec3(vel * 0.5 + 0.5, 0.5);
    velocityColor.yz = velocityColor.zy;
    return mix(velocityColor, baseColor, 0.5);
}

vec4 draw(vec2 fragCoord) {
    vec2 p = fragCoord / ubo.resolution.xy - 0.5;
    p.x *= ubo.resolution.x / ubo.resolution.y;
    p *= 5.0;
    float time = ubo.time + noise(vec3(coloring_info))*50.0;
    vec2 vel = computeField(p, time);
    
    vec3 color = vel2color(vel, vec3(coloring_info)/255.0);
    
    return vec4(color, 1.0);
}

void main() {
    vec4 color = draw(gl_FragCoord.xy);
    color.a = getAlphaFromShape();
    // color.a = 0.3;
    outColor = color;
}
