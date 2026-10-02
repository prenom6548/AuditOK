#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform vec4 texClamp;
uniform vec2 halfpixel;
uniform float offset;

uniform sampler2D texture;

uniform float alpha;
uniform float antiAliasing;
uniform float aspect; // width / height
uniform vec4 roundCornerRadius;

varying vec2 uv;
varying vec2 sdfUv;

vec4 linearSampleClamp(sampler2D tex, vec2 uv_coord)
{
    vec2 clamped_uv = clamp(uv_coord, texClamp.xy, texClamp.zw);
    return texture2D(tex, clamped_uv);
}

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r)
{
    r.xy = (p.x >0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

void main()
{
    vec4 sum = linearSampleClamp(texture, uv + vec2(-halfpixel.x * 2.0, 0.0) * offset);
    sum += linearSampleClamp(texture, uv + vec2(-halfpixel.x, halfpixel.y) * offset) * 2.0;
    sum += linearSampleClamp(texture, uv + vec2(0.0, halfpixel.y * 2.0) * offset);
    sum += linearSampleClamp(texture, uv + vec2(halfpixel.x, halfpixel.y) * offset) * 2.0;
    sum += linearSampleClamp(texture, uv + vec2(halfpixel.x * 2.0, 0.0) * offset);
    sum += linearSampleClamp(texture, uv + vec2(halfpixel.x, -halfpixel.y) * offset) * 2.0;
    sum += linearSampleClamp(texture, uv + vec2(0.0, -halfpixel.y * 2.0) * offset);
    sum += linearSampleClamp(texture, uv + vec2(-halfpixel.x, -halfpixel.y) * offset) * 2.0;
    vec4 texColor = sum * 0.0833333333 * alpha;

    vec2 st = sdfUv;
    st.x *= aspect;
    vec2 rectHalfSize = vec2(aspect, 1.0) * 0.5;
    vec2 rectCenter = st - rectHalfSize;
    float dist = sdRoundedBox(rectCenter, rectHalfSize, roundCornerRadius);
    float halfAA = antiAliasing * 0.5;
    float shape = 1.0 - smoothstep(-halfAA, halfAA, dist);
    gl_FragColor = texColor * shape;
}
