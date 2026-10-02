#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform float antiAliasing;
uniform float aspect; // width / height
uniform vec4 rect; // distance. x y w h
uniform vec4 roundRadius;

uniform float borderThickness;
uniform vec4 borderColor;

uniform float titleHeight;
uniform vec4 titleColor;

varying vec2 vUV;

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r) {
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

float sdOpSubtraction(float a, float b) {
    return max(a, -b);
}

void main() {
    vec2 st = vUV * 2.0;
    st.x *= aspect;
    vec2 offset = -rect.xy * 2.0;

    float outerDist = sdRoundedBox(st + offset, rect.zw, roundRadius);
    float innerDist = outerDist + borderThickness;
    float borderDist = sdOpSubtraction(outerDist, innerDist);
    float borderShape = 1.0 - smoothstep(-antiAliasing, antiAliasing, borderDist);

    // title
    float titleShape = 0.0;
    if (titleHeight > 0.001) {
        float titleDist = sdRoundedBox(
            st + offset + vec2(0.0, rect.w - titleHeight - borderThickness),
            vec2(rect.z - borderThickness, titleHeight),
            vec4(0.0, roundRadius[1] - borderThickness, 0.0, roundRadius[3] - borderThickness));
        titleShape = 1.0 - smoothstep(-antiAliasing, antiAliasing, titleDist);
    }
    
    gl_FragColor = titleShape * titleColor + borderShape * borderColor;
}
