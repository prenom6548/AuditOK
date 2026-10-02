uniform mat3 uv2ndc;
uniform mat3 uvRotation;

attribute vec2 inUV;

varying vec2 vUV;
varying vec2 vUVSDF;

void main() {
    vec3 uv3 = vec3(inUV, 1.0);
    gl_Position = vec4(uv2ndc * uv3, 1.0);
    vUV = inUV;
    vUVSDF = (uvRotation * uv3).xy;
}
