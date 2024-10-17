// // Convenient helper functions that do salting etc.
// uvec4 rndU44(uvec4 u) { return u.yzwx * u.zwxy ^ u; }
// uvec4 h44uvc(vec4  c) { return uvec4(c * 22.33 + 33.33); }
// vec4  hlpr24(vec4  c) { return fract(fract(c) / fract(c.wxyz * c.zwxy + c.yzwx)); }
// vec4  colc44(vec4  c) { return smoothstep(0.4, 0.6, c / 43e8); }
// vec3  hlpr23(vec2  v) { return fract(fract(v *= v.y + 333.3).xyx / fract(v.yxy * v.xxy)); }
// vec2  h1toh2(float x) { return vec2(x / modf(x, x), x / 33e3 + 0.03); }
// vec4  h3toh4(vec3  p) { return vec4(p.x * p.y + p.z, p); }
// vec4  h2toh4(vec2  v) { return vec4(v / 3.33 + 321.0, v * 1e3 + 333.3); }
// float v2tofl(vec2  v) { return v.x * (v.y / 12.34 + 56.78); }

// // Four dimensional input versions of hashing.
// vec4  hash44(vec4  c) { return colc44(vec4(rndU44(rndU44(h44uvc(c))))); }
// vec3  hash43(vec4  c) { return hash44(c).xyz; }
// vec2  hash42(vec4  c) { return hash44(c).yz; }
// float hash41(vec4  c) { return hash44(c).w; }

// // Three dimensional input versions of hashing.
// vec4  hash34(vec3  p) { return hash44(h3toh4(p));}
// vec3  hash33(vec3  p) { return hash43(h3toh4(p));}
// vec2  hash32(vec3  p) { return hash42(h3toh4(p));}
// float hash31(vec3  p) { return hash41(h3toh4(p));}

// // Two dimensional input versions of hashing.
// vec4  hash24(vec2  v) { return hash44(v.xyxx * ubo.resolution.xyyx + v.yxyx); }
// vec3  hash23(vec2  v) { return hash24(v).xyz; }
// vec2  hash22(vec2  v) { return hash24(v).xy; }
// float hash21(vec2  v) { return hash24(v).x; }

// // One dimensional input versions of hashing.
// vec4  hash14(float x) { return hash24(h1toh2(x)); }
// vec3  hash13(float x) { return hash23(h1toh2(x)); }
// vec2  hash12(float x) { return hash22(h1toh2(x)); }
// float hash11(float x) { return hash21(h1toh2(x)); }

// float hash31(vec3 p3){
// 	p3  = fract(p3 * MOD3);
//     p3 += dot(p3, p3.yzx + 19.19);
//     return -1.0 + 2.0 * fract((p3.x + p3.y) * p3.z);
// }
uint uhash21(uvec2 p){
    p *= uvec2(73333,7777);
    p ^= (uvec2(3333777777)>>(p>>28));
    uint n = p.x*p.y;
    return n^(n>>15);
}

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21)); 
    p += dot(p, p + 78.23);
    return fract(p.x * p.y);
}
vec2 hash22(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 78.23);
    return fract(vec2(p.x * p.y, p.y * p.x));
}
float hash31(vec3 p) {
    p = fract(p * vec3(443.8975, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}
vec2 hash32(vec3 p) {
    p = fract(p * vec3(443.8975, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract(vec2(p.x * p.z, p.y * p.x));
}