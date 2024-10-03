#version 450

#include "common/header.glsl"
#include "noise/perlin.glsl"

vec2 distort(float time, vec2 uv) {
    uv += 0.75 * perlinNoise2d(uv * 4.0 + float(time) + perlinNoise2d(uv * 8.0 - float(time)) / 2.0) / 2.0;
    return uv;
}

vec3 noise(uvec3 iuv) {
    const int LOOP = 5;
    iuv *=uvec3(5332,2524,7552);
    for(int i = 0; i < LOOP; i++)
    	iuv += (iuv.yzx<<2) ^ (iuv.yxz)+iuv.z;
    return vec3(iuv)*(1.0/float(0xffffffffU));
}

vec3 draw(in vec2 fragCoord ){
    // vec2 new_uv = distort(ubo.time*10, fragCoord);
    float time = ubo.time*20;
    vec2 uv = fragCoord/vec2(ubo.resolution);
    uvec3 p = uvec3(fragCoord, time + uv.x*uv.y);
	return noise(p);
}

void main() {
    vec3 color = draw(gl_FragCoord.xy);  // call render function
    outColor = vec4(color, 1.0);  // output final fragment color
}