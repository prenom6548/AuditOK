if not add_languages('cpp', native: false, required: false)
  subdir_done()
endif

security_protocol = custom_target(
  'security-c',
  input: 'security.xml',
  output: 'security-protocol.c',
  command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)

security_server_header = custom_target(
  'security-server-h',
  input: 'security.xml',
  output: 'security-protocol.h',
  command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

security_client_header = custom_target(
  'security-client-h',
  input: 'security.xml',
  output: 'security-client-protocol.h',
  command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
)

qt5 = import('qt5')
qt5_dep = dependency(
  'qt5',
  modules: ['Core', 'Gui', 'Widgets'],
  private_headers: true,
  required: false 
)

if not qt5_dep.found()
  subdir_done()
endif

gui_source = [
  'gui.cpp',
  security_protocol,
  security_client_header,
]

#gui_source += qt5.preprocess(
#  moc_sources: ['gui.cpp'],
#  dependencies: qt5_dep,
#)

wayland_client = dependency('wayland-client')

gui_exe = executable(
  'gui',
  sources: gui_source,
  dependencies: [qt5_dep, wayland_client],
  cpp_args: ['-Wno-unused-parameter'],
  override_options: ['optimization=3', 'debug=false'],
  install: false
)

security_source = [
  'security.c',
  security_protocol,
  security_server_header,
]

security_source += custom_target(
    input: gui_exe,
    output: 'gui_exe.h',
    command: [embed, 'gui_exe'],
    feed: true,
    capture: true,
)

if wlroots.type_name() == 'pkgconfig'
 wlr_inc = wlroots.get_variable('includedir')
else
  wlr_inc = subproject('wlroots').get_variable('wlr_inc')
endif

shared_module(
  'security',
  security_source,
  name_prefix: '',

  link_with: util_static,
  include_directories : wlcom_inc,
  install_dir: plugindir,
  install: false
)
