uniform mat3 uv2texcoord;
uniform mat3 uv2texcoord2;
uniform mat3 uv2ndc;

attribute vec2 inUV;

varying vec2 v_texCoord;
varying vec2 v_texCoord2;

void main()
{
    vec3 uv3 = vec3(inUV, 1.0);
    v_texCoord = (uv2texcoord * uv3).xy;
    v_texCoord2 = (uv2texcoord2 * uv3).xy;
    gl_Position = vec4(uv2ndc * uv3, 1.0);
}
