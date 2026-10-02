#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

varying vec2 v_texCoord;
varying vec2 v_texCoord2;

uniform sampler2D textureA;
uniform sampler2D textureB;
uniform float blendFactor;
uniform float alpha;

void main() {
    vec2 isInDst = step(vec2(0.0, 0.0), v_texCoord2) * step(v_texCoord2, vec2(1.0, 1.0));
    float validDst = isInDst.x * isInDst.y;

    vec4 colorA = texture2D(textureA, v_texCoord);
    vec4 colorB = texture2D(textureB, v_texCoord2);

    // mixColor:  (A * (1 - f) + B * f)
    vec4 mixColor = mix(colorA, colorB, blendFactor);

    gl_FragColor = mix(colorA, mixColor, validDst) * alpha;
}
