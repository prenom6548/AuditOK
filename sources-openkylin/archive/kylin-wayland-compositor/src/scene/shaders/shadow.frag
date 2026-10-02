#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform vec4 color;
uniform float sigma;
uniform vec4 rect;
uniform float roundRadius;

uniform float windowAntiAliasing;
uniform float windowAspect;
uniform vec4 windowRect;
uniform vec4 windowRoundRadius;

varying vec2 vPos;
varying vec2 vUV;

// License: CC0 (http://creativecommons.org/publicdomain/zero/1.0/)

// A standard gaussian function, used for weighting samples
float gaussian(float x, float sigma) {
    const float pi = 3.141592653589793;
    return exp(-(x * x) / (2.0 * sigma * sigma)) / (sqrt(2.0 * pi) * sigma);
}

// This approximates the error function, needed for the gaussian integral
vec2 erf(vec2 x) {
    vec2 s = sign(x), a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x *= x;
    return s - s / (x * x);
}

// Return the blurred mask along the x dimension
float roundedBoxShadowX(float x, float y, float sigma, float corner, vec2 halfSize) {
    float delta = min(halfSize.y - corner - abs(y), 0.0);
    float curved = halfSize.x - corner + sqrt(max(0.0, corner * corner - delta * delta));
    vec2 integral = 0.5 + 0.5 * erf((x + vec2(-curved, curved)) * (sqrt(0.5) / sigma));
    return integral.y - integral.x;
}

#define MULTI_SAMPLE_COUNT 3

// Return the mask for the shadow of a box from lower to upper
float roundedBoxShadow(vec2 lower, vec2 upper, vec2 point, float sigma, float corner) {
    // Center everything to make the math easier
    vec2 center = (lower + upper) * 0.5;
    vec2 halfSize = (upper - lower) * 0.5;
    point -= center;

    // The signal is only non-zero in a limited range, so don't waste samples
    float low = point.y - halfSize.y;
    float high = point.y + halfSize.y;
    float start = clamp(-3.0 * sigma, low, high);
    float end = clamp(3.0 * sigma, low, high);

    // Accumulate samples
    float step = (end - start) / float(MULTI_SAMPLE_COUNT);
    float y = start + step * 0.5;
    float value = 0.0;
    for (int i = 0; i < MULTI_SAMPLE_COUNT; i++) {
        value += roundedBoxShadowX(point.x, point.y - y, sigma, corner, halfSize) * gaussian(y, sigma) * step;
        y += step;
    }

    return value;
}

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r) {
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main() {
    vec2 st = vUV * 2.0;
    st.x *= windowAspect;
    vec2 windowOffset = -windowRect.xy * 2.0;
    float windowDist = sdRoundedBox(st + windowOffset, windowRect.zw, windowRoundRadius);
    float windowMask = smoothstep(-windowAntiAliasing, windowAntiAliasing, windowDist);

    float shadow = roundedBoxShadow(rect.xy, rect.zw, vPos, sigma, roundRadius);
    gl_FragColor = color * shadow * windowMask;
}
