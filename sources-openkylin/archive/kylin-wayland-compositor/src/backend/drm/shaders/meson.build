shaders = [
  'tex.vert',
  'tex.frag',
  'tex_oes.frag',
  'tex_yuv.frag',
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
