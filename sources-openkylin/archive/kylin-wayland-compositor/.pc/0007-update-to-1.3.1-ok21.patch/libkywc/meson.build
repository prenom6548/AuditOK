wayland_client = dependency('wayland-client')
files_libkywc = files(
  'libkywc.c',
  'output.c',
  'toplevel.c',
  'thumbnail.c',
  'workspace.c',
  'cursor.c',
  # providers
  'kywc_capture.c',
  'kywc_output.c',
  'kywc_toplevel.c',
  'kywc_workspace.c',
)

protos = [
  'kywc-capture-manager-v1',
  'kywc-output-manager-v1', 
  'kywc-toplevel-manager-v1',
  'kywc-workspace-manager-v1',
]

foreach p : protos 
  files_libkywc += protocols_code[p]
  files_libkywc += protocols_client_header[p]
endforeach

egl = dependency('egl')
glesv2 = dependency('glesv2')
wayland_egl = dependency('wayland-egl')

buffer_sources = ['buffer.c', 'buffer.h']
foreach p : ['xdg-shell', 'xdg-decoration']
  if p in protocols_code and p in protocols_client_header
    buffer_sources += [protocols_code[p], protocols_client_header[p]]
  endif
endforeach

buffer_static = static_library(
  'kywc_buffer',
  buffer_sources,
  dependencies: [egl, glesv2, wayland_egl, png],
  build_by_default : false, 
)

kywc_buffer = declare_dependency(
  link_with: buffer_static,
)

ld_args = '-Wl,--version-script,@0@/@1@'.format(
  meson.current_source_dir(), 'kywc.syms')
link_depends = files('kywc.syms')

kywc = shared_library(
  'kywc',
  files_libkywc,
  c_args: '-fvisibility=default',
  link_args : ld_args,
  link_depends : link_depends,
  dependencies: wayland_client,
  install : true,
  version : meson.project_version(),
)

libkywc = declare_dependency(
  link_with: kywc,
  dependencies: wayland_client,
  include_directories: include_directories('.'),
)

install_headers('libkywc.h')

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  kywc,
  description: 'Kylin wayland compositor client library',
  url: 'https://gitee.com/openkylin/kylin-wayland-compositor',
)
