util_common = files(
  'anim.c',
  'boxes.c',
  'client.c',
  'dialog.c',
  'event.c',
  'file.c',
  'hash_table.c',
  'limit.c',
  'nls.c',
  'queue.c',
  'spsc_chain.c',
  'string.c',
  'sysfs.c',
  'time.c',
)

util_sources = files(
  'identifier.c',
  'logger.c',
  'spawn.c',
)

util_static = static_library(
  'util_static',
  util_common,
  include_directories: wlcom_inc,
  gnu_symbol_visibility: 'hidden',
  build_by_default : false,
)

util_client = static_library(
  'util_client',
  util_sources,
  link_with: util_static,
  include_directories: wlcom_inc,
  c_args: '-DUTIL_USED_IN_CLIENT',
  gnu_symbol_visibility: 'hidden',
  build_by_default : false,
)

util = declare_dependency(
  link_with: util_client,
)

crypto = dependency('openssl')
wlcom_deps += crypto

wlcom_sources += util_common
wlcom_sources += util_sources
wlcom_sources += files(
  'dbus.c',
  'debug.c',
  'drm_format.c',
  'matrix.c',
  'quirks.c',
  'vec2.c',
  'wayland.c',
)
