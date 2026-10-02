#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform vec2 start;
uniform vec2 end;

struct ColorStop
{
	float offset;
    vec4 color;
};
uniform ColorStop colorStops[2];

uniform float antiAliasing;
uniform float aspect;
uniform vec4 roundCornerRadius;

varying vec2 vUV;
varying vec2 vUVSDF;

float linearStep(float t1, float t2, float x)
{
    return clamp((x - t1) / (t2 - t1), 0.0, 1.0);
}

float linearInterpolate(vec2 p, vec2 p0, vec2 p1)
{
    vec2 v = p1 - p0;
    vec2 d = p - p0;
    return dot(v, d) / dot(v, v);
}

float sdRoundedBox(in vec2 p, in vec2 b, in vec4 r)
{
    r.xy = (p.x >0.0) ? r.xy : r.zw;
    r.x = (p.y > 0.0) ? r.x : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

float roundedRectMask()
{
    vec2 st = vUVSDF;
    st.x *= aspect;

    vec2 rectHalfSize = vec2(aspect, 1.0) * 0.5;
    vec2 rectCenter = st - rectHalfSize;
    float dist = sdRoundedBox(rectCenter, rectHalfSize, roundCornerRadius);
    float shape = 1.0 - smoothstep(-antiAliasing, antiAliasing, dist);
    return shape;
}

void main() {
	float t = linearInterpolate(vUV, start, end);

	vec4 out_color = mix(colorStops[0].color, colorStops[1].color,
        linearStep(colorStops[0].offset, colorStops[1].offset, t));

    gl_FragColor = out_color * roundedRectMask();
}
