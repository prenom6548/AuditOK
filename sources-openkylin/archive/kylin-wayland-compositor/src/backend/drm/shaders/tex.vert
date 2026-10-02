// in
attribute vec2 in_position;

uniform vec2 size;

//out to frag
varying vec2 v_texcoord;

void main() 
{
   v_texcoord = in_position / size;
   vec2 ndc_pos = v_texcoord * 2.0 - 1.0;
   gl_Position = vec4(ndc_pos, 0.0, 1.0);
}
