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

uniform float pixel_offset_x;


vec4 apply_color_pipeline(vec4 color)
{
   color.rgb = (color.rgb - 0.5) * contrast + 0.5;
   color.rgb *= whitepoint;
   color.rgb = (color_matrix * vec4(color.rgb, color.a)).rgb;
   color *= brightness;
   return color;
}

vec3 rgb2yuv(vec3 rgb) {
   return mat3(
         0.257, -0.148, 0.439,
         0.504, -0.291, -0.368,
         0.098, 0.439, -0.071
         ) * rgb + vec3(0.063, 0.502, 0.502);
}

void main()
{
   vec4 color1 = texture2D(tex, v_texcoord + vec2(-0.5 * pixel_offset_x, 0.0));
   vec4 color2 = texture2D(tex, v_texcoord + vec2(+0.5 * pixel_offset_x, 0.0));

   color1 = apply_color_pipeline(color1);
   color2 = apply_color_pipeline(color2);

   vec3 yuyv0 = rgb2yuv(color1.rgb);
   vec3 yuyv1 = rgb2yuv(color2.rgb);

   float y0 = yuyv0.x;
   float y1 = yuyv1.x;
   float u = 0.5 * (yuyv0.y + yuyv1.y);
   float v = 0.5 * (yuyv0.z + yuyv1.z);

   gl_FragColor = vec4(y1, u, y0, v);
}
