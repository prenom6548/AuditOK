xcb_errors = dependency('libxcb-errors', required: get_option('libxcb-errors'))

xwayland_dep = dependency(
  'xwayland',
  required: false,
  fallback: 'xserver',
  default_options: [
    'werror=false',
    'xvfb=false',
  ],
)

xwayland_feature_names = [
  'listenfd',
  'no_touch_pointer_emulation',
  'force_xrandr_emulation',
  'terminate_delay',
]

if xwayland_dep.found()
  internal_config.set_quoted('XWAYLAND_PATH', xwayland_dep.get_variable('xwayland', default_value: '/usr/bin/Xwayland'))
  foreach name : xwayland_feature_names
    have = xwayland_dep.get_variable('have_' + name, default_value: 'false') == 'true'
    internal_config.set10('HAVE_XWAYLAND_' + name.to_upper(), have)
  endforeach
else
  internal_config.set_quoted('XWAYLAND_PATH', '/usr/bin/Xwayland')
  foreach name : xwayland_feature_names
    internal_config.set10('HAVE_XWAYLAND_' + name.to_upper(), false)
  endforeach
endif
internal_config.set10('HAVE_XCB_ERRORS', xcb_errors.found())

xwayland_libs = []
xwayland_required = {
  'xcb': [],
  'xcb-composite': [],
  'xcb-ewmh': [],
  'xcb-icccm': [],
  'xcb-render': [],
  'xcb-res': [],
  'xcb-shape': [],
  'xcb-xfixes': [],
  'xcb-sync': [],
}
xwayland_optional = {
  'libxcb-errors': 'Required for printing X11 errors.',
}

foreach lib, version : xwayland_required
  dep = dependency(lib, version: version)
  xwayland_libs += dep
endforeach

foreach lib, desc : xwayland_optional
  dep = dependency(lib, required: get_option('libxcb-errors'))
  xwayland_libs += dep
endforeach

wlcom_deps += xwayland_libs

wlcom_sources += files(
  'grab.c',
  'shell.c',
  'server.c',
  'sockets.c',
  'unmanaged.c',
  'view.c',
  'xauth.c',
  'xwayland.c',
  'xwm.c',
)

subdir('selection')
