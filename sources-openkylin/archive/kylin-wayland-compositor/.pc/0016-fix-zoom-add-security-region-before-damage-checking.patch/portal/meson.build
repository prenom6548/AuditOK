files_portal = files(
  'main.c',
  'input_capture.c',
  'remote_desktop.c',
  'session.c',
  'wayland.c',
)

portal_deps = [
  wayland_client, sdbus,
  libkywc, util,
]

executable(
  'xdg-desktop-portal-wlcom',
  files_portal,
  include_directories: wlcom_inc,
  dependencies: portal_deps,
  install_dir: get_option('libexecdir'),
  install: true,
)

install_data(
  'data/xdg-desktop-portal-wlcom.service',
  install_dir: '/usr/lib/systemd/user'
)

install_data(
  'data/org.freedesktop.impl.portal.desktop.wlcom.service',
  install_dir: '/usr/share/dbus-1/services'
)

install_data(
  'data/wlcom.portal',
  install_dir: '/usr/share/xdg-desktop-portal/portals'
)

install_data(
  'data/ukui-portals.conf',
  install_dir: '/usr/share/xdg-desktop-portal'
)
