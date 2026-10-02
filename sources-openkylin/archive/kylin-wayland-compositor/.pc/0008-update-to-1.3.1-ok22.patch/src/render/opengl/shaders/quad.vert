#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

uniform mat3 uv2ndc;

attribute vec2 inUV;

void main()
{
    vec3 uv3 = vec3(inUV, 1.0);
    gl_Position = vec4(uv2ndc * uv3, 1.0);
}
