uniform mat3 uvRotation;
uniform mat3 uv2texcoord;
uniform mat3 uv2ndc;

attribute vec2 inUV;

varying vec2 vUV;
varying vec2 vTexcoord;

void main()
{
    vec3 uv3 = vec3(inUV, 1.0);
    vUV = (uvRotation * uv3).xy;
    vTexcoord = (uv2texcoord * uv3).xy;
    gl_Position = vec4(uv2ndc * uv3, 1.0);
}
