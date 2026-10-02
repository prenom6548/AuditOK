#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

varying vec2 vUV;
varying vec2 vTexcoord;
uniform sampler2D tex;
uniform float alpha;

uniform int forceOpaque; // texture alpha force = 1
uniform float antiAliasing;
uniform float aspect; // width / height
uniform vec4 roundCornerRadius;

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r)
{
    r.xy = (p.x >0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main()
{
    vec2 st = vUV;
    st.x *= aspect;
    
    vec2 rectHalfSize = vec2(aspect, 1.0) * 0.5;
    vec2 rectCenter = st - rectHalfSize;
    float dist = sdRoundedBox(rectCenter, rectHalfSize, roundCornerRadius);
    float shape = 1.0 - smoothstep(0.0, antiAliasing, dist);
    vec4 texColor = texture2D(tex, vTexcoord) * alpha;
    if (forceOpaque != 0) {
        texColor.a = 1.0;
    }
    gl_FragColor = texColor * shape;
}
