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

varying vec2 uv;

vec4 linearSampleClamp(sampler2D tex, vec2 uv_coord)
{
    vec2 clamped_uv = clamp(uv_coord, texClamp.xy, texClamp.zw);
    return texture2D(tex, clamped_uv);
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

    gl_FragColor = sum * 0.0833333333;
}
