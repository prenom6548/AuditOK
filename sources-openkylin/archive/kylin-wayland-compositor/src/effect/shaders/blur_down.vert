attribute vec2 pos;

uniform vec2 frambufferSize;

varying vec2 uv;

void main() 
{
    uv = pos / frambufferSize;
    vec2 ndcPos = uv * 2.0 - 1.0;
    gl_Position = vec4(ndcPos, 0.0, 1.0);
}
