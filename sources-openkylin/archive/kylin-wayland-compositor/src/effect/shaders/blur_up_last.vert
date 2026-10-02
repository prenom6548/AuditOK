attribute vec2 pos;

uniform vec2 frambufferSize;
uniform vec4 dstBox;
uniform mat3 sdfUvRotation;

varying vec2 uv;
varying vec2 sdfUv;

void main()
{
    sdfUv = (pos - dstBox.xy) / dstBox.zw;
    sdfUv = (sdfUvRotation * vec3(sdfUv, 1.0)).xy;

    uv = pos / frambufferSize;
    vec2 ndcPos = uv * 2.0 - 1.0;
    gl_Position = vec4(ndcPos, 0.0, 1.0);
}
