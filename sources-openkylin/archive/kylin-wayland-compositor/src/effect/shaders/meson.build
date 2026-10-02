shaders = [
  'blur_down.vert',
  'blur_down.frag',
  'blur_up.vert',
  'blur_up.frag',
  'blur_up_last.vert',
  'blur_up_last.frag',
  'circle_progressbar.vert',
  'circle_progressbar.frag',
  'magic_lamp.vert',
  'magic_lamp.frag',
  'shape_color.vert',
  'shape_color.frag',
  'tap_ripple.vert',
  'tap_ripple.frag',
]

foreach name : shaders
  output = name.underscorify() + '.h'
  var = name.underscorify()
  wlcom_sources += custom_target(
    output,
    command: [embed, var],
    input: name,
    output: output,
    feed: true,
    capture: true,
  )
endforeach
