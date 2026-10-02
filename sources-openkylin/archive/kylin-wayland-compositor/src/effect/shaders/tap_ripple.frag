#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

varying vec2 uv;

uniform float antiAliasing;
uniform float radius;
uniform float attenuation;
uniform vec3 color;

void main() {
    float dist = distance(uv, vec2(0.5, 0.5));
    float dist2 = smoothstep(-antiAliasing, antiAliasing, radius - dist);

    float alpha = radius - dist;
    alpha = 1.0 - alpha - attenuation;
    alpha *= dist2;
    gl_FragColor = vec4(color * dist2 * alpha, alpha);
}
