float randomValue(vec2 position) {
    return fract(sin(dot(position, vec2(12.543, 514.123))) * 4732.12);
}
vec2 gradient(vec2 p) {
    return normalize(vec2(cos(dot(p, vec2(12.9898, 78.233))), sin(dot(p, vec2(12.9898, 78.233)))));
}
vec2 fade(vec2 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

#define perlinNoise2D perlinNoise2d
float perlinNoise2d(vec2 position) {
    vec2 integerPart = floor(position);
    vec2 fractionalPart = fract(position);

    vec2 g00 = gradient(integerPart);
    vec2 g10 = gradient(integerPart + vec2(1.0, 0.0));
    vec2 g01 = gradient(integerPart + vec2(0.0, 1.0));
    vec2 g11 = gradient(integerPart + vec2(1.0, 1.0));

    vec2 d00 = fractionalPart;
    vec2 d10 = fractionalPart - vec2(1.0, 0.0);
    vec2 d01 = fractionalPart - vec2(0.0, 1.0);
    vec2 d11 = fractionalPart - vec2(1.0, 1.0);

    float n00 = dot(g00, d00);
    float n10 = dot(g10, d10);
    float n01 = dot(g01, d01);
    float n11 = dot(g11, d11);

    vec2 fadeFactor = fade(fractionalPart);

    float nx0 = mix(n00, n10, fadeFactor.x);
    float nx1 = mix(n01, n11, fadeFactor.x);
    return mix(nx0, nx1, fadeFactor.y);
}

float randomValue(vec3 position) {
    return fract(sin(dot(position, vec3(12.543, 514.123, 23.754))) * 4732.12);
}
vec3 gradient(vec3 p) {
    return normalize(vec3(cos(dot(p, vec3(12.9898, 78.233, 45.654))), 
                         sin(dot(p, vec3(12.9898, 78.233, 45.654))), 
                         cos(dot(p, vec3(78.233, 12.9898, 23.754)))));
}
vec3 fade(vec3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

#define perlinNoise3D perlinNoise3d 
float perlinNoise3d(vec3 position) {
    vec3 integerPart = floor(position);
    vec3 fractionalPart = fract(position);

    vec3 g000 = gradient(integerPart);
    vec3 g100 = gradient(integerPart + vec3(1.0, 0.0, 0.0));
    vec3 g010 = gradient(integerPart + vec3(0.0, 1.0, 0.0));
    vec3 g110 = gradient(integerPart + vec3(1.0, 1.0, 0.0));
    vec3 g001 = gradient(integerPart + vec3(0.0, 0.0, 1.0));
    vec3 g101 = gradient(integerPart + vec3(1.0, 0.0, 1.0));
    vec3 g011 = gradient(integerPart + vec3(0.0, 1.0, 1.0));
    vec3 g111 = gradient(integerPart + vec3(1.0, 1.0, 1.0));

    vec3 d000 = fractionalPart;
    vec3 d100 = fractionalPart - vec3(1.0, 0.0, 0.0);
    vec3 d010 = fractionalPart - vec3(0.0, 1.0, 0.0);
    vec3 d110 = fractionalPart - vec3(1.0, 1.0, 0.0);
    vec3 d001 = fractionalPart - vec3(0.0, 0.0, 1.0);
    vec3 d101 = fractionalPart - vec3(1.0, 0.0, 1.0);
    vec3 d011 = fractionalPart - vec3(0.0, 1.0, 1.0);
    vec3 d111 = fractionalPart - vec3(1.0, 1.0, 1.0);

    float n000 = dot(g000, d000);
    float n100 = dot(g100, d100);
    float n010 = dot(g010, d010);
    float n110 = dot(g110, d110);
    float n001 = dot(g001, d001);
    float n101 = dot(g101, d101);
    float n011 = dot(g011, d011);
    float n111 = dot(g111, d111);

    vec3 fadeFactor = fade(fractionalPart);

    float nx00 = mix(n000, n100, fadeFactor.x);
    float nx10 = mix(n010, n110, fadeFactor.x);
    float nx01 = mix(n001, n101, fadeFactor.x);
    float nx11 = mix(n011, n111, fadeFactor.x);

    float nxy0 = mix(nx00, nx10, fadeFactor.y);
    float nxy1 = mix(nx01, nx11, fadeFactor.y);

    return mix(nxy0, nxy1, fadeFactor.z);
}
