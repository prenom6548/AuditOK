uniform mat3 uv2ndc;
uniform mat3 inverseTransform;

attribute vec2 inUV;

varying vec2 vUV;

void main() {
    gl_Position = vec4(uv2ndc * vec3(inUV, 1.0), 1.0);
    vUV = (inverseTransform * vec3(inUV, 1.0)).xy;
}
