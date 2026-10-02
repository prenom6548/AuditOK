shaders = [
  'quad.vert',
  'quad.frag',
  'quad_ex.vert',
  'quad_ex.frag',
  'tex_common.vert',
  'tex_rgba.frag',
  'tex_rgbx.frag',
  'tex_external.frag',
  'tex_ex.vert',
  'tex_rgba_ex.frag',
  'tex_rgbx_ex.frag',
  'tex_external_ex.frag',
  'tex_mix.vert',
  'tex_mix.frag',
]

foreach name : shaders
  output = name.underscorify() + '_str.h'
  var = name.underscorify() + '_str'
  wlcom_sources += custom_target(
    output,
    command: [embed, var],
    input: name,
    output: output,
    feed: true,
    capture: true,
  )
endforeach
