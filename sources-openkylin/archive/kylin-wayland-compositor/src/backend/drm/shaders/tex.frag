#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

//in
varying vec2 v_texcoord;

uniform sampler2D tex;

uniform float contrast;

uniform vec3 whitepoint;

uniform mat4 color_matrix;

uniform float brightness;


void main()
{
   vec4 color = texture2D(tex, v_texcoord);
   color.rgb = (color.rgb - 0.5) * contrast + 0.5;
   color.rgb *= whitepoint;
   color.rgb = (color_matrix * vec4(color.rgb, color.a)).rgb;
   gl_FragColor = color * brightness;
}
