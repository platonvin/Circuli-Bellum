const int NUM_OCTAVES = 5;

float mod289(float x) {return x - floor(x / 289.0) * 289.0;}
vec4 mod289(vec4 x) {return x - floor(x / 289.0) * 289.0;}
vec4 permute(vec4 x) {return mod289((x * 34.0 + 1.0) * x);}

float noise(vec3 position) {
    vec3 integerPart = floor(position);
    vec3 fractionalPart = position - integerPart;
    fractionalPart = fractionalPart * fractionalPart * (3.0 - 2.0 * fractionalPart);

    vec4 permBase = integerPart.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    
    vec4 perm1 = permute(permBase.xyxy);
    vec4 perm2 = permute(perm1.xyxy + permBase.zzww);

    vec4 perm3 = permute(perm2 + integerPart.zzzz);
    vec4 perm4 = permute(perm2 + integerPart.zzzz + 1.0);

    vec4 noiseValue1 = fract(perm3 * (1.0 / 41.0));
    vec4 noiseValue2 = fract(perm4 * (1.0 / 41.0));

    vec4 interpolationZ = mix(noiseValue1, noiseValue2, fractionalPart.z);
    vec2 interpolationX = mix(interpolationZ.xz, interpolationZ.yw, fractionalPart.x);

    return mix(interpolationX.x, interpolationX.y, fractionalPart.y);
}

float fbm(vec3 position) {
    float amplitude = 0.5;
    float value = 0.0;
    vec3 shift = vec3(100.0);

    for (int i = 0; i < NUM_OCTAVES; i++) {
        value += amplitude * noise(position);  // add weighted noise value
        position = position * 2.0 + shift;     // increase frequency and apply shift
        amplitude *= 0.5;                      // decrease amplitude for next octave
    }

    return value;
}