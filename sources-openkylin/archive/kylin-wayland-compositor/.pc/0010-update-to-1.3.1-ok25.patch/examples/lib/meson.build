wlu_sources = files(
  'src/buffer.c',
  'src/display.c',
  'src/input.c',
  'src/window.c',
)

protos = [
  'xdg-shell',
  'xdg-decoration', 
  'linux-dmabuf-v1',
]

foreach p : protos
  wlu_sources += protocols_code[p]
  wlu_sources += protocols_client_header[p]
endforeach

wayland_client = dependency('wayland-client')
pixman = dependency('pixman-1')
drm = dependency('libdrm')
gbm = dependency('gbm')
egl = dependency('egl')
gles = dependency('glesv2')

wlu_deps = [
  wayland_client,
  pixman,
  drm,
  gbm,
  egl,
  gles
]

wlu_inc = include_directories('include')

wlu_lib = static_library('wlu',
  wlu_sources,
  dependencies: wlu_deps,
  include_directories: wlu_inc,
)

libwlu = declare_dependency(
  link_with: wlu_lib,
  dependencies: wlu_deps,
  include_directories: wlu_inc,
)
