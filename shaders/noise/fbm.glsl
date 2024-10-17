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

#define HASHSCALE 0.1031
float hash(float p) {
    #ifdef NOISE_PERIOD
	p = mod(p, NOISE_PERIOD);
    #endif
    
	vec3 p3 = fract(vec3(p) * HASHSCALE);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
float fade(float t) { return t*t*t*(t*(6.*t-15.)+10.); } // interpolation
float grad(float hash, float p) { // gradient
    int i = int(1e4*hash);
	return (i & 1) == 0 ? p : -p;
}
float perlinNoise1D(float p) {
	float pi = floor(p); // integer part of the p
	float pf = p - pi; // fractional part of the p
	float w = fade(pf); // use fractional part of the p to interpolate between two integer edges
    return mix(grad(hash(pi), pf), grad(hash(pi + 1.0), pf - 1.0), w) * 2.0;
}
// returns value in interval [-1..+1]
float fbm(float pos, int octaves, float persistence) {
	float total = 0., frequency = 1., amplitude = 1., maxValue = 0.;
	for(int i = 0; i < octaves; ++i) {
		total += perlinNoise1D(pos * frequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.;
	}
	return total / maxValue;
}